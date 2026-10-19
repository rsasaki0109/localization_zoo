#!/usr/bin/env python3
"""Analyze stored InTEn normal equations in a sensor-centered, scaled tangent.

This is offline analysis, outside the estimator. A spectrum is neither a pose
quality gate nor evidence that correspondences or the estimated motion are right.
"""
import argparse
from collections import Counter
import hashlib
import json
import math
import os
from pathlib import Path
import sys
import tempfile

import numpy as np

try:
    from .check_inten_trace import TraceError, require, strict_json, validate_trace
except ImportError:  # Direct script invocation.
    from check_inten_trace import TraceError, require, strict_json, validate_trace


POSE_ATOL = 1e-8
SYMMETRY_RTOL = 1e-10


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def skew(v):
    x, y, z = v
    return np.array([[0., -z, y], [z, 0., -x], [-y, x, 0.]])


def tangent_map(pose, rotation_length_m):
    """Map q=[sensor translation, L*sensor rotation] to the native left tangent.

    Native v = R*u + [t]x*R*w, native omega = R*w, with q=[u,L*w].
    This recenters the infinitesimal rotation at the current sensor origin.
    """
    T = np.asarray(pose, dtype=float).reshape(4, 4)
    R, t = T[:3, :3], T[:3, 3]
    require(np.isfinite(T).all() and
            np.allclose(T[3], [0, 0, 0, 1], atol=POSE_ATOL, rtol=0) and
            np.allclose(R.T @ R, np.eye(3), atol=POSE_ATOL, rtol=0) and
            abs(np.linalg.det(R) - 1) <= POSE_ATOL,
            'linearization pose is not a finite rigid transform')
    return np.block([[R, skew(t) @ R / rotation_length_m],
                     [np.zeros((3, 3)), R / rotation_length_m]])


def analyze_iteration(iteration, rotation_length_m, rank_rtol):
    """Keep native observations, including tagged failures, alongside the spectrum."""
    row = {key: iteration[key] for key in
           ('pairs', 'class_pairs', 'weighted_squared_residual', 'step_attempted',
            'step_finite', 'update_applied')}
    row.update(native_H=iteration['H'], linearization_pose=iteration['linearization_pose'],
               native_delta=iteration['delta'], native_gradient=iteration['g'],
               status=None, scaled_H=None, scaled_gradient=None,
               eigenvalues=None, numerical_rank=None, eigenvalue_cutoff=None,
               condition_number=None, symmetry_relative_error=None)
    if any(isinstance(value, str) for value in iteration['H']):
        row['status'] = 'nonfinite_H'
        return row
    try:
        with np.errstate(over='raise', invalid='raise', divide='raise'):
            A = tangent_map(iteration['linearization_pose'], rotation_length_m)
            H = np.asarray(iteration['H'], dtype=float).reshape(6, 6)
            magnitude = float(np.max(np.abs(H)))
            symmetry = float(np.max(np.abs(H - H.T))) / magnitude if magnitude else 0.
            row['symmetry_relative_error'] = symmetry
            if symmetry > SYMMETRY_RTOL:
                row['status'] = 'asymmetric_H'
                return row
            # Only roundoff-sized antisymmetry is averaged; no damping is added.
            scaled = A.T @ (H * .5 + H.T * .5) @ A
            scaled = scaled * .5 + scaled.T * .5
            eigenvalues = np.linalg.eigvalsh(scaled)
            if not np.isfinite(eigenvalues).all():
                raise FloatingPointError('nonfinite eigenvalues')
            row['scaled_H'] = scaled.ravel().tolist()
            row['eigenvalues'] = eigenvalues.tolist()  # Retain small negatives.
            if not any(isinstance(value, str) for value in iteration['g']):
                with np.errstate(over='ignore', invalid='ignore'):
                    gradient = A.T @ np.asarray(iteration['g'], dtype=float)
                if np.isfinite(gradient).all():
                    row['scaled_gradient'] = gradient.tolist()
            cutoff = rank_rtol * float(np.max(np.abs(eigenvalues)))
            row['eigenvalue_cutoff'] = cutoff
            if eigenvalues[0] < -cutoff:
                row['status'] = 'indefinite_H'
                return row
            row['numerical_rank'] = int(np.count_nonzero(eigenvalues > cutoff))
            row['status'] = 'zero_system' if not magnitude else 'ok'
            if row['numerical_rank'] == 6:
                condition = float(eigenvalues[-1] / eigenvalues[0])
                if math.isfinite(condition):
                    row['condition_number'] = condition
    except TraceError as error:
        row['status'] = 'invalid_linearization_pose'
        row['detail'] = str(error)
    except (FloatingPointError, OverflowError, np.linalg.LinAlgError) as error:
        row['status'] = 'numerical_failure'
        row['detail'] = str(error)
    return row


def analyze_trace(trace, summary, receipt, output_dir, rotation_length_m, rank_rtol=1e-9):
    require(math.isfinite(rotation_length_m) and rotation_length_m > 0,
            'rotation length must be finite and positive')
    require(math.isfinite(rank_rtol) and 0 < rank_rtol < 1,
            'rank relative tolerance must be finite and between zero and one')
    trace, summary, receipt, output_dir = map(Path, (trace, summary, receipt, output_dir))
    require(not os.path.lexists(output_dir), 'output directory already exists')
    validation = validate_trace(trace, summary, receipt)
    stats = {stage: dict(iterations=0, statuses=Counter(), numerical_ranks=Counter())
             for stage in ('s2s', 's2m')}
    digest = hashlib.sha256()
    # Keep one frame in memory. Failed input/analysis leaves no published output.
    with tempfile.TemporaryDirectory(prefix='.inten-conditioning-', dir=output_dir.parent) as temp:
        temp = Path(temp)
        with (temp / 'iterations.jsonl').open('w') as result, trace.open('rb') as stream:
            for raw in stream:
                digest.update(raw)
                frame = strict_json(raw)
                if frame['type'] != 'frame':
                    continue
                for stage in ('s2s', 's2m'):
                    solver = frame[stage]
                    for iteration in solver['iterations']:
                        row = analyze_iteration(iteration, rotation_length_m, rank_rtol)
                        row.update(input_index=frame['input_index'], frame_index=frame['frame_index'],
                                   stage=stage, iteration_index=iteration['index'],
                                   stop=solver['stop'], returned_pairs=solver['returned_pairs'],
                                   s2s_fallback=frame['s2s_fallback'], s2m_accepted=frame['s2m_accepted'])
                        result.write(json.dumps(row, allow_nan=False) + '\n')
                        stats[stage]['iterations'] += 1
                        stats[stage]['statuses'][row['status']] += 1
                        if row['numerical_rank'] is not None:
                            stats[stage]['numerical_ranks'][str(row['numerical_rank'])] += 1
        require(digest.hexdigest() == validation['trace_sha256'] and
                sha256(summary) == validation['summary_sha256'] and
                sha256(receipt) == validation['receipt_sha256'],
                'validated input changed during analysis')
        for stage in stats:
            require(stats[stage]['iterations'] == validation['stages'][stage]['iterations'],
                    'iteration coverage differs from validation')
        here = Path(__file__).resolve()
        report = dict(schema_version=1, exit_code=0,
                      scope='Offline normal-equation analysis only. No estimator gate, accuracy, '
                            'correspondence correctness or solver-cause finding.',
                      command=[sys.executable, *sys.argv], cwd=str(Path.cwd()),
                      implementation_sha256={here.name: sha256(here),
                                             'check_inten_trace.py': sha256(here.with_name('check_inten_trace.py'))},
                      inputs=dict(trace=str(trace), summary=str(summary), receipt=str(receipt)),
                      validation=validation,
                      convention=dict(coordinates='current_sensor_center_and_axes_at_linearization',
                                      q_order='translation_m_then_rotation_length_m_times_rotation_rad',
                                      rotation_length_m=rotation_length_m,
                                      tangent_map='A=Ad(linearization_pose)*diag(I,I/L); native_delta=A*q',
                                      normal_matrix='Hq=A.T*H*A; gq=A.T*g; no damping',
                                      rank_rtol=rank_rtol, rank_cutoff='rank_rtol*max(abs(eigenvalues))',
                                      symmetry_rtol=SYMMETRY_RTOL, rigid_pose_atol=POSE_ATOL,
                                      condition_number='lambda_max/lambda_min only for PSD numerical rank 6; otherwise null'),
                      stages=stats, iterations_sha256=sha256(temp / 'iterations.jsonl'))
        (temp / 'summary.json').write_text(json.dumps(report, indent=2, allow_nan=False) + '\n')
        output_dir.mkdir()  # Exclusive creation, including after a concurrent publication.
        for name in ('iterations.jsonl', 'summary.json'):
            os.link(temp / name, output_dir / name)
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace', 'summary', 'receipt', 'output-dir'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--rotation-length-m', required=True, type=float,
                        help='Explicit characteristic length L for rotational coordinates; no privileged default.')
    parser.add_argument('--rank-rtol', type=float, default=1e-9,
                        help='Relative numerical eigenvalue cutoff; never an estimator decision.')
    args = parser.parse_args(argv)
    try:
        analyze_trace(args.trace, args.summary, args.receipt, args.output_dir,
                      args.rotation_length_m, args.rank_rtol)
        return 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print('cannot analyze InTEn trace: ' + str(error), file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
