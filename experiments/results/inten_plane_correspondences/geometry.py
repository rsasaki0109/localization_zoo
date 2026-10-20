"""Reconstruct actual plane equations and run declared fixed-system controls."""
import json
from pathlib import Path

import numpy as np


def require(ok, message):
    if not ok:
        raise ValueError(message)


def distribution(values):
    a = np.asarray(list(values), dtype=float)
    a = a[np.isfinite(a)]
    if not len(a):
        return dict(n=0, mean=None, minimum=None, q25=None, median=None, q75=None, maximum=None)
    q = np.quantile(a, [0, .25, .5, .75, 1])
    return dict(n=len(a), mean=float(a.mean()), **dict(zip(
        ('minimum', 'q25', 'median', 'q75', 'maximum'), map(float, q))))


def equations(x, a, r):
    J = np.column_stack((a, np.cross(x, a)))
    weight = np.ones(len(r))
    np.divide(.1, np.abs(r), out=weight, where=np.abs(r) > .1)
    return J.T @ (weight[:, None] * J), J.T @ (weight * r), J, weight


def finite_update(seed, delta):
    w = delta[3:]
    angle = np.linalg.norm(w)
    R = np.eye(3)
    if angle:
        u = w / angle
        K = np.array([[0, -u[2], u[1]], [u[2], 0, -u[0]], [-u[1], u[0], 0]])
        R += np.sin(angle) * K + (1 - np.cos(angle)) * K @ K
    return R @ seed[:3, 3] + delta[:3]


def analyze_case(case, row, expected, tolerance):
    require(row['id'] == case['id'], 'case order mismatch')
    require(row['seed'] == case['seeds']['oracle_current_gt'], 'seed mismatch')
    require(row['output'] == expected['output'], 'native oracle output changed')
    require(row['features'] == expected['features'], 'features changed')
    for c, e in zip(row['first_systems'], expected['first_systems']):
        require(c == e, 'native first system changed')
    seed = np.asarray(row['seed']).reshape(4, 4)
    truth = np.asarray(case['gt_relative']).reshape(4, 4)
    step = np.linalg.norm(truth[:3, 3])
    moving = step >= .1
    projection = np.r_[truth[:3, 3], np.cross(seed[:3, 3], truth[:3, 3])] / step**2 if moving else np.zeros(6)
    H = sum(np.asarray(c['H']).reshape(6, 6) for c in row['first_systems'])
    g = sum(np.asarray(c['g']) for c in row['first_systems'])
    A = H + 1e-6 * np.eye(6)
    leverage = np.linalg.solve(A.T, projection)
    delta = np.linalg.solve(A, -g)

    def update(h, gradient):
        d = np.linalg.solve(h + 1e-6 * np.eye(6), -gradient)
        t = finite_update(seed, d)
        return dict(first_forward_fraction=float(projection @ d) if moving else None,
                    first_finite_length_ratio=float(np.linalg.norm(t) / step) if moving else None,
                    first_finite_vector_error=float(np.linalg.norm(t - truth[:3, 3]) / step) if moving else None)

    result = dict(id=row['id'], frame=case['frame'], moving=bool(moving),
                  natural_bad_length=case['labels']['s2s_bad_length'] == 'True',
                  oracle_output_length_ratio=float(np.linalg.norm(np.asarray(row['output']).reshape(4, 4)[:3, 3]) / step) if moving else None,
                  native_first=update(H, g), planes={})
    result['oracle_bad_length'] = abs(result['oracle_output_length_ratio'] - 1) > .2 if moving else False
    maxima = dict(H=0., g=0., normal=0., residual=0.)
    point_metrics = {}
    for cls, name in enumerate(('ground', 'facade'), 1):
        plane = row['planes'][cls - 1]
        target = np.asarray(plane['target'], dtype=float).reshape(-1, 4)
        source = np.asarray(plane['source'], dtype=float).reshape(-1, 4)
        require(len(plane['queries']) == len(source), 'missing source query')
        xs, normals, residuals, pcas, centroids, metrics = [], [], [], [], [], []
        degenerates = 0
        for f, query in zip(source, plane['queries']):
            accepted, ids, d2, normal, residual = query
            if not accepted:
                continue
            require(len(ids) == len(d2) and len(ids) >= 3 and d2[0] <= 1., 'invalid accepted query')
            qj = target[ids[0]]
            neighbours = [i for i, d in zip(ids, d2) if d <= 1.]
            same = next((i for i in neighbours[1:] if target[i, 3] == qj[3]), None)
            adj = next((i for i in neighbours[1:] if 1 <= abs(target[i, 3] - qj[3]) <= 2), None)
            require(same is not None and adj is not None, 'row selection failed')
            e1, e2 = qj[:3] - target[adj, :3], qj[:3] - target[same, :3]
            cross = np.cross(e1, e2)
            require(np.linalg.norm(cross) >= 1e-6, 'native triangle invalid')
            a = np.asarray(normal)
            x = seed[:3, :3] @ f[:3] + seed[:3, 3]
            normal_error = float(np.max(np.abs(cross / np.linalg.norm(cross) - a)))
            residual_error = abs(float(a @ (x - qj[:3])) - residual)
            maxima['normal'] = max(maxima['normal'], normal_error)
            maxima['residual'] = max(maxima['residual'], residual_error)
            require(normal_error < 1e-12 and residual_error < 1e-12, 'triangle reconstruction mismatch')
            points = target[neighbours, :3]
            centre = points.mean(axis=0)
            centered = points - centre
            eigenvalues, eigenvectors = np.linalg.eigh(centered.T @ centered / len(points))
            degenerate = eigenvalues[1] <= 1e-12 * eigenvalues[2]
            pa = eigenvectors[:, 0]
            if pa @ a < 0:
                pa = -pa
            if degenerate:
                degenerates += 1
                pa, centre = a, qj[:3]
            up = a if a[2] >= 0 else -a
            horizontal = qj[:2] / max(np.linalg.norm(qj[:2]), 1e-30)
            metrics.append(dict(triangle_sine=float(np.linalg.norm(cross) / np.linalg.norm(e1) / np.linalg.norm(e2)),
                pca_normal_angle_deg=float(np.degrees(np.arccos(np.clip(abs(pa @ a), 0., 1.)))),
                pca_smallest_over_trace=float(max(eigenvalues[0], 0.) / max(eigenvalues.sum(), 1e-30)),
                pca_middle_over_largest=float(eigenvalues[1] / max(eigenvalues[2], 1e-30)),
                up_normal_radial=float(up[:2] @ horizontal), up_normal_z=float(up[2]),
                residual_abs_m=abs(float(residual)), neighbours=len(neighbours)))
            xs.append(x); normals.append(a); residuals.append(residual)
            pcas.append((pa, qj[:3])); centroids.append(centre)
        x = np.asarray(xs).reshape(-1, 3)
        a = np.asarray(normals).reshape(-1, 3)
        r = np.asarray(residuals)
        h, gradient, J, weights = equations(x, a, r)
        native = row['first_systems'][cls]
        require(len(r) == native['n'], 'plane count mismatch')
        for key, actual in (('H', h.ravel()), ('g', gradient)):
            expected_values = np.asarray(native[key])
            maxima[key] = max(maxima[key], float(np.max(np.abs(actual - expected_values))))
            require(np.allclose(actual, expected_values, atol=tolerance['system_atol'], rtol=tolerance['system_rtol']), 'plane H/g mismatch')
        contributions = -(J @ leverage) * weights * r
        require(np.allclose(contributions.sum(), -projection @ np.linalg.solve(A, gradient), atol=1e-10), 'per-residual attribution sum mismatch')
        for m, value in zip(metrics, contributions):
            m['fixed_total_forward_fraction'] = float(value) if moving else None
        summary = dict(source_features=len(source), accepted=len(r), pca_degenerate=degenerates,
            fixed_total_forward_fraction=float(contributions.sum()) if moving else None,
            backward_residual_count=int((contributions < 0).sum()) if moving else None,
            distributions={k: distribution(m[k] for m in metrics if m[k] is not None) for k in metrics[0]} if metrics else {})
        if name == 'ground':
            summary['controls'] = {}
            pa = np.asarray([p[0] for p in pcas]).reshape(-1, 3)
            for label, centres in (('pca_through_nearest', [p[1] for p in pcas]), ('pca_through_centroid', centroids)):
                rr = np.einsum('ij,ij->i', pa, x - np.asarray(centres).reshape(-1, 3))
                hh, gg, _, _ = equations(x, pa, rr)
                control = update(H - h + hh, g - gradient + gg)
                control['ground_fixed_original_total_forward_fraction'] = float(-leverage @ gg) if moving else None
                summary['controls'][label] = control
        result['planes'][name] = summary
        point_metrics[name] = metrics
    require(np.allclose(result['native_first']['first_forward_fraction'] or 0., projection @ delta), 'native update mismatch')
    return result, maxima, point_metrics


def analyze_sequence(cases, path, expected, tolerance):
    results, pooled = [], {name: [] for name in ('ground', 'facade')}
    maxima = dict(H=0., g=0., normal=0., residual=0.)
    with Path(path).open() as stream:
        for case in cases:
            row = json.loads(next(stream))
            result, errors, metrics = analyze_case(case, row, expected[case['id']], tolerance)
            results.append(result)
            for key, value in errors.items():
                maxima[key] = max(maxima[key], value)
            for name, values in metrics.items():
                pooled[name].extend(values)
        require(not stream.read().strip(), 'extra native case')
    summary = dict(case_count=len(results), native_output_and_first_systems_exact=True,
        geometry_reconstruction_max_absolute_difference=maxima,
        plane_totals={name: dict(accepted=sum(r['planes'][name]['accepted'] for r in results),
            source_features=sum(r['planes'][name]['source_features'] for r in results),
            pca_degenerate=sum(r['planes'][name]['pca_degenerate'] for r in results),
            distributions={k: distribution(m[k] for m in values if m[k] is not None) for k in values[0]} if values else {}) for name, values in pooled.items()})
    return dict(summary=summary, cases=results)
