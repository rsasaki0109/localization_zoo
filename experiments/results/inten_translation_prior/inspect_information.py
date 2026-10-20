"""Inspect every inherited GT-seeded first system without scans or estimator replay."""
from pathlib import Path
import hashlib
import json
import subprocess

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]


def skew(t):
    x, y, z = t
    return np.array([[0., -z, y], [z, 0., -x], [-y, x, 0.]])


def main():
    result = dict(source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip(),
                  input_sha256={}, sequences={}, ratio=.03,
                  scope='All inherited GT-seeded first systems; diagnostic inputs, not full-trajectory accuracy or a calibrated rejection rule.')
    for seq in ('01', '02', '10'):
        folder = W/'experiments/results/inten_seed_intervention/sequences'/seq
        paths = [folder/name for name in ('cases.json', 'native.stdout')]
        result['input_sha256'].update({str(p.relative_to(W)): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths})
        cases = {c['id']: c for c in json.loads(paths[0].read_text())['cases']}
        rows = []
        for line in paths[1].read_text().splitlines():
            native = json.loads(line)
            if native['mode'] != 'oracle_current_gt':
                continue
            case = cases[native['id']]
            T = np.array(native['seed']).reshape(4, 4)
            truth = np.array(case['gt_relative']).reshape(4, 4)
            length = np.linalg.norm(truth[:3, 3])
            H = sum(np.array(c['H']).reshape(6, 6) for c in native['first_systems'])
            g = sum(np.array(c['g']) for c in native['first_systems'])
            B = np.eye(6); B[:3, 3:] = skew(T[:3, 3])
            centered = B.T @ H @ B
            S = centered[:3, :3] - centered[:3, 3:] @ np.linalg.solve(centered[3:, 3:] + 1e-6*np.eye(3), centered[3:, :3])
            values, vectors = np.linalg.eigh((S+S.T)/2)
            if not (np.isfinite(values).all() and values[-1] > 0 and values[0] > -1e-7):
                raise ValueError('Invalid translation information: '+native['id'])
            weak = vectors[:, values < .03*values[-1]]
            penalty = (vectors*np.maximum(.03*values[-1]-values, 0)) @ vectors.T
            J = np.column_stack((np.eye(3), -skew(T[:3, 3])))
            delta = np.linalg.solve(H + 1e-6*np.eye(6), -g)
            adjusted = np.linalg.solve(H + J.T@penalty@J + 1e-6*np.eye(6), -g)
            direction = truth[:3, 3]/length if length >= .1 else None
            rows.append(dict(id=native['id'], frame=case['frame'], moving=direction is not None,
                eigenvalues=values.tolist(), smallest_over_largest=float(values[0]/values[-1]),
                weak_dimensions=weak.shape[1], forward_in_weak_subspace=float(np.linalg.norm(weak.T@direction)) if direction is not None else None,
                first_forward_fraction=float(direction @ J @ delta / length) if direction is not None else None,
                prior_first_forward_fraction=float(direction @ J @ adjusted / length) if direction is not None else None,
                class_pairs=[c['n'] for c in native['first_systems']]))
        if len(rows) != len(cases):
            raise ValueError('Missing case')
        moving = [r for r in rows if r['moving']]
        smaller = sum(abs(r['prior_first_forward_fraction']) < abs(r['first_forward_fraction']) for r in moving)
        result['sequences'][seq] = dict(cases=len(rows), moving=len(moving), smaller_absolute_first_forward=smaller,
            larger_absolute_first_forward=sum(abs(r['prior_first_forward_fraction']) > abs(r['first_forward_fraction']) for r in moving), cases_detail=rows)
    output = E/'information.json'
    if output.exists():
        raise ValueError('Refusing overwrite')
    output.write_text(json.dumps(result, indent=2, allow_nan=False)+'\n')


if __name__ == '__main__':
    main()
