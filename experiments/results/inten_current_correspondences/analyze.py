"""Validate actual-prefix parity, reconstruct every system, retain cluster contributions."""
from pathlib import Path
from collections import Counter, defaultdict
import json
import tempfile
import numpy as np

from run import E, W, R, module, read, write


def skew(v):
    x, y, z = v
    return np.array([[0., -z, y], [z, 0., -x], [-y, x, 0.]])


def equation(row):
    x, a = np.array(row['x']), np.array(row['a'])
    J = np.r_[a, np.cross(x, a)]
    return row['weight']*np.outer(J, J), row['weight']*J*row['r']


def bounds(points):
    p = np.asarray(points)
    return dict(n=len(p), minimum=p[:, :3].min(axis=0).tolist(),
                maximum=p[:, :3].max(axis=0).tolist(), mean=p[:, :3].mean(axis=0).tolist())


def main():
    stage = R/'staging'
    traces = {r['frame']: r for r in map(json.loads, (stage/'native.stdout').read_text().splitlines())}
    frames = read(E/'protocol.json')['frames']
    archive = module('geometry_restore', W/'experiments/results/inten_natural_traces/diagnose.py')
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp = Path(temp)
        archive.restore(W/'experiments/results/inten_facade_pca/sequences/01', temp)
        previous = {}
        for line in (temp/'candidate/trace.jsonl').read_text().splitlines()[1:]:
            r = json.loads(line)
            if r['type'] == 'frame' and r['frame_index'] in traces:
                previous[r['frame_index']] = r
        summary = read(temp/'candidate/summary.json')
        a = np.loadtxt(temp/'candidate'/summary['artifacts']['evaluated_gt_path']).reshape(-1, 3, 4)
        gt = np.tile(np.eye(4), (len(a), 1, 1)); gt[:, :3, :] = a
        for i, row in traces.items():
            for field in ('pose', 's2s', 's2m'):
                assert row[field] == previous[i][field], (i, field, 'normal prefix changed')
    features = {r['frame']: r['features'] for r in map(json.loads, (stage/'features.jsonl').read_text().splitlines())}
    feature_sets = {f: [set(map(tuple, fs)) for fs in classes] for f, classes in features.items()}
    residuals, planes = defaultdict(list), defaultdict(list)
    with (stage/'geometry.jsonl').open() as stream:
        for row in map(json.loads, stream):
            key = (row['frame'], row['iteration'])
            (residuals if row['type'] == 'residual' else planes)[key].append(row)
    report = []
    maxima = dict(H_relative=0., g_absolute=0., residual_consistency=0., step_backward_error=0.)
    for i in frames:
        s = traces[i]['s2s']
        relative = np.linalg.inv(gt[i-1])@gt[i]
        direction = relative[:3, 3]/np.linalg.norm(relative[:3, 3])
        length = float(np.linalg.norm(relative[:3, 3]))
        iterations = []
        for it, native in enumerate(s['iterations']):
            rows = residuals[(i, it)]
            Hc = np.zeros((4, 6, 6)); gc = np.zeros((4, 6)); count = np.zeros(4, dtype=int)
            clusterH, clusterg = defaultdict(lambda: np.zeros((6, 6))), defaultdict(lambda: np.zeros(6))
            for r in rows:
                c = r['class']; h, g = equation(r)
                Hc[c] += h; gc[c] += g; count[c] += 1
                if c == 2:
                    clusterH[r['source'][4]] += h; clusterg[r['source'][4]] += g
            assert count.tolist() == native['class_pairs']
            T = np.array(native['linearization_pose']).reshape(4, 4)
            J = np.c_[np.eye(3), -skew(T[:3, 3])]
            info = np.array(s.get('translation_prior_information', np.zeros(9))).reshape(3, 3)
            hprior = J.T@info@J
            gprior = J.T@info@(T[:3, 3]-np.array(s['input']).reshape(4, 4)[:3, 3])
            H, g = Hc.sum(axis=0)+hprior, gc.sum(axis=0)+gprior
            expectedH, expectedg = np.array(native['H']).reshape(6, 6), np.array(native['g'])
            errH = float(np.linalg.norm(H-expectedH)/max(1., np.linalg.norm(expectedH)))
            errg = float(np.max(np.abs(g-expectedg)))
            maxima['H_relative'] = max(maxima['H_relative'], errH)
            maxima['g_absolute'] = max(maxima['g_absolute'], errg)
            assert errH < 1e-10 and np.allclose(g, expectedg, rtol=1e-10, atol=1e-8)
            A = expectedH+1e-6*np.eye(6)
            delta = np.array(native['delta'])
            backward = float(np.linalg.norm(A@delta+expectedg)/(np.linalg.norm(A)*np.linalg.norm(delta)+np.linalg.norm(expectedg)))
            maxima['step_backward_error'] = max(maxima['step_backward_error'], backward)
            assert backward < 1e-10
            def contribution(v):
                return float(direction@J@np.linalg.solve(A, -v)/length*100)
            first_planes = planes[(i, it)]
            assert Counter(r['class'] for r in first_planes) == Counter({1: int(count[1]), 2: int(count[2])})
            bycluster = defaultdict(list)
            for p in first_planes:
                c = p['class']
                src = p['source']; n = p['neighbours']; q = np.array(n[0][2][:3]); normal = np.array(p['normal'])
                assert tuple(src) in feature_sets[i][c]
                for index, distance, target in n:
                    assert target == features[i-1][c][index]
                    # FLANN uses float squared distances; coordinates are native doubles.
                    assert distance >= 0
                assert p['same'] in [v[0] for v in n] and p['adjacent'] in [v[0] for v in n]
                err = abs(float(normal@(np.array(p['x'])-q))-p['r'])
                maxima['residual_consistency'] = max(maxima['residual_consistency'], err)
                assert err < 1e-10
                if c == 2:
                    bycluster[src[4]].append(p)
            clusters = []
            for cluster, ps in sorted(bycluster.items()):
                target_counts = Counter(p['neighbours'][0][2][4] for p in ps)
                static_residuals = []
                forward_information = 0.
                for p in ps:
                    n = np.array(p['normal']); q = np.array(p['neighbours'][0][2][:3])
                    xgt = relative[:3, :3]@p['source'][:3]+relative[:3, 3]
                    static_residuals.append(abs(float(n@(xgt-q))))
                    forward_information += float(n@direction)**2
                clusters.append(dict(source_cluster=cluster, pairs=len(ps),
                    first_system_forward_percent=contribution(clusterg[cluster]),
                    conditional_forward_information=float(direction@clusterH[cluster][:3, :3]@direction),
                    unweighted_forward_information=forward_information,
                    source_bounds=bounds([p['source'] for p in ps]),
                    target_cluster_counts=dict(sorted(target_counts.items())),
                    target_anchor_bounds=bounds([p['neighbours'][0][2] for p in ps]),
                    gt_fixed_support_abs_residual_m=dict(mean=float(np.mean(static_residuals)), median=float(np.median(static_residuals)), maximum=float(max(static_residuals)))))
            iterations.append(dict(iteration=it, pairs=count.tolist(),
                class_forward_percent=[contribution(v) for v in gc], prior_forward_percent=contribution(gprior),
                all_first_forward_percent=contribution(expectedg),
                native_linearization_length_ratio=float(np.linalg.norm(T[:3, 3])/length),
                clusters=clusters))
        report.append(dict(frame=i, gt_relative=relative.reshape(-1).tolist(), gt_length_m=length,
            s2s_input_length_ratio=float(np.linalg.norm(np.array(s['input']).reshape(4, 4)[:3, 3])/length),
            s2s_output_length_ratio=float(np.linalg.norm(np.array(s['output']).reshape(4, 4)[:3, 3])/length),
            iterations=iterations))
    write(stage/'report.json', dict(exit_code=0, all_continuous_solver_traces_and_poses_exact=True,
        prefix_frames=len(traces), checks=maxima, cases=report,
        interpretation_limits=read(E/'protocol.json')['analysis']))


if __name__ == '__main__':
    main()
