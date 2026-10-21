"""Run every frozen cluster/region terminal ablation; retain all outcomes."""
import json
from pathlib import Path
import numpy as np
from run import E, W, R, PY, build, read, write, sha, run, module


def main():
    p = read(E/'ablation_protocol.json')
    assert sha(E/'ablation_native.cpp') == p['ablation_native_sha256']
    stage = R/'ablations'; stage.mkdir()
    command = read(R/'staging/compile_native_command.json')['command']
    command = [str(E/'ablation_native.cpp') if a == str(E/'diagnostic_native.cpp') else
               str(R/'ablation.o') if a == str(R/'native.o') else a for a in command]
    assert run('compile_native', command, stage) == 0
    command = read(R/'staging/compile_probe_command.json')['command']
    command = [str(E/'ablation_probe.cpp') if a == str(E/'probe.cpp') else
               str(R/'ablation.o') if a == str(R/'native.o') else
               str(R/'ablation_probe') if a == str(R/'probe') else a for a in command]
    assert run('compile_probe', command, stage) == 0
    write(stage/'binary.json', dict(path=str(R/'ablation_probe'), sha256=sha(R/'ablation_probe'),
          object_sha256=sha(R/'ablation.o'), linked_static_libraries=read(R/'staging/binary.json')['linked_static_libraries']))
    traces = {r['frame']: r for r in map(json.loads, (R/'staging/native.stdout').read_text().splitlines())}
    features = {r['frame']: r['features'] for r in map(json.loads, (R/'staging/features.jsonl').read_text().splitlines())}
    baseline_report = read(R/'staging/report.json')
    raw = R/'kitti/dataset/sequences/01/velodyne'
    expected = []
    lines = []
    for case in baseline_report['cases']:
        i = case['frame']
        clusters = sorted(set(int(f[4]) for f in features[i][2]))
        arms = [(0, -1), *[(1, c) for c in clusters], (2, -1), (3, -1)]
        for name, seed in [('natural', traces[i]['s2s']['input']), ('oracle', case['gt_relative'])]:
            for mode, cluster in arms:
                expected.append([i, name, mode, cluster])
                values = ' '.join(format(float(v), '.17g') for v in seed)
                lines.append(f'{i} {name} {mode} {cluster} {raw/(str(i-1).zfill(6)+".bin")} {raw/(str(i).zfill(6)+".bin")} {values}')
    payload = ('\n'.join(lines)+'\n').encode(); (stage/'input.txt').write_bytes(payload)
    assert run('native', build.docker()+[str(R/'ablation_probe')], stage, payload) == 0
    assert sha(R/'ablation_probe') == read(stage/'binary.json')['sha256']
    rows = list(map(json.loads, (stage/'native.stdout').read_text().splitlines()))
    assert [[r['frame'], r['seed_name'], r['mode'], r['cluster']] for r in rows] == expected
    report = []
    for case in baseline_report['cases']:
        i = case['frame']; gt = np.array(case['gt_relative']).reshape(4, 4)
        length = np.linalg.norm(gt[:3, 3]); entry = dict(frame=i, seeds=[])
        for seed in ('natural', 'oracle'):
            selected = [r for r in rows if r['frame'] == i and r['seed_name'] == seed]
            base = selected[0]
            if seed == 'natural':
                assert base['output'] == traces[i]['s2s']['output'], ('pair control diverged', i)
                assert [s['n'] for s in base['first_systems']] == traces[i]['s2s']['iterations'][0]['class_pairs']
            out = []
            for row in selected:
                for cls in (0, 1, 3):
                    assert row['first_systems'][cls] == base['first_systems'][cls]
                T = np.array(row['output']).reshape(4, 4)
                out.append(dict(mode=row['mode'], cluster=row['cluster'],
                    length_ratio=float(np.linalg.norm(T[:3, 3])/length),
                    length_error=float(abs(np.linalg.norm(T[:3, 3])/length-1)),
                    vector_error=float(np.linalg.norm(T[:3, 3]-gt[:3, 3])/length),
                    first_pairs=[s['n'] for s in row['first_systems']]))
            b = out[0]
            counts = dict(lower=0, higher=0, equal=0)
            for v in out[1:]:
                for metric in ('length_error', 'vector_error'):
                    d = v[metric]-b[metric]; label = 'equal' if abs(d) <= 1e-12 else 'lower' if d < 0 else 'higher'
                    v[metric+'_vs_base'] = label
                if v['mode'] == 1: counts[v['length_error_vs_base']] += 1
            entry['seeds'].append(dict(seed=seed, individual_cluster_length_error_counts=counts, arms=out))
        report.append(entry)
    write(stage/'report.json', dict(exit_code=0, native_calls=len(rows),
          natural_pair_outputs_exact=True, non_facade_first_equations_exact=True, cases=report,
          limits='Terminal conditional S2S sensitivity only. Native adaptive prior changes with removed facade H; first non-facade data equations remain exact. Ground truth is used only by the explicit oracle diagnostic and offline errors. No recursive trajectory, full-sequence accuracy, semantic-object label, held-out or deployable setting claim.'))
    archive = module('ablation_archive', W/'experiments/results/inten_natural_traces/run_sequences.py')
    archive.archive(stage, E/'ablations')
    write(R/'ablations_complete.json', dict(exit_code=0))


if __name__ == '__main__':
    main()
