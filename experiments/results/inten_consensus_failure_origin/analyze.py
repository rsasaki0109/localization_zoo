"""Locate observed stage crossings in every archived full fixed-setting pair.

GT is read only by this offline diagnostic. No native replay, new trajectory,
gate, threshold sweep or estimator selection is performed.
"""
from pathlib import Path
from collections import Counter
import argparse
import csv
import gzip
import hashlib
import importlib.util
import json
import math
import subprocess
import sys

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]
PARENT = E.parent/'inten_consensus_query_acceleration'
sys.path.insert(0, str(W/'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose


def read(path): return json.loads(Path(path).read_text())
def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def require(ok, reason):
    if not ok: raise ValueError(reason)
def matrix(value): return np.asarray(value, dtype=float).reshape(4, 4)
def ratio_agrees(left, right):
    # Same portable bound as the complete-report verifier, not an accuracy gate.
    return math.isclose(left, right, rel_tol=1e-10, abs_tol=1e-12)
def write(path, value): path.write_text(json.dumps(value, indent=2, allow_nan=False)+'\n')


def trace_records(folder, mode):
    entry = read(folder/'archive.json')['files'][mode+'/trace.jsonl']
    require(entry['storage'] == 'ordered_gzip_shards', 'Unexpected trace archive')
    digest = hashlib.sha256(); size = 0; pending = b''
    for part in entry['parts']:
        compressed = (folder/part['path']).read_bytes()
        require(hashlib.sha256(compressed).hexdigest() == part['gzip_sha256'], 'Compressed trace changed')
        raw = gzip.decompress(compressed)
        require(len(raw) == part['raw_bytes'] and hashlib.sha256(raw).hexdigest() == part['raw_sha256'], 'Trace shard changed')
        digest.update(raw); size += len(raw)
        lines = (pending+raw).split(b'\n'); pending = lines.pop()
        for line in lines:
            require(bool(line), 'Empty trace record')
            yield json.loads(line)
    require(not pending, 'Incomplete terminal trace record')
    require(size == entry['bytes'] and digest.hexdigest() == entry['sha256'], 'Full trace changed')


def stage_metrics(T, gt):
    step = float(np.linalg.norm(gt[:3, 3])); moving = step >= .1
    t = T[:3, 3]
    length = float(np.linalg.norm(t)/step) if moving else None
    vector = float(np.linalg.norm(t-gt[:3, 3])/step) if moving else None
    forward = float(t@gt[:3, 3]/step**2) if moving else None
    return dict(length_ratio=length, vector_error=vector, forward_ratio=forward,
                bad_length=bool(moving and abs(length-1) > .2),
                bad_vector=bool(moving and vector > .2))


def crossing(joint_bad, corrected_bad, accepted_bad):
    if joint_bad:
        return 'joint_already_bad' if accepted_bad else 'joint_bad_to_accepted_good'
    if not accepted_bad: return 'joint_good_to_accepted_good'
    return 'joint_good_seed_bad_accepted_bad' if corrected_bad else 'joint_good_seed_good_accepted_bad'


def ranges(frames):
    out = []
    for i in frames:
        if out and i == out[-1][1]+1: out[-1][1] = i
        else: out.append([i, i])
    return out


def analyze_mode(seq, mode, check_frame):
    folder = PARENT/'sequences'/seq
    summary = read(folder/mode/'summary.json')['methods'][0]
    require(read(folder/mode/'native_command.json')['exit_code'] == 0, 'Native failure')
    gt = np.asarray(normalize_first_pose(load_kitti_poses(folder/mode/'dogfooding_results/velodyne_evaluated_gt.txt')))
    poses = np.asarray(normalize_first_pose(load_kitti_poses(folder/mode/'dogfooding_results/InTEn_LOAM.txt')))
    old = {int(r['frame']): r for r in csv.DictReader((PARENT/'report'/f'{seq}_{mode}_steps.csv').open())}
    require(len(gt) == len(poses) == summary['frames'] == len(old)+1, 'Saved pose coverage changed')
    rows = []; frames = 0; ended = False; transitions = {k: Counter() for k in ('length', 'vector')}
    stream = trace_records(folder, mode); header = next(stream)
    require(header['type'] == 'header' and header['params'].get('s2s_geometry_consensus', False) == (mode == 'candidate'), 'Wrong archived configuration')
    for f in stream:
        if f['type'] == 'end':
            require(not ended, 'Duplicate end record'); ended = True; continue
        require(not ended and f['type'] == 'frame' and f['frame_index'] == frames, 'Trace frame coverage changed')
        i = frames; frames += 1
        check_frame(f, mode == 'candidate')
        if f['initialization']:
            require(i == 0, 'Late initialization'); continue
        truth = np.linalg.inv(gt[i-1])@gt[i]
        final = np.linalg.inv(poses[i-1])@poses[i]
        c = f.get('s2s_consensus'); applied = bool(c and c['applied'])
        row = dict(sequence=seq, mode=mode, frame=i,
                   moving=bool(np.linalg.norm(truth[:3, 3]) >= .1), applied=applied,
                   usable=bool(c and c['usable']), joint_stop=f['s2s']['stop'],
                   refinement_stop=c['refinement']['stop'] if c else 'not_run',
                   s2m_stop=f['s2m']['stop'], joint_pairs=f['s2s']['returned_pairs'],
                   refinement_pairs=c['refinement']['returned_pairs'] if c else 0,
                   s2m_pairs=f['s2m']['returned_pairs'])
        stages = dict(predicted=matrix(f['s2s']['input']), joint=matrix(f['s2s']['output']),
                      accepted=matrix(f['accepted_relative']), final=final)
        if applied: stages['corrected_seed'] = matrix(c['refinement']['input'])
        for name, T in stages.items():
            for key, value in stage_metrics(T, truth).items(): row[name+'_'+key] = value
        for key in ('length_ratio', 'vector_error', 'forward_ratio', 'bad_length', 'bad_vector'):
            row.setdefault('corrected_seed_'+key, None)
        for key in ('final_bad_length', 'final_bad_vector', 's2s_bad_length', 's2s_bad_vector'):
            target = key.replace('s2s_', 'accepted_')
            require(row[target] == (old[i][key] == 'True'), 'Existing step label differs: '+key)
        for key in ('final_length_ratio', 'joint_s2s_length_ratio', 's2s_length_ratio'):
            target = {'joint_s2s_length_ratio': 'joint_length_ratio', 's2s_length_ratio': 'accepted_length_ratio'}.get(key, key)
            previous = old[i][key]
            require((previous == '' and row[target] is None) or (previous != '' and ratio_agrees(float(previous), row[target])), f'Existing step ratio differs: {seq}/{mode}/{i} {key}: {previous} versus {row[target]}')
        n = c['valid_clusters'] if c else 0
        row.update(valid_clusters=n, chosen_index=c['best_index'] if c else None,
                   chosen_shift_m=(c['best_index']-60)*.05 if c else None,
                   chosen_votes=c['votes'][c['best_index']] if c else None,
                   zero_votes=c['votes'][60] if c else None,
                   zero_already_majority=bool(c and n and 2*c['votes'][60] > n),
                   excluded_clusters=len(c['excluded_clusters']) if c else 0,
                   corrected_seed_opposes_prediction=False,
                   corrected_seed_forward_prediction_ratio=None)
        if applied:
            a = stages['predicted'][:3, 3]; b = stages['corrected_seed'][:3, 3]
            require(float(a@a) > 0, 'Usable consensus with zero prediction')
            row['corrected_seed_forward_prediction_ratio'] = float(b@a/(a@a))
            row['corrected_seed_opposes_prediction'] = bool(b@a < 0)
        for label in ('length', 'vector'):
            row['crossing_'+label] = crossing(row['joint_bad_'+label], row['corrected_seed_bad_'+label], row['accepted_bad_'+label]) if row['moving'] and applied else 'not_moving_applied'
            if row['moving'] and applied: transitions[label][row['crossing_'+label]] += 1
            row['s2m_introduces_'+label] = bool(row['moving'] and not row['accepted_bad_'+label] and row['final_bad_'+label])
            row['s2m_removes_'+label] = bool(row['moving'] and row['accepted_bad_'+label] and not row['final_bad_'+label])
        rows.append(row)
    require(ended and frames == summary['frames'], 'Incomplete archived traversal')
    require(len(rows) == frames-1, 'Incomplete step coverage')
    require(all(sum(v.values()) == sum(r['moving'] and r['applied'] for r in rows) for v in transitions.values()), 'Transition partition incomplete')
    return rows, dict(frames=frames, steps=len(rows), native_exit_code=0,
                      moving_applied=sum(r['moving'] and r['applied'] for r in rows),
                      transitions={k: dict(v) for k, v in transitions.items()},
                      s2m_introduces={k: sum(r['s2m_introduces_'+k] for r in rows) for k in transitions},
                      s2m_removes={k: sum(r['s2m_removes_'+k] for r in rows) for k in transitions},
                      native_metrics={k: summary[k] for k in ('kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m', 'ate_m', 'rpe_trans_pct')})


def build(output):
    protocol = read(E/'protocol.json')
    update_path = E/'portability_source_update.json'
    updates = read(update_path) if update_path.exists() else {}
    for name, digest in protocol['source_sha256'].items():
        amendment = updates.get('source_updates', {}).get(name)
        if amendment:
            require(amendment['original_sha256'] == digest and sha(W/amendment['original_copy']) == digest, 'Original frozen source not preserved: '+name)
            digest = amendment['sha256']
        require(sha(W/name) == digest, 'Frozen source changed: '+name)
    for group in ('evidence_sha256', 'scientific_report_sha256'):
        for name, digest in updates.get(group, {}).items(): require(sha(W/name) == digest, 'Portability evidence/result changed: '+name)
    for name, digest in protocol['input_sha256'].items(): require(sha(W/name) == digest, 'Frozen input changed: '+name)
    spec = importlib.util.spec_from_file_location('original_consensus_check', PARENT/'check_consensus_trace.py')
    checker = importlib.util.module_from_spec(spec); spec.loader.exec_module(checker)
    output.mkdir(parents=True, exist_ok=False)
    sequences = {}; entries = []; events = []
    for seq in protocol['sequences']:
        modes = {}; paired = {}
        for mode in ('baseline', 'candidate'):
            rows, modes[mode] = analyze_mode(seq, mode, checker.check_frame); paired[mode] = rows
            with (output/f'{seq}_{mode}_steps.csv').open('w') as stream:
                writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator='\n'); writer.writeheader(); writer.writerows(rows)
            for label in ('length', 'vector'):
                for start, end in ranges(r['frame'] for r in rows if r['final_bad_'+label]):
                    entries.append(dict(sequence=seq, mode=mode, label=label, first_frame=start, last_frame=end,
                                        entry=rows[start-1], previous=rows[start-2] if start > 1 else None))
            if mode == 'candidate':
                events.extend(r for r in rows if r['moving'] and r['applied'] and any(r['crossing_'+label].startswith('joint_good_seed_') for label in ('length', 'vector')))
        require(len(paired['baseline']) == len(paired['candidate']), 'Arm coverage differs')
        for label in ('length', 'vector'):
            actual = [r['frame'] for b, r in zip(paired['baseline'], paired['candidate']) if r['final_bad_'+label] and not b['final_bad_'+label]]
            previous = read(PARENT/'report/summary.json')['sequences'][seq]['local_label_changes'][label]['candidate_only_frames']
            require(actual == previous, 'New-frame set changed')
            modes['candidate']['new_final_bad_'+label+'_frames'] = actual
        sequences[seq] = modes
        print('Reconstructed complete fixed pair', seq, flush=True)
    write(output/'entries.json', entries); write(output/'introduced_events.json', events)
    result = dict(exit_code=0, sequences=sequences, sequence_count=len(sequences),
                  native_reruns=0, raw_scans_acquired=False, settings_changed=False,
                  all_archived_decisions_projected_steps_and_labels_verified=True,
                  all_original_native_metrics_preserved=True,
                  scope='Observed within-frame stage crossings on each realized history. Corrected-seed error uses the actual refinement input, not a newly solved counterfactual trajectory. GT is offline diagnostic only. Counts do not establish physical surface motion, continuous profile support at refined R, or an accuracy improvement.')
    write(output/'summary.json', result)
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    own = Path(__file__).resolve().relative_to(W).as_posix()
    require(subprocess.check_output(['git', 'show', 'HEAD:'+own], cwd=W) == Path(__file__).read_bytes(), 'Commit diagnostic source before executing')
    build(args.output_dir)
