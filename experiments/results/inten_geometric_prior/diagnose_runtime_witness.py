#!/usr/bin/env python3
"""Reproduce the failing archived facade equation without changing tolerances."""
import argparse
import gzip
import hashlib
import importlib.util
import json
import lzma
from pathlib import Path
import sys
import numpy as np
from receipt import save, sha


def main():
    p = argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root', type=Path, required=True)
    p.add_argument('--fixture', type=Path, required=True)
    p.add_argument('--extract', action='store_true')
    p.add_argument('--output', type=Path)
    args = p.parse_args()
    study = args.source_root / 'experiments/results/inten_support_information'
    key = (46, 14, False, 1, 2)
    if args.extract:
        args.fixture.mkdir(parents=True, exist_ok=False)
        folder = study / 'groups/08'
        meta = json.loads((folder / 'archive.json').read_text())['files']['support.extra.jsonl']
        path = folder / meta['path']
        assert sha(path) == meta['stored_sha256']
        features = {}; query = None
        with lzma.open(path, 'rt') as stream:
            for line in stream:
                raw = json.loads(line)['raw_line']
                if '"type":"support_features"' in raw[:80]:
                    row = json.loads(raw)
                    if row['frame'] in (45, 46):
                        assert row['frame'] not in features
                        features[row['frame']] = np.asarray(row['classes'][2], dtype=float).reshape(-1, 6)
                elif '"type":"support_query"' in raw[:80] and '"frame":46,' in raw[:160]:
                    row = json.loads(raw)
                    if tuple(row[k] for k in ('frame','mode','translation_only','iteration','class')) == key:
                        assert query is None
                        query = raw
        assert query is not None and set(features) == {45, 46}
        (args.fixture / 'query.json').write_text(query + ('' if query.endswith('\n') else '\n'))
        np.savez_compressed(args.fixture / 'clouds.npz', source=features[46], target=features[45])
        save(args.fixture / 'manifest.json', {
            'selection': 'The first strict numerical failure in the complete serial verification, 08/46/mode14/joint/iteration1/facade. Diagnostic only; no threshold or estimator change.',
            'archive_sha256': sha(path), 'key': list(key),
            'query_sha256': sha(args.fixture / 'query.json'),
            'clouds_sha256': sha(args.fixture / 'clouds.npz'),
        })
        return 0
    assert args.output is not None
    import scipy
    manifest = json.loads((args.fixture / 'manifest.json').read_text())
    assert sha(args.fixture / 'query.json') == manifest['query_sha256']
    assert sha(args.fixture / 'clouds.npz') == manifest['clouds_sha256']
    spec = importlib.util.spec_from_file_location('frozen_runtime_witness', study / 'verify_mixed_parallel.py')
    verifier = importlib.util.module_from_spec(spec); spec.loader.exec_module(verifier)
    verifier.bindings()
    query = json.loads((args.fixture / 'query.json').read_text())
    data = np.load(args.fixture / 'clouds.npz'); blank = np.empty((0, 6))
    clouds = {46: [blank, blank, data['source'], blank], 45: [blank, blank, data['target'], blank]}
    namespace = verifier.base.base.namespace
    original = namespace['close']; observations = []
    def observed(a, b, label):
        left, right = np.asarray(a), np.asarray(b)
        difference = np.abs(left - right)
        observations.append({'label': label, 'actual': left.tolist(), 'expected': right.tolist(),
                             'max_abs_error': float(np.max(difference)),
                             'max_tolerance_fraction': float(np.max(difference / (1e-12 + 1e-10 * np.abs(right)))),
                             'within_original_tolerance': bool(np.allclose(left, right, rtol=1e-10, atol=1e-12))})
        original(a, b, label)
    namespace['close'] = observed
    report = {'scope': 'Single failed-query runtime comparison, not full verification or trajectory accuracy.',
              'python': sys.version, 'executable': sys.executable,
              'numpy': np.__version__, 'scipy': scipy.__version__,
              'fixture_manifest_sha256': sha(args.fixture / 'manifest.json'),
              'passed': False, 'observations': observations}
    try:
        verifier.checker.reconstruct(query, clouds, set(), {}, {})
        report['passed'] = True
    except Exception as error:
        report['error'] = repr(error)
    finally:
        namespace['close'] = original
    save(args.output, report)
    print(json.dumps({k: report[k] for k in ('numpy', 'scipy', 'passed')}))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
