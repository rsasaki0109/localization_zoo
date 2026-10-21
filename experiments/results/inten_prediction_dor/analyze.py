"""Full-sequence detector coverage and conditional first-system contributions.

All GT labels and the inherited hand region are offline diagnostics only.
Neither is used by the producer or an automatic decision.
"""
from pathlib import Path
from collections import defaultdict, Counter
from itertools import groupby
import json
import tempfile
import numpy as np
from run import E, W, R, read, write, module


def skew(v):
    x, y, z = v
    return np.array([[0., -z, y], [z, 0., -x], [-y, x, 0.]])


def distribution(values):
    return dict(n=len(values), minimum=min(values) if values else None,
                median=float(np.median(values)) if values else None, maximum=max(values) if values else None)


def main():
    p = read(E/'protocol.json'); stage = R/'staging'
    traces = {r['frame_index']: r for r in map(json.loads, (stage/'native.stdout').read_text().splitlines())}
    restore = module('dor_restore', W/'experiments/results/inten_natural_traces/diagnose.py').restore
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp = Path(temp); restore(W/'experiments/results/inten_facade_pca/sequences/01', temp)
        previous = {}
        for row in map(json.loads, (temp/'candidate/trace.jsonl').read_text().splitlines()):
            if row['type'] == 'frame': previous[row['frame_index']] = row
        assert traces.keys() == previous.keys()
        old_root = previous[0]['scan_path'].removesuffix('/000000.bin/cloud.pcd')
        raw = R/'kitti/dataset/sequences/01/velodyne'
        for i, row in traces.items():
            expected = previous[i]
            assert row['scan_path'] == str(raw/f'{i:06d}.bin')
            assert expected['scan_path'] == old_root+f'/{i:06d}.bin/cloud.pcd'
            assert row['anchored_pose'] == row['pose']
            assert {k:v for k,v in row.items() if k not in ('scan_path','anchored_pose')} == {k:v for k,v in expected.items() if k not in ('scan_path','anchored_pose')}, (i, 'native frame changed')
        summary = read(temp/'candidate/summary.json')
        a = np.loadtxt(temp/'candidate'/summary['artifacts']['evaluated_gt_path']).reshape(-1, 3, 4)
        gt = np.tile(np.eye(4), (len(a), 1, 1)); gt[:, :3, :] = a
    masks = {}
    for r in map(json.loads, (stage/'masks.jsonl').read_text().splitlines()):
        key = (r['frame'], r['stage']); assert key not in masks
        for c, total, covered, flagged, dynamic, nf, ne in r['clusters']:
            assert 0 <= flagged <= covered <= total and total > 0
            assert dynamic == int(flagged >= .5*total)
        masks[key] = r
    assert set(masks) == {(i, s) for i in range(1, len(traces)) for s in range(3)}
    bycluster = {key: {c[0]: c for c in value['clusters']} for key,value in masks.items()}
    for i in range(1, len(traces)):
        assert masks[i, 0]['pose'] == masks[i, 1]['pose']
        assert masks[i, 2]['pose'] == traces[i]['pose']
        assert sum(c[1] for c in masks[i, 2]['clusters'] if c[4]) == traces[i]['dor_removed']
    # Independently recover every native coverage/flag count from detailed point records.
    counts = defaultdict(lambda: [0, 0, 0])
    with (stage/'ranges.jsonl').open() as stream:
        for line in stream:
            frame, s, f, pixel, distance, rendered = json.loads(line)
            c = f[4]; v = counts[frame, s, c]
            v[0] += 1; v[1] += int(pixel >= 0 and rendered > 0)
            v[2] += int(pixel >= 0 and rendered > 0 and abs(rendered-distance) > .1*distance)
    for key, value in counts.items():
        frame, s, c = key
        assert value == bycluster[frame, s][c][1:4], key
    assert {f for f, s, c in counts} == set(p['detailed_frames'])
    frame_rows = []; cluster_output = stage/'cluster_contributions.jsonl'
    with cluster_output.open('x') as cout, (stage/'correspondences.jsonl').open() as stream:
        grouped = groupby(map(json.loads, stream), key=lambda r: r['frame'])
        seen = []
        for i, queries_iter in grouped:
            seen.append(i); queries = list(queries_iter); native = traces[i]['s2s']; first = native['iterations'][0]
            assert len(queries) == first['class_pairs'][2]
            T = np.array(first['linearization_pose']).reshape(4, 4)
            J = np.c_[np.eye(3), -skew(T[:3, 3])]
            A = np.array(first['H']).reshape(6, 6)+1e-6*np.eye(6)
            relative = np.linalg.inv(gt[i-1])@gt[i]; length = float(np.linalg.norm(relative[:3, 3]))
            direction = relative[:3, 3]/length if length > 0 else np.zeros(3)
            # Linear functional through the same joint native damped Hessian.
            functional = np.linalg.solve(A.T, J.T@direction)/max(length, 1e-30)*100
            classes = defaultdict(lambda: dict(pairs=0, first_forward_percent=0.))
            cluster_stats = defaultdict(lambda: dict(pairs=0, first_forward_percent=0.))
            srcpred = bycluster[i, 0]; srcfuture = bycluster[i, 1]; srcfinal = bycluster[i, 2]
            target = bycluster.get((i-1, 2), {})
            def marked(table, cluster): return bool(table.get(cluster, [0,0,0,0,0])[4])
            for q in queries:
                x, normal = np.array(q['x']), np.array(q['normal'])
                source = q['source']; c = source[4]; r = q['r']; weight = min(1., .1/max(abs(r), 1e-300))
                gradient = weight*np.r_[normal, np.cross(x, normal)]*r
                contribution = float(-functional@gradient)
                neighbours = [v for v in q['neighbours'] if v[1] <= 1.]
                assert neighbours
                nearest_cluster = q['neighbours'][0][2][4]
                any_target = any(marked(target, v[2][4]) for v in neighbours)
                all_target = all(marked(target, v[2][4]) for v in neighbours)
                roi = -12 <= source[0] <= 15 and -5 <= source[1] <= -2 and -1.6 <= source[2] <= .5
                labels = ['all']
                for label, active in [('source_prediction_prior_local',marked(srcpred,c)),('source_prediction_current_local',marked(srcfuture,c)),('source_final',marked(srcfinal,c)),('target_previous_final_nearest',marked(target,nearest_cluster)),('target_previous_final_any_support',any_target),('target_previous_final_all_support',all_target),('inherited_roi',roi),('inherited_roi_source_prediction',roi and marked(srcpred,c))]:
                    if active: labels.append(label)
                for label in labels:
                    classes[label]['pairs'] += 1; classes[label]['first_forward_percent'] += contribution
                cluster_stats[c]['pairs'] += 1; cluster_stats[c]['first_forward_percent'] += contribution
            for c, d in cluster_stats.items():
                row = dict(frame=i,cluster=c,**d,masks=[bycluster[i,s][c] for s in range(3)])
                cout.write(json.dumps(row,separators=(',',':'),allow_nan=False)+'\n')
            final = np.linalg.inv(np.array(traces[i-1]['pose']).reshape(4,4))@np.array(traces[i]['pose']).reshape(4,4)
            output = np.array(native['output']).reshape(4,4)
            input_pose = np.array(native['input']).reshape(4,4)
            s2s_ratio = float(np.linalg.norm(output[:3,3])/max(length,1e-30))
            final_ratio = float(np.linalg.norm(final[:3,3])/max(length,1e-30))
            coverage = []
            for s in range(3):
                values = masks[i,s]['clusters']
                coverage.append(dict(stage=s,clusters=len(values),dynamic_clusters=sum(c[4] for c in values),
                    all_non_ground_points=sum(c[1] for c in values),covered_points=sum(c[2] for c in values),flagged_points=sum(c[3] for c in values),
                    native_masked_facade_features=sum(c[5] for c in values if c[4]),
                    cannot_reach_object_threshold_clusters=sum(c[2] < .5*c[1] for c in values),
                    cannot_reach_object_threshold_facade_features=sum(c[5] for c in values if c[2] < .5*c[1])))
            moving = length >= .1
            frame_rows.append(dict(frame=i,gt_step_m=length,moving=moving,s2s_bad_length=moving and abs(s2s_ratio-1)>.2,
                final_bad_length=moving and abs(final_ratio-1)>.2,s2s_input_length_ratio=float(np.linalg.norm(input_pose[:3,3])/max(length,1e-30)),
                s2s_output_length_ratio=s2s_ratio,final_length_ratio=final_ratio,
                detector_stages=coverage,accepted_facade_groups=dict(classes)))
    assert seen == list(range(1,len(traces)))
    with (stage/'frames.jsonl').open('x') as out:
        for row in frame_rows:out.write(json.dumps(row,separators=(',',':'),allow_nan=False)+'\n')
    summaries = {}
    for group, selected in [('all_moving',[r for r in frame_rows if r['moving']]),('s2s_bad_length',[r for r in frame_rows if r['s2s_bad_length']]),('final_bad_length',[r for r in frame_rows if r['final_bad_length']]),('other_moving',[r for r in frame_rows if r['moving'] and not r['s2s_bad_length']])]:
        summaries[group] = dict(frames=len(selected),stages=[dict(stage=s,
            source_masked_pairs=sum(r['accepted_facade_groups'].get(['source_prediction_prior_local','source_prediction_current_local','source_final'][s],{}).get('pairs',0) for r in selected),
            accepted_facade_pairs=sum(r['accepted_facade_groups']['all']['pairs'] for r in selected),
            cannot_reach_threshold_facade_features=sum(r['detector_stages'][s]['cannot_reach_object_threshold_facade_features'] for r in selected)) for s in range(3)])
    write(stage/'report.json',dict(exit_code=0,normal_full_frames=len(traces),all_unanchored_reference_frame_fields_exact=True,
        excluded_fields=['scan_path (declared validated conventions)','anchored_pose (probe relative, harness GT display anchor)'],old_scan_root=old_root,
        native_masks_and_dor_removed_exact=True,detailed_range_counts_exact=True,summary=summaries,
        endpoints=[r for r in frame_rows if r['frame'] in p['selected_endpoints']],limits=p['analysis']))


if __name__=='__main__':main()
