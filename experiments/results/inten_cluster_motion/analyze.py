"""All frozen cases, exact native controls, and independent first-equation reconstruction."""
from collections import Counter, defaultdict
import json
import numpy as np
from common import E, W, R, read, write


def main():
    protocol = read(E/'protocol.json')
    assert read(R/'native_complete.json')['exit_code'] == 0
    rows = list(map(json.loads, (R/'stage/native.stdout.jsonl').read_text().splitlines()))
    terminals = {(v['frame'], v['mode']): v for v in rows if v['type'] == 'terminal'}
    profiles = defaultdict(list)
    for row in rows:
        if row['type'] == 'profile': profiles[row['frame']].append(row)
    assert list(terminals) == [(i, m) for i in protocol['frames'] for m in (0, 1)]
    traces = {v['frame']: v for v in map(json.loads, (R/'inputs/native.stdout').read_text().splitlines())}
    features = {v['frame']: v['features'] for v in map(json.loads, (R/'inputs/features.jsonl').read_text().splitlines())}
    old_classes = {}
    with (R/'inputs/first_systems.jsonl').open() as stream:
        for line in stream:
            row = json.loads(line)
            if row['mode'] == 0 and row['frame'] in protocol['frames']:
                old_classes[row['frame']] = row['first_systems']
    residuals = defaultdict(list)
    with (R/'inputs/geometry.jsonl').open() as stream:
        for line in stream:
            row = json.loads(line)
            if row['type'] == 'residual' and row['iteration'] == 0:
                residuals[(row['frame'], row['class'])].append(row)
    gt = {v['frame']: np.array(v['gt_relative']).reshape(4, 4) for v in read(R/'inputs/report.json')['cases']}
    counts = dict(lower=0, higher=0, equal=0)
    maximum_H, maximum_g = 0., 0.
    cases = []
    for i in protocol['frames']:
        baseline, candidate = [terminals[(i, m)] for m in (0, 1)]
        assert baseline['output'] == traces[i]['s2s']['output']
        assert baseline['first_systems'] == old_classes[i]
        a = {k: v for k, v in baseline['solver'].items() if k != 'returned_pairs'}
        b = {k: v for k, v in traces[i]['s2s'].items() if k != 'returned_pairs'}
        assert a == b, ('Baseline normal solver mismatch', i)
        assert baseline['solver']['returned_pairs'] == sum(v['n'] for v in old_classes[i])
        expected = sorted({int(f[4]) for f in features[i][2] if f[4] >= 2})
        assert [v['cluster'] for v in profiles[i]] == expected
        direction = np.array(traces[i]['s2s']['input']).reshape(4, 4)[:3, 3]
        if np.linalg.norm(direction): direction /= np.linalg.norm(direction)
        point_counts = Counter(f[4] for f in features[i][4])
        for row in profiles[i]:
            assert np.allclose(direction, row['direction'], atol=1e-15, rtol=1e-15)
            assert row['points'] == point_counts[row['cluster']]
            costs = row['costs_m2']
            assert len(costs) == (121 if row['points'] and np.linalg.norm(direction) else 0)
            assert all(np.isfinite(c) and c >= 0 for c in costs)
            best = min(range(len(costs)), key=lambda k: (costs[k], abs(k-60), k)) if costs else 60
            excluded = bool(costs and 0 < best < 120 and abs((best-60)*.05) > 1. and costs[best] < .5*costs[60])
            assert row['best_index'] == best and row['excluded'] == excluded
        mask = {v['cluster'] for v in profiles[i] if v['excluded']}
        for c in (0, 1, 3): assert candidate['first_systems'][c] == baseline['first_systems'][c]
        for mode, terminal in enumerate((baseline, candidate)):
            classes = terminal['first_systems']
            for c in range(4):
                H, g, n = np.zeros((6, 6)), np.zeros(6), 0
                for row in residuals[(i, c)]:
                    if mode and c == 2 and row['source'][4] in mask: continue
                    x, a = np.array(row['x']), np.array(row['a'])
                    J = np.r_[a, np.cross(x, a)]
                    w = 1. if abs(row['r']) <= .1 else .1/abs(row['r'])
                    assert row['weight'] == w
                    H += w*np.outer(J, J); g += w*J*row['r']; n += 1
                observed = classes[c]
                assert observed['n'] == n
                h_error = np.linalg.norm(H-np.array(observed['H']).reshape(6, 6))/max(1., np.linalg.norm(H))
                g_error = np.linalg.norm(g-np.array(observed['g']))/max(1., np.linalg.norm(g))
                maximum_H, maximum_g = max(maximum_H, h_error), max(maximum_g, g_error)
                assert h_error < 1e-11 and g_error < 1e-11
            for iteration in terminal['solver']['iterations']:
                if not iteration['step_attempted']: continue
                assert iteration['step_finite'] and iteration['update_applied']
                H = np.array(iteration['H']).reshape(6, 6)+1e-6*np.eye(6)
                g = np.array(iteration['g']); delta = np.array(iteration['delta'])
                backward = np.linalg.norm(H@delta+g)/max(1., np.linalg.norm(H)*np.linalg.norm(delta)+np.linalg.norm(g))
                assert backward < 1e-10
        errors = []
        for terminal in (baseline, candidate):
            T = np.array(terminal['output']).reshape(4, 4); truth = gt[i]; length = np.linalg.norm(truth[:3, 3])
            rotation = np.arccos(np.clip((np.trace(T[:3, :3]@truth[:3, :3].T)-1)/2, -1., 1.))*1000
            ratio = np.linalg.norm(T[:3, 3])/length
            errors.append(dict(length_ratio=float(ratio), length_error=float(abs(ratio-1)),
                               vector_error=float(np.linalg.norm(T[:3, 3]-truth[:3, 3])/length),
                               rotation_error_mrad=float(rotation), first_pairs=[v['n'] for v in terminal['first_systems']]))
        difference = errors[1]['length_error']-errors[0]['length_error']
        label = 'equal' if abs(difference) <= 1e-12 else 'lower' if difference < 0 else 'higher'
        counts[label] += 1
        cases.append(dict(frame=i, profiled_clusters=len(profiles[i]), excluded_clusters=sorted(mask),
                          excluded_source_facades=sum(f[4] in mask for f in features[i][2]),
                          baseline=errors[0], candidate=errors[1], length_error_vs_baseline=label))
    write(R/'analysis/report.json', dict(exit_code=0, frames=protocol['frames'], native_calls=len(terminals),
          profiled_clusters=sum(map(len, profiles.values())), baseline_outputs_first_systems_and_full_solver_exact=True,
          forced_return_count_independently_checked=True, non_facade_first_equations_exact=True,
          all_first_equations_independently_reconstructed=True, maximum_H_relative_error=maximum_H,
          maximum_g_relative_error=maximum_g, all_finite_updates_backward_checked=True,
          length_error_counts=counts, cases=cases,
          limits=protocol['limits']))


if __name__ == '__main__':
    main()
