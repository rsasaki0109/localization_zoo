#!/usr/bin/env python3
"""Join committed InTEn progress counts to saved steps without rerunning odometry."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

import numpy as np

REPO = Path(__file__).resolve().parents[3]
STUDY = Path(__file__).resolve().parent
sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from diagnose_kitti_steps import contiguous_ranges, diagnose
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose

FIELDS = ('frame', 'frames', 'ground', 'facade', 'edge', 'reflector', 's2s_pairs',
          's2m_pairs', 's2m_edge', 's2m_ground', 's2m_facade', 's2m_intensity')
PROGRESS = re.compile(
    r'\[InTEn-LOAM paper\]\s+(\d+)/(\d+)\s+g/f/e/r=(\d+)/(\d+)/(\d+)/(\d+)'
    r'\s+s2s=(\d+)\s+s2m=(\d+)\s+\(e/g/f/i (\d+)/(\d+)/(\d+)/(\d+)\)')


def parse_progress(log, frames, stride):
    records = [dict(zip(FIELDS, map(int, match))) for match in PROGRESS.findall(log)]
    if [r['frame'] for r in records] != list(range(0, frames, stride)):
        raise ValueError('Incomplete, duplicate or reordered progress records')
    for row in records:
        if row['frames'] != frames or row['s2m_pairs'] != sum(
                row[k] for k in ('s2m_edge', 's2m_ground', 's2m_facade', 's2m_intensity')):
            raise ValueError('Inconsistent frame or class-pair counts')
    return records


def support_summary(rows, cutoff):
    # Initialization is not a solved step. Callers pass only destination frames.
    return dict(logged_steps=len(rows),
                s2s_below_cutoff=sum(r['s2s_pairs'] < cutoff for r in rows),
                s2m_below_cutoff=sum(r['s2m_pairs'] < cutoff for r in rows),
                s2s_min=min((r['s2s_pairs'] for r in rows), default=None),
                s2m_min=min((r['s2m_pairs'] for r in rows), default=None))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    if args.output_dir.exists():
        parser.error('Use a fresh output directory; do not replace evidence.')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    sources = {}

    def committed(relative, commit=None, require_worktree=True):
        ref = commit or protocol['input_commit']
        raw = subprocess.check_output(['git', 'show', ref + ':' + relative], cwd=REPO)
        if require_worktree:
            assert raw == (REPO / relative).read_bytes(), 'Modified analysis input: ' + relative
        sources[ref + ':' + relative] = hashlib.sha256(raw).hexdigest()
        return raw

    protocol = json.loads((STUDY / 'protocol.json').read_text())
    for name in ('analyze.py', 'protocol.json'):
        committed((STUDY / name).relative_to(REPO).as_posix(), revision)
    for name in ('diagnose_kitti_steps.py', 'evaluate_external_kitti_odometry.py'):
        committed('evaluation/scripts/' + name)
    provenance = json.loads(committed('experiments/results/mesh_inten_kitti_coverage/environment.json'))
    runs = json.loads(committed('experiments/results/mesh_inten_kitti_coverage/parallel_execution.json'))
    recorded_native_commit = provenance['evaluator_git_commit']
    native_path = 'papers/inten_loam/src/inten_loam_paper.cpp'
    native_sources = {}
    for relative in (native_path, 'papers/inten_loam/include/inten_loam/inten_loam_paper.h'):
        recorded = committed(relative, recorded_native_commit, require_worktree=False)
        study_input = committed(relative, require_worktree=False)
        if recorded != study_input:
            raise ValueError('Recorded execution and pinned study source differ: ' + relative)
        native_sources[relative] = dict(recorded_execution_commit=recorded_native_commit,
                                       study_input_commit=protocol['input_commit'],
                                       sha256=hashlib.sha256(recorded).hexdigest(), identical=True)
        if relative == native_path:
            native = recorded.decode()
    assert 'if (result.s2s_pairs < 30) rel = last_rel_;' in native
    assert 'if (result.s2m_pairs >= 30) {' in native
    runner = committed('evaluation/src/pcd_dogfooding.cpp', recorded_native_commit,
                       require_worktree=False).decode()
    assert 'if (i % 10 == 0)' in runner and 'int paper_s2m_interval = 1;' in runner
    pp = protocol['diagnostic_parameters']
    cutoff = protocol['support_cutoff']
    all_rows, sequences, intervals = [], [], []
    fixed_args = None
    for seq in protocol['sequences']:
        manifest = json.loads(committed(f'experiments/inten_loam_kitti_seq_{seq}_full_paper_comp_matrix.json'))
        variant = next(v for v in manifest['variants'] if v['id'] == protocol['variant'])
        if fixed_args is None:
            fixed_args = variant['args']
        assert variant['args'] == fixed_args
        execution = next(r for r in runs['runs'] if r['sequence'] == seq)
        assert execution['exit_code'] == 0
        prefix = f'experiments/results/runs/inten_loam_kitti_seq_{seq}_full_paper_comp_matrix/{protocol["variant"]}/'
        summary = json.loads(committed(prefix + 'summary.json'))
        expected = json.loads(committed(prefix + 'step_diagnostics.json'))
        estimates = load_kitti_poses(REPO / (prefix + 'trajectory_kitti.txt'))
        references = load_kitti_poses(REPO / (prefix + 'evaluated_gt_kitti.txt'))
        committed(prefix + 'trajectory_kitti.txt')
        committed(prefix + 'evaluated_gt_kitti.txt')
        diagnostic = diagnose(estimates, references, block_frames=pp['block_frames'],
                              min_step_m=pp['min_step_m'],
                              relative_threshold=pp['relative_step_threshold'])
        for key in ('frames', 'moving_step_count', 'steps_length_off_over_threshold',
                    'steps_vector_off_over_threshold', 'bad_length_step_frame_ranges',
                    'bad_vector_step_frame_ranges'):
            assert diagnostic[key] == expected[key], (seq, key)
        method = next(m for m in summary['methods'] if m['name'] == 'InTEn-LOAM')
        assert 'intensity=on mapping=on dor=on' in method['note']
        metric_deltas = {k: abs(v - method[k]) for k, v in
                         diagnostic['official_metrics_from_saved_poses'].items()
                         if k in ('ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')}
        assert len(metric_deltas) == 4 and max(metric_deltas.values()) <= 1e-6
        assert diagnostic['frames'] == summary['num_frames'] == len(estimates) == len(references)
        records = parse_progress(committed(prefix + 'run.log').decode(), len(estimates), protocol['expected_log_stride'])
        est = np.asarray(normalize_first_pose(estimates))
        gt = np.asarray(normalize_first_pose(references))
        moving, bad_length, bad_vector = [], [], []
        by_frame = {}
        for row in records:
            frame = row['frame']
            if frame == 0:
                continue
            er = np.linalg.inv(est[frame - 1]) @ est[frame]
            gr = np.linalg.inv(gt[frame - 1]) @ gt[frame]
            length = float(np.linalg.norm(gr[:3, 3]))
            active = length >= pp['min_step_m']
            ratio = float(np.linalg.norm(er[:3, 3]) / length) if active else None
            error = float(np.linalg.norm(er[:3, 3] - gr[:3, 3]) / length) if active else None
            row.update(sequence=seq, gt_step_m=length, moving=active, step_length_ratio=ratio,
                       step_vector_relative_error=error,
                       bad_length=active and abs(ratio - 1) > pp['relative_step_threshold'],
                       bad_vector=active and error > pp['relative_step_threshold'])
            by_frame[frame] = row
            all_rows.append(row)
            if active:
                moving.append(row)
            if row['bad_length']:
                bad_length.append(row)
            if row['bad_vector']:
                bad_vector.append(row)
        for first, last in expected['bad_length_step_frame_ranges']:
            chosen = [r for f, r in by_frame.items() if first <= f <= last]
            assert all(r['bad_length'] for r in chosen)
            intervals.append(dict(sequence=seq, first_frame=first, last_frame=last,
                                  total_bad_steps=last - first + 1,
                                  unlogged_bad_steps=last - first + 1 - len(chosen),
                                  **support_summary(chosen, cutoff)))
        sequences.append(dict(sequence=seq, frames=len(est), logged_progress_records=len(records),
                              logged_noninitial_steps=len(records) - 1,
                              all_bad_length_steps=expected['steps_length_off_over_threshold'],
                              all_bad_vector_steps=expected['steps_vector_off_over_threshold'],
                              unlogged_bad_length_steps=expected['steps_length_off_over_threshold'] - len(bad_length),
                              logged_moving=support_summary(moving, cutoff),
                              logged_bad_length=support_summary(bad_length, cutoff),
                              logged_bad_vector=support_summary(bad_vector, cutoff),
                              saved_pose_metric_abs_delta=metric_deltas))
    args.output_dir.mkdir(parents=True)
    for name, rows in [('logged_steps', all_rows), ('bad_length_intervals', intervals)]:
        with (args.output_dir / (name + '.csv')).open('w', newline='') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator='\n')
            writer.writeheader()
            writer.writerows(rows)
    result = dict(command=[sys.executable, *sys.argv], cwd=str(REPO), analysis_commit=revision,
                  protocol=protocol, recorded_native_commit=recorded_native_commit,
                  recorded_native_binary_sha256=provenance['binary_sha256'],
                  native_source_provenance=dict(files=native_sources,
                      scope='Interpret recorded progress using its historical source blobs and pinned study input. '
                            'Current native checkout files are not analysis inputs or a replay of these results.'),
                  input_sha256=sources, fixed_variant_args=fixed_args,
                  sequences=sequences, bad_length_intervals=intervals, exit_code=0)
    (args.output_dir / 'summary.json').write_text(json.dumps(result, indent=2, allow_nan=False) + '\n')
    print(json.dumps({'sequences':len(sequences), 'exit_code':0}))


if __name__ == '__main__':
    main()
