#!/usr/bin/env python3
"""GT-free, opt-in fixed input/objective probe of archived coarse estimates."""
import argparse
import importlib.metadata
import json
from pathlib import Path

import numpy as np
import small_gicp
from refine_pairs import sha


def elevation(points, degrees):
    out = np.array(points, dtype=np.float64, copy=True)
    if degrees == 0:
        return out
    angle = np.radians(degrees)
    horizontal = np.linalg.norm(out[:, :2], axis=1)
    valid = horizontal > 1e-12
    corrected = horizontal[valid]*np.cos(angle)-out[valid, 2]*np.sin(angle)
    out[valid, :2] *= (corrected/horizontal[valid])[:, None]
    out[valid, 2] = out[valid, 2]*np.cos(angle)+horizontal[valid]*np.sin(angle)
    return out


def main():
    parser = argparse.ArgumentParser(__doc__)
    for name in ('protocol', 'raw-root', 'output'):
        parser.add_argument('--'+name, type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise FileExistsError(args.output)
    p = json.loads(args.protocol.read_text())
    study = args.protocol.resolve().parent
    assert sha(study/'matcher_protocol.json') == p['pair_protocol_sha256']
    assert sha(study/'refinement_protocol.json') == p['refinement_protocol_sha256']
    pairs = json.loads((study/'matcher_protocol.json').read_text())
    refinement = json.loads((study/'refinement_protocol.json').read_text())
    assert importlib.metadata.version('small-gicp') == p['small_gicp_version']
    rows = []
    for case in pairs['cases']:
        raw = {}
        for name in ('source', 'target'):
            pin = pairs['inputs'][f"{case[name+'_frame']:06d}.bin"]
            path = args.raw_root/pin['name']
            assert sha(path) == pin['sha256'] and path.stat().st_size == pin['bytes']
            raw[name] = np.fromfile(path, dtype='<f4').reshape(-1, 4)[:, :3]
        for degrees in p['elevation_degrees']:
            clouds = {name: elevation(x, degrees) for name, x in raw.items()}
            for objective in p['objectives']:
                config = dict(refinement['config'], registration_type=objective)
                for variant in pairs['variants']:
                    name = variant['id']+'__'+case['id']+'.estimate.json'
                    coarse = study/'matcher_run'/name
                    assert sha(coarse) == refinement['coarse_sha256'][name]
                    row = {'case': case['id'], 'variant': variant['id'],
                           'elevation_degrees': degrees, 'objective': objective}
                    try:
                        result = small_gicp.align(clouds['target'], clouds['source'],
                            init_T_target_source=np.array(json.loads(coarse.read_text())['transform']), **config)
                        transform = np.asarray(result.T_target_source)
                        assert np.isfinite(transform).all()
                        row.update(transform=transform.tolist(), converged=bool(result.converged),
                                   num_inliers=int(result.num_inliers), iterations=int(result.iterations))
                    except Exception as error:
                        row['error'] = repr(error)
                    rows.append(row)
                    args.output.write_text(json.dumps({'protocol_sha256': sha(args.protocol),
                        'backend_sha256': sha(Path(small_gicp.__file__)), 'cases': rows}, indent=2, allow_nan=False)+'\n')
    assert len(rows) == len(pairs['cases'])*len(pairs['variants'])*len(p['elevation_degrees'])*len(p['objectives'])
    assert not any('error' in row for row in rows), 'some native fits failed; all outcomes retained'


if __name__ == '__main__':
    main()
