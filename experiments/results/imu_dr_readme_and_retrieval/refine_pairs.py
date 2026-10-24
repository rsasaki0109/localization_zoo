#!/usr/bin/env python3
"""Explicit opt-in diagnostic: refine saved coarse estimates without GT access."""
import argparse
import hashlib
import importlib.metadata
import json
from pathlib import Path
import time

import numpy as np
import small_gicp


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--protocol', type=Path, required=True)
    parser.add_argument('--raw-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise FileExistsError(args.output)
    protocol = json.loads(args.protocol.read_text())
    assert importlib.metadata.version('small-gicp') == protocol['package_version']
    study = args.protocol.resolve().parent
    pairs = json.loads((study/'matcher_protocol.json').read_text())
    assert sha(study/'matcher_protocol.json') == protocol['pair_protocol_sha256']
    config = protocol['config']
    rows = []

    def align(source, target, initial):
        start = time.perf_counter()
        result = small_gicp.align(np.asarray(target, dtype=np.float64),
                                  np.asarray(source, dtype=np.float64),
                                  init_T_target_source=initial, **config)
        transform = np.asarray(result.T_target_source)
        assert transform.shape == (4, 4) and np.isfinite(transform).all()
        return {'transform': transform.tolist(), 'converged': bool(result.converged),
                'num_inliers': int(result.num_inliers),
                'iterations': int(result.iterations), 'seconds': time.perf_counter()-start}

    # Analytic control checks the library's transform direction independently of GT.
    rng = np.random.default_rng(42)
    source = rng.uniform(-5, 5, (5000, 3))
    reference = np.eye(4)
    angle = .05
    reference[:2, :2] = [[np.cos(angle), -np.sin(angle)], [np.sin(angle), np.cos(angle)]]
    reference[:3, 3] = [.1, -.1, .05]
    control = align(source, source @ reference[:3, :3].T + reference[:3, 3], np.eye(4))
    control['expected_transform'] = reference.tolist()
    control['maximum_matrix_error'] = float(np.max(np.abs(np.array(control['transform'])-reference)))
    for case in pairs['cases']:
        clouds = {}
        for name in ('source', 'target'):
            pin = pairs['inputs'][f"{case[name+'_frame']:06d}.bin"]
            path = args.raw_root/pin['name']
            assert sha(path) == pin['sha256'] and path.stat().st_size == pin['bytes']
            clouds[name] = np.fromfile(path, dtype='<f4').reshape(-1, 4)[:, :3]
        assert case['source_yaw_deg'] == 0
        for variant in pairs['variants']:
            name = variant['id']+'__'+case['id']+'.estimate.json'
            coarse = study/'matcher_run'/name
            assert sha(coarse) == protocol['coarse_sha256'][name]
            initial = np.array(json.loads(coarse.read_text())['transform'])
            row = align(clouds['source'], clouds['target'], initial)
            rows.append({'case': case['id'], 'variant': variant['id'], **row})
            args.output.write_text(json.dumps({'control': control, 'cases': rows,
                'protocol_sha256': sha(args.protocol), 'small_gicp_backend':
                {'path': small_gicp.__file__, 'sha256': sha(Path(small_gicp.__file__))}},
                indent=2, allow_nan=False)+'\n')
    assert control['maximum_matrix_error'] < 1e-3, 'analytic transform-direction control failed'


if __name__ == '__main__':
    main()
