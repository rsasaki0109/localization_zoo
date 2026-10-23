"""Complete factorial controls, all actual first queries and offline errors."""
from collections import defaultdict
import json
import numpy as np
from common import E, R, read, write
from consensus import select


def skew(v):
    x, y, z = v
    return np.array([[0., -z, y], [z, 0., -x], [-y, x, 0.]])


def main():
    protocol = read(E/'protocol.json')
    parent = list(map(json.loads, (R/'inputs/profiles.jsonl').read_text().splitlines()))
    old = {(v['frame'], v['mode']): v for v in parent if v['type'] == 'terminal'}
    native = list(map(json.loads, (R/'stage/native.stdout.jsonl').read_text().splitlines()))
    expected = [(i, m) for i in protocol['frames'] for m in range(5)]
    assert [(v['frame'], v['mode']) for v in native] == expected
    terminal = {(v['frame'], v['mode']): v for v in native}
    residuals = defaultdict(list)
    with (R/'stage/residuals.jsonl').open() as stream:
        for line in stream:
            row = json.loads(line)
            assert (row['frame'], row['mode']) in expected and row['class'] in range(4)
            residuals[(row['frame'], row['mode'], row['class'])].append(row)
    truth = {v['frame']: np.array(v['gt_relative']).reshape(4, 4) for v in read(R/'inputs/report.json')['cases']}
    decisions = read(R/'stage/consensus.json')['frames']
    maxima = dict(H_relative=0., g_relative=0., prior_system_relative=0., step_backward=0.)
    cases, counters = [], {str(m): {key: dict(lower=0, higher=0, equal=0) for key in ('length_error', 'vector_error', 'rotation_error_mrad')} for m in range(1, 5)}
    for i, selection in zip(protocol['frames'], decisions):
        assert selection['frame'] == i
        profiles = [v for v in parent if v['type'] == 'profile' and v['frame'] == i]
        assert select(profiles) == selection['decision']
        for mode, previous in ((0, 0), (4, 1)):
            current = terminal[(i, mode)]; before = old[(i, previous)]
            assert all(current[k] == before[k] for k in ('output', 'first_systems', 'solver')), ('Exact control differs', i, mode)
        for unmasked, masked in ((0, 2), (1, 3)):
            a, b = terminal[(i, unmasked)], terminal[(i, masked)]
            for c in (0, 1, 3): assert a['first_systems'][c] == b['first_systems'][c]
            # Masked first facade queries remain an ordered subset at the same seed.
            left = iter(residuals[(i, unmasked, 2)])
            for row in residuals[(i, masked, 2)]:
                target = {k: row[k] for k in ('x', 'a', 'r', 'weight')}
                assert any({k: r[k] for k in target} == target for r in left)
        arms = []
        for mode in range(5):
            row = terminal[(i, mode)]; solver = row['solver']; classes = row['first_systems']
            seed = selection['shifted_seed'] if mode in (1, 3) else selection['original_seed']
            assert solver['called'] and solver['input'] == seed and solver['output'] == row['output']
            assert solver['returned_pairs'] == sum(c['n'] for c in classes)
            for c in range(4):
                H, g = np.zeros((6, 6)), np.zeros(6)
                queries = residuals[(i, mode, c)]
                for q in queries:
                    x, a = np.array(q['x']), np.array(q['a']); J = np.r_[a, np.cross(x, a)]
                    w = 1. if abs(q['r']) <= .1 else .1/abs(q['r'])
                    assert q['weight'] == w
                    H += w*np.outer(J, J); g += w*J*q['r']
                assert len(queries) == classes[c]['n']
                eh = np.linalg.norm(H-np.array(classes[c]['H']).reshape(6, 6))/max(1., np.linalg.norm(H))
                eg = np.linalg.norm(g-np.array(classes[c]['g']))/max(1., np.linalg.norm(g))
                maxima['H_relative'] = max(maxima['H_relative'], float(eh)); maxima['g_relative'] = max(maxima['g_relative'], float(eg))
                assert eh < 1e-11 and eg < 1e-11
            Hdata = sum(np.array(c['H']).reshape(6, 6) for c in classes)
            prior = np.array(solver.get('translation_prior_information', [0.]*9)).reshape(3, 3)
            Jprior = np.c_[np.eye(3), -skew(np.array(seed).reshape(4, 4)[:3, 3])]
            reconstructed = Hdata+Jprior.T@prior@Jprior
            eh = np.linalg.norm(reconstructed-np.array(solver['iterations'][0]['H']).reshape(6, 6))/max(1., np.linalg.norm(reconstructed))
            maxima['prior_system_relative'] = max(maxima['prior_system_relative'], float(eh)); assert eh < 1e-11
            for iteration in solver['iterations']:
                if not iteration['step_attempted']: continue
                assert iteration['step_finite'] and iteration['update_applied']
                H = np.array(iteration['H']).reshape(6, 6)+1e-6*np.eye(6)
                g = np.array(iteration['g']); delta = np.array(iteration['delta'])
                eb = np.linalg.norm(H@delta+g)/max(1., np.linalg.norm(H)*np.linalg.norm(delta)+np.linalg.norm(g))
                maxima['step_backward'] = max(maxima['step_backward'], float(eb)); assert eb < 1e-10
            T = np.array(row['output']).reshape(4, 4); gt = truth[i]; length = np.linalg.norm(gt[:3, 3])
            ratio = np.linalg.norm(T[:3, 3])/length
            rotation = np.arccos(np.clip((np.trace(T[:3, :3]@gt[:3, :3].T)-1)/2, -1., 1.))*1000
            arms.append(dict(mode=mode, length_ratio=float(ratio), length_error=float(abs(ratio-1)),
                             vector_error=float(np.linalg.norm(T[:3, 3]-gt[:3, 3])/length), rotation_error_mrad=float(rotation),
                             bad_length=bool(abs(ratio-1) > .2), first_pairs=[c['n'] for c in classes]))
        for arm in arms[1:]:
            labels = {}
            for metric in ('length_error', 'vector_error', 'rotation_error_mrad'):
                delta = arm[metric]-arms[0][metric]
                label = 'equal' if abs(delta) <= 1e-12 else 'lower' if delta < 0 else 'higher'
                labels[metric] = label; counters[str(arm['mode'])][metric][label] += 1
            arm['vs_baseline'] = labels
        cases.append(dict(frame=i, decision=selection['decision'], arms=arms))
    lookup = {c['frame']: c for c in cases}
    passed = (all(lookup[204]['arms'][3][m] < lookup[204]['arms'][0][m] for m in ('length_error', 'vector_error'))
              and lookup[204]['arms'][3]['rotation_error_mrad'] <= lookup[204]['arms'][0]['rotation_error_mrad']
              and all(lookup[i]['arms'][3][m] <= lookup[i]['arms'][0][m] for i in (205, 247) for m in ('length_error', 'vector_error'))
              and not any(c['arms'][3]['bad_length'] and not c['arms'][0]['bad_length'] for c in cases))
    write(R/'analysis/report.json', dict(exit_code=0, frames=protocol['frames'], native_calls=len(native),
          original_and_previous_rejected_controls_exact=True, non_facade_first_systems_exact_within_seed=True,
          all_first_class_systems_independently_reconstructed=True, adaptive_prior_first_H_reconstructed=True,
          all_finite_steps_backward_checked=True, masked_facade_first_queries_ordered_subsets=True,
          maxima=maxima, candidate_progression_condition_passed=bool(passed), metric_counts_vs_original=counters,
          cases=cases, limits=protocol['limits']))


if __name__ == '__main__': main()
