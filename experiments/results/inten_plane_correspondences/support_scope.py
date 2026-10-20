"""Check whether missing-row support exists outside the recorded ten neighbours.

Offline diagnostic only: same retained target features and native distance gate.
This does not run an alternative iterative solver or change any native query.
"""
from pathlib import Path
from collections import Counter
import argparse
import importlib.util
import json
import subprocess
import tempfile

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('scope_archival', W/'experiments/results/inten_natural_traces/diagnose.py')
    archival = importlib.util.module_from_spec(spec); spec.loader.exec_module(archival)
    result = dict(exit_code=0, source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip(),
        scope='Only native in-gate queries rejected for missing same/adjacent scan-line support. Counts of support on the SAME target features beyond the recorded ten candidates; no new solver result.',
        distance='Float target/query coordinates, squared Euclidean distance <=1, original nearest point excluded from same-line candidates. Native row gaps1..2 and triangle norm>=1e-6. No fitted threshold.',
        limits='A constructible extra triangle does not establish a correct surface or an improved solve. Retrospective development diagnosis; native outputs remain unchanged.', sequences={})
    for seq in ('01', '02', '10'):
        totals = {name: Counter() for name in ('ground', 'facade')}
        cases = {}
        with tempfile.TemporaryDirectory() as temporary:
            temporary = Path(temporary)
            archival.restore(E/'sequences'/seq, temporary)
            with (temporary/'native.jsonl').open() as stream:
                for line in stream:
                    record = json.loads(line)
                    seed = np.asarray(record['seed']).reshape(4, 4)
                    case = {}
                    for name, plane in zip(('ground', 'facade'), record['planes']):
                        target = np.asarray(plane['target']).reshape(-1, 4)
                        xyz = target[:, :3].astype(np.float32)
                        rows = target[:, 3].astype(int)
                        counts = Counter()
                        row_indices = {r: np.flatnonzero(rows == r) for r in set(rows)}
                        for source, query in zip(plane['source'], plane['queries']):
                            accepted, ids, distances, normal, residual = query
                            if accepted or len(ids) < 3 or distances[0] > 1.:
                                continue
                            qj = ids[0]; row = rows[qj]
                            old = [i for i, d in zip(ids[1:], distances[1:]) if d <= 1.]
                            same = next((i for i in old if rows[i] == row), None)
                            adj = next((i for i in old if 1 <= abs(rows[i] - row) <= 2), None)
                            if same is not None and adj is not None:
                                continue  # Native triangle degeneracy, not row absence.
                            counts['native_missing_row_rejections'] += 1
                            x = (seed[:3, :3] @ np.asarray(source[:3]) + seed[:3, 3]).astype(np.float32)

                            def closest(indices):
                                if not len(indices):
                                    return None
                                d = xyz[indices] - x
                                squared = (d[:, 0]*d[:, 0] + d[:, 1]*d[:, 1]) + d[:, 2]*d[:, 2]
                                k = int(squared.argmin())
                                return int(indices[k]) if squared[k] <= 1. else None

                            same_indices = row_indices[row]
                            full_same = closest(same_indices[same_indices != qj])
                            adj_indices = np.concatenate([row_indices.get(row + dr, np.empty(0, dtype=int)) for dr in (-2, -1, 1, 2)])
                            full_adj = closest(adj_indices)
                            if same is None:
                                counts['native_same_missing'] += 1
                                counts['same_available_beyond_ten' if full_same is not None else 'same_still_missing_in_full_target_gate'] += 1
                            if adj is None:
                                counts['native_adjacent_missing'] += 1
                                counts['adjacent_available_beyond_ten' if full_adj is not None else 'adjacent_still_missing_in_full_target_gate'] += 1
                            if full_same is not None and full_adj is not None:
                                counts['both_required_rows_available_in_full_target_gate'] += 1
                                cross = np.cross(target[qj, :3] - target[full_same, :3], target[qj, :3] - target[full_adj, :3])
                                if np.linalg.norm(cross) >= 1e-6:
                                    counts['extra_constructible_triangles'] += 1
                        case[name] = dict(counts); totals[name].update(counts)
                    cases[record['id']] = case
        original = json.loads((E/'report'/(seq+'_query_support.json')).read_text())
        for name in ('ground', 'facade'):
            expected = sum(original['totals'][name].get(k, 0) for k in
                           ('missing_same_row', 'missing_adjacent_row', 'missing_same_and_adjacent_row'))
            if totals[name]['native_missing_row_rejections'] != expected:
                raise ValueError('scope universe differs from complete native query report')
        data = dict(native_rejection_universe_matches=True, totals={name: dict(c) for name, c in totals.items()}, cases=cases)
        (args.output_dir/(seq+'_scope.json')).write_text(json.dumps(data, indent=2)+'\n')
        result['sequences'][seq] = data['totals']
        print('scope_complete', seq, flush=True)
    (args.output_dir/'summary.json').write_text(json.dumps(result, indent=2)+'\n')


if __name__ == '__main__':
    main()
