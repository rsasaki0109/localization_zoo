#!/usr/bin/env python3
"""Run pinned official KISS-Matcher pair checks; score GT only after workers exit."""
from __future__ import annotations

import argparse
import hashlib
import importlib
import importlib.metadata
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time

import numpy as np

from evaluate_external_kitti_odometry import load_reference_csv


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def augmentation(yaw_deg):
    angle = np.radians(yaw_deg)
    matrix = np.eye(4)
    matrix[:2, :2] = [[np.cos(angle), -np.sin(angle)], [np.sin(angle), np.cos(angle)]]
    return matrix


def expected_transform(poses, source, target, yaw_deg):
    # Worker registers A*p_source into p_target: T_target_source * inverse(A).
    return np.linalg.inv(poses[target]) @ poses[source] @ np.linalg.inv(augmentation(yaw_deg))


def errors(estimate, reference):
    delta = np.linalg.inv(reference) @ estimate
    rotation = np.degrees(np.arccos(np.clip((np.trace(delta[:3, :3])-1)/2, -1, 1)))
    return float(rotation), float(np.linalg.norm(delta[:3, 3]))


def child_command(python, jobs, output):
    return [str(python), str(Path(__file__).resolve()), '--worker-jobs', str(jobs),
            '--worker-output', str(output)]


def worker(jobs_path, output):
    # This input has clouds/augmentations/config only; no reference poses or path.
    import kiss_matcher as km
    job = json.loads(jobs_path.read_text())
    if importlib.metadata.version('kiss-matcher') != job['package_version']:
        raise ValueError('official package version differs from frozen protocol')
    config = km.KISSMatcherConfig(job['voxel_size_m'])
    config.use_quatro = job['use_quatro']
    actual = {name: getattr(config, name) for name in job['expected_config']}
    if actual != job['expected_config']:
        raise ValueError('official matcher configuration differs from frozen protocol')
    case = job['case']
    if case['kind'] == 'synthetic':
        rng = np.random.default_rng(42)
        source = rng.uniform(-5, 5, (5000, 3)).astype(np.float32)
        q, r = np.linalg.qr(rng.standard_normal((3, 3)))
        rotation = q @ np.diag(np.sign(np.diag(r)))
        if np.linalg.det(rotation) < 0:
            rotation[:, 0] *= -1
        target = source @ rotation.T + [1.5, -.7, .4]
    else:
        clouds = []
        for name in ('source', 'target'):
            item = job[name]
            path = Path(job['raw_root']) / item['name']
            if sha(path) != item['sha256'] or path.stat().st_size != item['bytes']:
                raise ValueError('input hash/size mismatch: ' + str(path))
            cloud = np.fromfile(path, dtype='<f4').reshape(-1, 4)[:, :3]
            if not np.isfinite(cloud).all():
                raise ValueError('nonfinite input cloud')
            clouds.append(cloud)
        source, target = clouds
        source = source @ augmentation(case['source_yaw_deg'])[:3, :3].T
    source, target = [np.ascontiguousarray(p, dtype=np.float32) for p in (source, target)]
    matcher = km.KISSMatcher(config)
    start = time.perf_counter()
    result = matcher.estimate(source, target)
    elapsed = time.perf_counter() - start
    transform = np.eye(4)
    transform[:3, :3] = np.asarray(result.rotation)
    transform[:3, 3] = np.asarray(result.translation)
    if not np.isfinite(transform).all():
        raise ValueError('nonfinite estimated transform')
    backend = Path(importlib.import_module(km.KISSMatcher.__module__).__file__).resolve()
    save(output, {'transform': transform.tolist(), 'seconds': elapsed,
                  'source_points': len(source), 'target_points': len(target),
                  'rotation_inliers': matcher.get_num_rotation_inliers(),
                  'final_inliers': matcher.get_num_final_inliers(), 'config': actual,
                  'package_version': km.__version__,
                  'backend_files': {backend.name: sha(backend)},
                  'python': sys.version, 'platform': platform.platform(),
                  'thread_environment': {n: os.environ.get(n) for n in
                                         ('OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS')}})


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--manifest', type=Path)
    parser.add_argument('--raw-root', type=Path)
    parser.add_argument('--reference-csv', type=Path)
    parser.add_argument('--output-dir', type=Path)
    parser.add_argument('--official-python', type=Path, default=Path(sys.executable))
    parser.add_argument('--worker-jobs', type=Path, help=argparse.SUPPRESS)
    parser.add_argument('--worker-output', type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.worker_jobs:
        worker(args.worker_jobs, args.worker_output)
        return
    if any(v is None for v in (args.manifest, args.raw_root, args.reference_csv, args.output_dir)):
        parser.error('manifest, raw-root, reference-csv and output-dir are required')
    protocol = json.loads(args.manifest.read_text())
    if sha(args.reference_csv) != protocol['reference_csv_sha256']:
        raise ValueError('reference hash mismatch')
    args.output_dir.mkdir(parents=True, exist_ok=False)
    commands = []
    env = dict(os.environ, OMP_NUM_THREADS='2', OPENBLAS_NUM_THREADS='1')
    # Complete registration first. Workers never receive GT or a GT initializer.
    for variant in protocol['variants']:
        for case in protocol['cases']:
            stem = variant['id'] + '__' + case['id']
            job = {'package_version': protocol['package_version'],
                   'voxel_size_m': protocol['voxel_size_m'], 'case': case,
                   'use_quatro': variant['use_quatro'], 'expected_config': variant['config']}
            if case['kind'] == 'kitti':
                job['raw_root'] = str(args.raw_root.resolve())
                for name in ('source', 'target'):
                    job[name] = protocol['inputs'][f"{case[name + '_frame']:06d}.bin"]
            job_path = args.output_dir / (stem + '.job.json')
            output = args.output_dir / (stem + '.estimate.json')
            save(job_path, job)
            argv = child_command(args.official_python, job_path, output)
            start = time.perf_counter()
            with (args.output_dir/(stem+'.stdout')).open('wb') as stdout, \
                 (args.output_dir/(stem+'.stderr')).open('wb') as stderr:
                try:
                    result = subprocess.run(argv, env=env, stdout=stdout, stderr=stderr,
                                            timeout=protocol['timeout_seconds'])
                    code, timed_out = result.returncode, False
                except subprocess.TimeoutExpired:
                    code, timed_out = None, True
            row = {'variant': variant['id'], 'case': case['id'], 'argv': argv,
                   'exit_code': code, 'timed_out': timed_out,
                   'wall_seconds': time.perf_counter()-start}
            commands.append(row)
            save(args.output_dir/'commands.json', commands)
            print(stem, 'exit', code, 'timeout', timed_out, flush=True)
    poses = load_reference_csv(args.reference_csv)
    scored = []
    for command in commands:
        case = next(c for c in protocol['cases'] if c['id'] == command['case'])
        row = dict(command)
        row['success'] = False
        if command['exit_code'] == 0 and not command['timed_out']:
            estimate_path = args.output_dir/(row['variant']+'__'+row['case']+'.estimate.json')
            estimate = json.loads(estimate_path.read_text())
            if case['kind'] == 'synthetic':
                rng = np.random.default_rng(42)
                rng.uniform(-5, 5, (5000, 3))
                q, r = np.linalg.qr(rng.standard_normal((3, 3)))
                rotation = q @ np.diag(np.sign(np.diag(r)))
                if np.linalg.det(rotation) < 0:
                    rotation[:, 0] *= -1
                reference = np.eye(4)
                reference[:3, :3], reference[:3, 3] = rotation, [1.5, -.7, .4]
            else:
                reference = expected_transform(poses, case['source_frame'],
                                               case['target_frame'], case['source_yaw_deg'])
            row['rotation_error_deg'], row['translation_error_m'] = errors(
                np.array(estimate['transform']), reference)
            row['success'] = (row['rotation_error_deg'] <= protocol['success_rotation_deg']
                              and row['translation_error_m'] <= protocol['success_translation_m'])
            row['estimate_sha256'] = sha(estimate_path)
        scored.append(row)
    save(args.output_dir/'summary.json', {'protocol_sha256': sha(args.manifest),
         'reference_csv_sha256': sha(args.reference_csv), 'cases': scored,
         'all_commands_completed': all(r['exit_code'] == 0 and not r['timed_out'] for r in commands),
         'claim_limit': protocol['claim_limit']})


if __name__ == '__main__':
    main()
