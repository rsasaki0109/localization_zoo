#!/usr/bin/env python3
"""Oracle-only diagnostic. GT initialization here is excluded from benchmark scores."""
import argparse
import hashlib
import importlib.metadata
import io
import json
from pathlib import Path
import sys
import zipfile

import numpy as np
from scipy.spatial import cKDTree
import small_gicp

sys.path.insert(0, str(Path(__file__).resolve().parents[3]/'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_reference_csv
from run_official_kiss_matcher import errors


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def residuals(source, target, transform, gate):
    moved = source @ transform[:3, :3].T + transform[:3, 3]
    result = {}
    for name, points, destination in [('source_to_target', moved, target),
                                      ('target_to_source', target, moved)]:
        distance, _ = cKDTree(destination).query(points, workers=1)
        supported = distance <= gate
        result[name] = {'points': len(points), 'supported_points': int(supported.sum()),
                       'support_fraction': float(supported.mean()),
                       'supported_rmse_m': float(np.sqrt(np.mean(distance[supported]**2))) if supported.any() else None,
                       'clipped_mean_squared_m2': float(np.mean(np.minimum(distance, gate)**2)),
                       'median_distance_m': float(np.median(distance))}
    return result


def main():
    parser = argparse.ArgumentParser(__doc__)
    for name in ('protocol', 'raw-root', 'reference-csv', 'poses-zip', 'calib-zip', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise FileExistsError(args.output)
    study = args.protocol.resolve().parent
    p = json.loads(args.protocol.read_text())
    assert importlib.metadata.version('small-gicp') == '1.0.1'
    assert importlib.metadata.version('scipy') == '1.15.3'
    assert sha(args.reference_csv) == p['reference_sha256']
    assert sha(args.poses_zip) == p['official_poses_zip_sha256']
    assert sha(args.calib_zip) == p['official_calib_zip_sha256']
    assert sha(study/'matcher_protocol.json') == p['pair_protocol_sha256']
    assert sha(study/'refined_estimates.json') == p['refined_estimates_sha256']
    pairs = json.loads((study/'matcher_protocol.json').read_text())
    previous = json.loads((study/'refined_estimates.json').read_text())['cases']
    with zipfile.ZipFile(args.poses_zip) as z:
        official = np.loadtxt(io.BytesIO(z.read('dataset/poses/00.txt')))
    with zipfile.ZipFile(args.calib_zip) as z:
        calib = z.read('dataset/sequences/00/calib.txt').decode()
    tr = next(line for line in calib.splitlines() if line.startswith('Tr:'))
    extrinsic = np.eye(4)
    extrinsic[:3] = np.array(list(map(float, tr.split()[1:]))).reshape(3, 4)
    poses = load_reference_csv(args.reference_csv)
    rows = []
    for case in pairs['cases']:
        i, j = case['source_frame'], case['target_frame']
        reference = np.linalg.solve(poses[j], poses[i])
        camera = []
        for index in (i, j):
            matrix = np.eye(4)
            matrix[:3] = official[index].reshape(3, 4)
            camera.append(matrix)
        canonical = np.linalg.inv(extrinsic) @ np.linalg.solve(camera[1], camera[0]) @ extrinsic
        frame_difference = float(np.max(np.abs(reference-canonical)))
        assert frame_difference < p['frame_matrix_check_tolerance']
        clouds = {}
        for name in ('source', 'target'):
            pin = pairs['inputs'][f"{case[name+'_frame']:06d}.bin"]
            path = args.raw_root/pin['name']
            assert sha(path) == pin['sha256'] and path.stat().st_size == pin['bytes']
            clouds[name] = np.asarray(np.fromfile(path, dtype='<f4').reshape(-1, 4)[:, :3], dtype=np.float64)
        config = p['config']
        sampled = {name: np.asarray(small_gicp.voxelgrid_sampling(cloud, config['downsampling_resolution'], 1))
                   for name, cloud in clouds.items()}
        oracle = small_gicp.align(clouds['target'], clouds['source'],
                                  init_T_target_source=reference, **config)
        oracle_pose = np.asarray(oracle.T_target_source)
        transforms = [('reference', reference), ('gt_initialized_icp', oracle_pose)]
        transforms += [(row['variant']+'_refined', np.array(row['transform']))
                       for row in previous if row['case'] == case['id']]
        diagnostics = {}
        for name, transform in transforms:
            rotation, translation = errors(transform, reference)
            diagnostics[name] = {'transform': transform.tolist(), 'rotation_error_deg': rotation,
                                 'translation_error_m': translation, 'residuals': residuals(
                                     sampled['source'], sampled['target'], transform,
                                     config['max_correspondence_distance'])}
        rows.append({'case': case['id'], 'canonical_reference_max_abs_matrix_difference': frame_difference,
                     'oracle_converged': bool(oracle.converged), 'diagnostics': diagnostics})
        args.output.write_text(json.dumps({'oracle_diagnostic_only': True,
            'included_in_benchmark_success_count': False, 'protocol_sha256': sha(args.protocol),
            'backend_sha256': sha(Path(small_gicp.__file__)), 'cases': rows}, indent=2, allow_nan=False)+'\n')


if __name__ == '__main__':
    main()
