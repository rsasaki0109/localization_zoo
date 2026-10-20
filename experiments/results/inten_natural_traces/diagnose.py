#!/usr/bin/env python3
"""Join this study's complete natural traces, spectra and saved-pose steps.

Study-specific offline analysis, not a new estimator/tool API. No scan is opened.
The committed protocol fixes the motion labels and spectral convention. Every
step is exported; comparisons use all moving steps, not selected failure windows.
Stage errors are observations along the realized trajectory, not independent
S2S-only/S2M-only counterfactual trajectories. GT is used only here offline.
"""
import argparse
from collections import Counter
import csv
import gzip
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

import numpy as np

REPO = Path(__file__).resolve().parents[3]
STUDY = Path(__file__).resolve().parent
sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from diagnose_kitti_steps import diagnose
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose, rotation_angle


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read(path):
    return json.loads(Path(path).read_text())


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def restore(folder, temp):
    """Verify every archived byte; reassemble split JSON lines before parsing."""
    archive = read(folder / 'archive.json')
    require(archive['exit_code'] == 0 and archive['byte_roundtrip_verified'], 'unclosed archive')
    for name, entry in archive['files'].items():
        destination = temp / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        with destination.open('wb') as out:
            if entry['storage'] == 'ordered_gzip_shards':
                for part in entry['parts']:
                    path = folder / part['path']
                    require(sha(path) == part['gzip_sha256'] and
                            path.stat().st_size == part['gzip_bytes'], 'stored shard mismatch')
                    raw = gzip.decompress(path.read_bytes())
                    require(hashlib.sha256(raw).hexdigest() == part['raw_sha256'] and
                            len(raw) == part['raw_bytes'], 'decoded shard mismatch')
                    out.write(raw)
            else:
                path = folder / entry['path']
                require(sha(path) == entry['stored_sha256'], 'stored file mismatch')
                out.write(gzip.decompress(path.read_bytes()) if entry['storage'] == 'gzip'
                          else path.read_bytes())
        require(sha(destination) == entry['sha256'] and destination.stat().st_size == entry['bytes'],
                'restored file mismatch: ' + name)
    return archive


def distribution(values):
    values = [v for v in values if v is not None]
    require(all(np.isfinite(v) for v in values), 'nonfinite derived statistic')
    return dict(n=len(values), min=float(min(values)) if values else None,
                p50=float(np.median(values)) if values else None,
                max=float(max(values)) if values else None)


def aggregate(rows):
    continuous = ('gt_step_m', 's2s_input_length_ratio', 's2s_length_ratio',
                  'prediction_length_ratio', 'final_length_ratio',
                  's2s_relative_vector_error', 'final_relative_vector_error',
                  's2m_correction_along_gt_fraction', 's2m_correction_m',
                  'final_rotation_error_rad', 'position_error_m', 'dor_removed',
                  'features_edge', 'features_ground', 'features_facade', 'features_reflector')
    result = dict(steps=len(rows), statistics={k: distribution(r[k] for r in rows) for k in continuous},
                  s2s_fallback=sum(r['s2s_fallback'] for r in rows),
                  s2m_accepted=sum(r['s2m_accepted'] for r in rows),
                  s2s_bad_length=sum(r['s2s_bad_length'] for r in rows),
                  final_bad_length=sum(r['bad_length'] for r in rows),
                  s2s_bad_to_final_bad=sum(r['s2s_bad_length'] and r['bad_length'] for r in rows),
                  s2s_good_to_final_bad=sum(not r['s2s_bad_length'] and r['bad_length'] for r in rows),
                  s2s_bad_to_final_good=sum(r['s2s_bad_length'] and not r['bad_length'] for r in rows),
                  stages={})
    for stage in ('s2s', 's2m'):
        names = ('pairs', 'iterations', 'condition_first', 'condition_last', 'condition_max',
                 'lambda_min_last', 'residual_per_pair_first', 'residual_per_pair_last')
        result['stages'][stage] = dict(
            stops=dict(Counter(r[stage + '_stop'] for r in rows)),
            statistics={k: distribution(r[stage + '_' + k] for r in rows) for k in names})
    return result


def matrix(value):
    result = np.asarray(value, dtype=float).reshape(4, 4)
    require(np.isfinite(result).all(), 'nonfinite pose')
    return result


def sequence(seq, output, protocol):
    folder = STUDY / 'sequences' / seq
    with tempfile.TemporaryDirectory(prefix='inten-natural-') as scratch:
        temp = Path(scratch)
        archive = restore(folder, temp)
        parity, validation = read(temp / 'parity.json'), read(temp / 'trace_validation.json')
        require(parity['exit_code'] == validation['exit_code'] == 0 and
                all(parity['saved_pose_bytes_equal'].values()) and validation['skipped_frames'] == 0,
                'incomplete or mismatched control')
        for name in parity['saved_pose_bytes_equal']:
            require((temp / 'on/dogfooding_results' / name).read_bytes() ==
                    (temp / 'off/dogfooding_results' / name).read_bytes(), 'pose parity changed')
        for mode in ('off', 'on'):
            require(read(temp / mode / 'native_command.json')['exit_code'] == 0, 'native failure')
        summary = read(temp / 'on/summary.json')
        estimate = load_kitti_poses(temp / 'on/dogfooding_results/InTEn_LOAM.txt')
        gt = load_kitti_poses(temp / 'on' / summary['artifacts']['evaluated_gt_path'])
        saved_motion = read(temp / 'motion_steps.json')
        motion = diagnose(estimate, gt, **dict(block_frames=protocol['motion_diagnostics']['block_frames'],
                          min_step_m=protocol['motion_diagnostics']['min_step_m'],
                          relative_threshold=protocol['motion_diagnostics']['relative_threshold']))
        for key in motion:
            require(motion[key] == saved_motion[key], 'motion replay mismatch: ' + key)
        gt = np.asarray(normalize_first_pose(gt))
        est = np.asarray(normalize_first_pose(estimate))
        n = len(gt)
        require(n == validation['input_frames'] == validation['processed_frames'], 'GT index coverage')
        conditioning = read(temp / 'conditioning/summary.json')
        require(sha(temp / 'on/trace.jsonl') == validation['trace_sha256'] and
                sha(temp / 'conditioning/iterations.jsonl') == conditioning['iterations_sha256'],
                'validated analysis stream mismatch')
        require(conditioning['convention']['rotation_length_m'] == protocol['conditioning']['rotation_length_m']
                and conditioning['convention']['rank_rtol'] == protocol['conditioning']['rank_rtol'],
                'spectral convention changed')
        rows, stage_counts = [], {s: Counter() for s in ('s2s', 's2m')}
        minimum_pairs = {s: [] for s in stage_counts}
        iteration_count = 0
        max_saved_pose_difference, max_prediction_difference = 0., 0.
        with (temp / 'on/trace.jsonl').open() as stream, (temp / 'conditioning/iterations.jsonl').open() as spectra:
            header = json.loads(next(stream))
            require(not header['params']['accelerated'], 'unexpected acceleration')
            previous = None
            frames = 0
            for line in stream:
                frame = json.loads(line)
                if frame['type'] == 'end':
                    continue
                require(frame['type'] == 'frame' and frame['frame_index'] == frame['input_index'] == frames,
                        'not continuous ordered traversal')
                i = frames
                frames += 1
                pose = matrix(frame['pose'])
                saved_difference = float(np.max(np.abs(matrix(frame['anchored_pose']) - estimate[i])))
                max_saved_pose_difference = max(max_saved_pose_difference, saved_difference)
                require(np.allclose(matrix(frame['anchored_pose']), estimate[i], atol=1e-5, rtol=1e-8),
                        'trace and saved pose disagree beyond saved precision')
                if frame['initialization']:
                    require(i == 0, 'late initialization')
                    previous = pose
                    continue
                require(previous is not None, 'missing initialization')
                local_gt = np.linalg.inv(gt[i-1]) @ gt[i]
                local_final_saved = np.linalg.inv(est[i-1]) @ est[i]
                local_final = np.linalg.inv(previous) @ pose
                prediction = np.linalg.inv(previous) @ matrix(frame['s2m_prediction'])
                accepted = matrix(frame['accepted_relative'])
                prediction_difference = float(np.max(np.abs(prediction - accepted)))
                max_prediction_difference = max(max_prediction_difference, prediction_difference)
                require(np.allclose(prediction, accepted, atol=1e-8, rtol=1e-8), 'prediction composition mismatch')
                step = float(np.linalg.norm(local_gt[:3, 3]))
                moving = step >= protocol['motion_diagnostics']['min_step_m']
                threshold = protocol['motion_diagnostics']['relative_threshold']
                def length(T):
                    return float(np.linalg.norm(T[:3, 3]) / step) if moving else None
                def vector(T):
                    return float(np.linalg.norm(T[:3, 3] - local_gt[:3, 3]) / step) if moving else None
                def bad(value):
                    return value is not None and abs(value - 1) > threshold
                s2s = matrix(frame['s2s']['output'])
                correction = local_final[:3, 3] - prediction[:3, 3]
                row = dict(frame=i, moving=moving, gt_step_m=step,
                           bad_length=bad(length(local_final_saved)),
                           bad_vector=moving and vector(local_final_saved) > threshold,
                           s2s_bad_length=bad(length(s2s)),
                           s2s_input_length_ratio=length(matrix(frame['s2s']['input'])),
                           s2s_length_ratio=length(s2s), prediction_length_ratio=length(prediction),
                           final_length_ratio=length(local_final_saved),
                           s2s_relative_vector_error=vector(s2s), final_relative_vector_error=vector(local_final_saved),
                           final_rotation_error_rad=rotation_angle(local_final_saved[:3,:3].T @ local_gt[:3,:3]),
                           s2m_correction_along_gt_fraction=float(correction @ local_gt[:3,3] / step**2) if moving else None,
                           s2m_correction_m=float(np.linalg.norm(correction)),
                           position_error_m=float(np.linalg.norm(est[i,:3,3] - gt[i,:3,3])),
                           s2s_fallback=frame['s2s_fallback'], s2m_accepted=frame['s2m_accepted'],
                           dor_removed=frame['dor_removed'],
                           **{'features_' + k: v for k, v in frame['features'].items()})
                for stage in ('s2s', 's2m'):
                    solver = frame[stage]
                    require(solver['called'] and solver['iterations'], 'unexpected uncalled stage')
                    analyzed = []
                    for iteration in solver['iterations']:
                        spectral = json.loads(next(spectra))
                        require((spectral['input_index'], spectral['frame_index'], spectral['stage'], spectral['iteration_index'])
                                == (i, i, stage, iteration['index']), 'iteration join mismatch')
                        for key, native in (('native_H','H'), ('native_gradient','g'), ('native_delta','delta'),
                                            ('linearization_pose','linearization_pose'), ('class_pairs','class_pairs')):
                            require(spectral[key] == iteration[native], 'analyzer/native mismatch: ' + key)
                        stage_counts[stage]['status:' + spectral['status']] += 1
                        stage_counts[stage]['rank:' + str(spectral['numerical_rank'])] += 1
                        stage_counts[stage]['step_attempted'] += int(iteration['step_attempted'])
                        stage_counts[stage]['step_finite'] += int(iteration['step_finite'])
                        stage_counts[stage]['update_applied'] += int(iteration['update_applied'])
                        minimum_pairs[stage].append(iteration['pairs'])
                        analyzed.append(spectral)
                        iteration_count += 1
                    prefix = stage + '_'
                    row.update({prefix+'pairs': solver['returned_pairs'], prefix+'stop': solver['stop'],
                                prefix+'iterations': len(analyzed),
                                prefix+'condition_first': analyzed[0]['condition_number'],
                                prefix+'condition_last': analyzed[-1]['condition_number'],
                                prefix+'condition_max': max((a['condition_number'] for a in analyzed
                                                            if a['condition_number'] is not None), default=None),
                                prefix+'lambda_min_last': analyzed[-1]['eigenvalues'][0] if analyzed[-1]['eigenvalues'] else None})
                    for label, a in (('first',analyzed[0]), ('last',analyzed[-1])):
                        value = a['weighted_squared_residual']
                        row[prefix+'residual_per_pair_'+label] = value / a['pairs'] if isinstance(value,(int,float)) and a['pairs'] else None
                rows.append(row)
                previous = pose
            require(frames == n and next(spectra, None) is None, 'incomplete frame/iteration join')
        require(sum(r['bad_length'] for r in rows) == motion['steps_length_off_over_threshold'] and
                sum(r['bad_vector'] for r in rows) == motion['steps_vector_off_over_threshold'], 'bad step label mismatch')
        require(iteration_count == sum(validation['stages'][s]['iterations'] for s in stage_counts), 'iteration coverage')
        groups = {name: aggregate(part) for name, part in (
            ('all_steps', rows), ('moving', [r for r in rows if r['moving']]),
            ('nonmoving', [r for r in rows if not r['moving']]),
            ('bad_length', [r for r in rows if r['bad_length']]),
            ('other_moving', [r for r in rows if r['moving'] and not r['bad_length']]),
            ('bad_vector', [r for r in rows if r['bad_vector']]))}
        ranges = [dict(first_frame=a, last_frame=b, observations=aggregate([r for r in rows if a <= r['frame'] <= b]))
                  for a,b in motion['bad_length_step_frame_ranges']]
        with (output / (seq + '_frames.csv')).open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
        return dict(sequence=seq, frames=n, native_parity=parity, validation=validation,
                    archive_manifest_sha256=sha(folder/'archive.json'), archive_files_verified=len(archive['files']),
                    all_iteration_counts=stage_counts, minimum_iteration_pairs={s:min(v) for s,v in minimum_pairs.items()},
                    joined_iterations=iteration_count, saved_pose_max_absolute_difference=max_saved_pose_difference,
                    prediction_composition_max_absolute_difference=max_prediction_difference,
                    metrics=motion['official_metrics_from_saved_poses'], groups=groups, bad_length_ranges=ranges,
                    motion_blocks=motion['blocks'], cleanup=read(folder/'cleanup.json'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    plan = read(STUDY/'diagnosis_protocol.json')
    subprocess.run(['git','diff','--exit-code',plan['raw_evidence_commit'],'--',
                    str(STUDY/'sequences'),str(STUDY/'protocol.json'),str(STUDY/'binary.json')],
                   cwd=REPO,check=True)
    for path in (Path(__file__).resolve(), STUDY/'diagnosis_protocol.json'):
        require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(REPO).as_posix()],cwd=REPO)
                == path.read_bytes(), 'analysis source must be committed before execution')
    args.output_dir.mkdir(parents=True, exist_ok=False)
    protocol = read(STUDY/'protocol.json')
    results = [sequence(s, args.output_dir, protocol) for s in protocol['sequence_order']]
    write(args.output_dir/'summary.json', dict(schema_version=1, exit_code=0,
          source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip(),
          command=[sys.executable,*sys.argv], cwd=str(Path.cwd()), implementation_sha256=sha(__file__),
          protocol_sha256=sha(STUDY/'protocol.json'), diagnosis_protocol_sha256=sha(STUDY/'diagnosis_protocol.json'),
          raw_evidence_commit=plan['raw_evidence_commit'], sequences=results,
          limits='Realized-trajectory observations only; no causal intervention or accuracy improvement. '
                 'Full rank is not correspondence correctness. Residuals use changing associations; '
                 'last H precedes its update. No inferred convergence threshold or rejection gate.'))


if __name__ == '__main__':
    main()
