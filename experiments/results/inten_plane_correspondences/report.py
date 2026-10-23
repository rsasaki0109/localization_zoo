"""Offline reproduction and complete report of the committed plane study."""
from pathlib import Path
import argparse
import csv
import hashlib
import importlib.util
import json
import subprocess
import tempfile

E = Path(__file__).resolve().parent
W = E.parents[2]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def read(path):
    return json.loads(path.read_text())


def write(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def query_support(path, geometry):
    import numpy as np
    from collections import Counter
    result = {}
    totals = {name: Counter() for name in ('ground', 'facade')}
    with path.open() as stream:
        for line in stream:
            record = json.loads(line); classes = {}
            for plane, name in zip(record['planes'], ('ground', 'facade')):
                target = np.asarray(plane['target']).reshape(-1, 4)
                counts = Counter()
                for query in plane['queries']:
                    accepted, ids, distances, normal, residual = query
                    if len(ids) < 3:
                        reason = 'fewer_than_three_targets'
                    elif distances[0] > 1.:
                        reason = 'nearest_outside_native_gate'
                    else:
                        qj = target[ids[0]]
                        neighbours = [i for i, d in zip(ids, distances) if d <= 1.]
                        same = next((i for i in neighbours[1:] if target[i, 3] == qj[3]), None)
                        adj = next((i for i in neighbours[1:] if 1 <= abs(target[i, 3]-qj[3]) <= 2), None)
                        if same is None and adj is None:
                            reason = 'missing_same_and_adjacent_row'
                        elif same is None:
                            reason = 'missing_same_row'
                        elif adj is None:
                            reason = 'missing_adjacent_row'
                        elif np.linalg.norm(np.cross(qj[:3]-target[adj, :3], qj[:3]-target[same, :3])) < 1e-6:
                            reason = 'degenerate_triangle'
                        else:
                            reason = 'accepted'
                    geometry.require((reason == 'accepted') == accepted, 'query rejection reconstruction mismatch')
                    counts[reason] += 1
                classes[name] = dict(counts)
                totals[name].update(counts)
            result[record['id']] = classes
    return dict(cases=result, totals={name: dict(c) for name, c in totals.items()},
                all_rejection_decisions_reconstructed=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=False)
    protocol = read(E/'protocol.json')
    geometry = module('plane_geometry', E/'geometry.py')
    archival = module('plane_archival', W/'experiments/results/inten_natural_traces/diagnose.py')
    study = dict(exit_code=0, source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=W, text=True).strip(),
                 protocol_sha256=hashlib.sha256((E/'protocol.json').read_bytes()).hexdigest(),
                 geometry_reproduced_from_archives=True, sequences={}, limits=protocol['limits'])
    plot_rows = []
    for seq in protocol['sequences']:
        folder = E/'sequences'/seq
        cases = read(folder/'cases.json')['cases']
        stored = read(folder/'geometry.json')
        with tempfile.TemporaryDirectory() as temporary:
            temporary = Path(temporary)
            native_dir, parent_dir = temporary/'new', temporary/'parent'
            native_dir.mkdir(); parent_dir.mkdir()
            archival.restore(folder, native_dir)
            archival.restore(W/'experiments/results/inten_seed_intervention/sequences'/seq, parent_dir)
            expected = {}
            with (parent_dir/'native.stdout').open() as stream:
                for line in stream:
                    row = json.loads(line)
                    if row['mode'] == 'oracle_current_gt':
                        expected[row['id']] = row
            reproduced = geometry.analyze_sequence(cases, native_dir/'native.jsonl', expected, protocol['control_tolerances'])
            geometry.require(reproduced == stored, 'offline geometry differs: ' + seq)
            support = query_support(native_dir/'native.jsonl', geometry)
            write(args.output_dir/(seq+'_query_support.json'), support)
        with (W/'experiments/results/inten_seed_intervention/report'/(seq+'_cases.csv')).open() as stream:
            parent_labels = {r['id']: r for r in csv.DictReader(stream) if r['mode'] == 'oracle_current_gt'}
        rows = []
        by_id = {}
        for case in stored['cases']:
            label = parent_labels[case['id']]
            item = dict(id=case['id'], frame=case['frame'], moving=case['moving'],
                natural_bad_length=case['natural_bad_length'], oracle_bad_length=case['oracle_bad_length'],
                oracle_bad_vector=label['output_bad_vector'] == 'True', s2s_bad_entry=label['s2s_bad_entry'] == 'True',
                edge_pairs=expected[case['id']]['first_systems'][0]['n'],
                oracle_output_length_ratio=case['oracle_output_length_ratio'],
                native_first_forward_fraction=case['native_first']['first_forward_fraction'],
                native_first_finite_vector_error=case['native_first']['first_finite_vector_error'])
            for name in ('ground', 'facade'):
                plane = case['planes'][name]
                item[name+'_pairs'] = plane['accepted']
                item[name+'_fixed_total_forward_fraction'] = plane['fixed_total_forward_fraction']
                for key in ('triangle_sine', 'pca_normal_angle_deg', 'up_normal_radial', 'up_normal_z', 'pca_smallest_over_trace'):
                    item[name+'_'+key+'_median'] = plane['distributions'].get(key, {}).get('median')
            for name, control in case['planes']['ground']['controls'].items():
                for key, value in control.items():
                    item[name+'_'+key] = value
            rows.append(item); by_id[item['id']] = case
        with (args.output_dir/(seq+'_cases.csv')).open('w') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator='\n')
            writer.writeheader(); writer.writerows(rows)
        plot_rows.extend((seq, row) for row in rows if row['moving'])
        groups = {}
        for name in ('all_selected_moving', 'natural_bad_length', 'other_selected_moving', 'oracle_bad_length', 'oracle_other_moving'):
            part = [r for r in rows if r['moving'] and
                (name == 'all_selected_moving' or
                 name == 'natural_bad_length' and r['natural_bad_length'] or
                 name == 'other_selected_moving' and not r['natural_bad_length'] or
                 name == 'oracle_bad_length' and r['oracle_bad_length'] or
                 name == 'oracle_other_moving' and not r['oracle_bad_length'])]
            group = dict(case_count=len(part), ids=[r['id'] for r in part],
                native_first_forward_fraction=geometry.distribution(r['native_first_forward_fraction'] for r in part),
                ground_fixed_total_forward_fraction=geometry.distribution(r['ground_fixed_total_forward_fraction'] for r in part),
                facade_fixed_total_forward_fraction=geometry.distribution(r['facade_fixed_total_forward_fraction'] for r in part), controls={})
            for control in ('pca_through_nearest', 'pca_through_centroid'):
                prefix = control+'_'
                group['controls'][control] = dict(
                    first_forward_fraction=geometry.distribution(r[prefix+'first_forward_fraction'] for r in part),
                    first_finite_vector_error=geometry.distribution(r[prefix+'first_finite_vector_error'] for r in part),
                    smaller_absolute_first_forward_count=sum(abs(r[prefix+'first_forward_fraction']) < abs(r['native_first_forward_fraction']) for r in part),
                    larger_absolute_first_forward_count=sum(abs(r[prefix+'first_forward_fraction']) > abs(r['native_first_forward_fraction']) for r in part),
                    smaller_first_vector_error_count=sum(r[prefix+'first_finite_vector_error'] < r['native_first_finite_vector_error'] for r in part),
                    larger_first_vector_error_count=sum(r[prefix+'first_finite_vector_error'] > r['native_first_finite_vector_error'] for r in part))
            groups[name] = group
        retained_ids = [r['id'] for r in rows if r['oracle_bad_length'] or r['oracle_bad_vector'] or r['s2s_bad_entry'] or r['id'] == '01_420']
        write(args.output_dir/(seq+'_failures_and_entries.json'), dict(ids=retained_ids, cases={i: by_id[i] for i in retained_ids}))
        study['sequences'][seq] = dict(**stored['summary'], moving_pairs=sum(r['moving'] for r in rows),
            oracle_bad_length_count=sum(r['oracle_bad_length'] for r in rows), oracle_bad_vector_count=sum(r['oracle_bad_vector'] for r in rows), groups=groups,
            query_support_totals=support['totals'], archive_sha256=hashlib.sha256((folder/'archive.json').read_bytes()).hexdigest())
        print('reproduced_geometry', seq, len(rows), flush=True)
    write(args.output_dir/'summary.json', study)
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    figure, axes = plt.subplots(1, 2, figsize=(11, 5), constrained_layout=True)
    names = ('pca_through_nearest', 'pca_through_centroid')
    for axis, name in zip(axes, names):
        for seq, color in zip(protocol['sequences'], ('#1864ab', '#c92a2a', '#2b8a3e')):
            part = [r for s, r in plot_rows if s == seq]
            x = [100*r['native_first_forward_fraction'] for r in part]
            y = [100*r[name+'_first_forward_fraction'] for r in part]
            axis.scatter(x, y, s=14, alpha=.6, color=color, label='KITTI '+seq)
        lo = min(axis.get_xlim()[0], axis.get_ylim()[0]); hi = max(axis.get_xlim()[1], axis.get_ylim()[1])
        axis.plot([lo, hi], [lo, hi], color='0.5', linewidth=.8)
        axis.axhline(0, color='0.7', linewidth=.6); axis.axvline(0, color='0.7', linewidth=.6)
        axis.set(xlim=(lo, hi), ylim=(lo, hi), xlabel='Native equations: forward correction [% of GT step]',
                 ylabel='Ground PCA control: forward correction [% of GT step]',
                 title='PCA plane through '+('nearest point' if name.endswith('nearest') else 'centroid'))
        axis.legend(fontsize=8)
    figure.suptitle('First update from GT, fixed associations: diagnostic only')
    figure.savefig(args.output_dir/'first_update_controls.png', dpi=160)
    plt.close(figure)


if __name__ == '__main__':
    main()
