"""Portable archive, decision, equation/control, and historical-preservation checks."""
from pathlib import Path
from collections import Counter, defaultdict
import argparse
import gzip
import hashlib
import importlib.util
import json
import subprocess

E = Path(__file__).resolve().parent
W = E.parents[2]
BASE = '761eb92bc172339aa22496275d0d7e9bfc1fec56'


def read(path):
    return json.loads(Path(path).read_text())


def restored(folder, name):
    entry = read(folder/'archive.json')['files'][name]
    if entry['storage'] == 'ordered_gzip_shards':
        blocks = []
        for part in entry['parts']:
            raw = (folder/part['path']).read_bytes()
            assert hashlib.sha256(raw).hexdigest() == part['gzip_sha256']
            raw = gzip.decompress(raw)
            assert len(raw) == part['raw_bytes'] and hashlib.sha256(raw).hexdigest() == part['raw_sha256']
            blocks.append(raw)
        result = b''.join(blocks)
    else:
        result = (folder/entry['path']).read_bytes()
        assert hashlib.sha256(result).hexdigest() == entry['stored_sha256']
        if entry['storage'] == 'gzip': result = gzip.decompress(result)
    assert len(result) == entry['bytes'] and hashlib.sha256(result).hexdigest() == entry['sha256']
    return result


def validate(history=True):
    protocol = read(E/'protocol.json')
    post = read(E/'postprocessing_protocol.json')
    for path, digest in protocol['source_sha256'].items():
        digest = post['verification_source_updates'].get(path, {}).get('sha256', digest)
        if history or path.startswith(str(E.relative_to(W))+'/') or path == 'tests/test_inten_cluster_motion_study.py':
            assert hashlib.sha256((W/path).read_bytes()).hexdigest() == digest, path
    for folder in ('controls', 'analysis', 'collection_execution', 'analysis_execution'):
        manifest = read(E/folder/'archive.json')
        assert manifest['byte_roundtrip_verified'] and manifest['exit_code'] == 0
        for name in manifest['files']: restored(E/folder, name)
    for name in ('compiler', 'compile_native', 'compile_probe', 'linked_libraries', 'mechanism_tests', 'native'):
        assert read(E/'controls'/(name+'_command.json'))['exit_code'] == 0
    for folder, name in (('collection_execution', 'collection'), ('analysis_execution', 'analysis')):
        assert read(E/folder/(name+'_command.json'))['exit_code'] == 0
    original = (E/'reference_native.cpp').read_text()
    assert hashlib.sha256(original.encode()).hexdigest() == protocol['source_sha256']['papers/inten_loam/src/inten_loam_paper.cpp']
    spec = importlib.util.spec_from_file_location('cluster_instrument', E/'instrument.py')
    instrument = importlib.util.module_from_spec(spec); spec.loader.exec_module(instrument)
    assert instrument.transformed(original) == (E/'diagnostic_native.cpp').read_text()
    sources = read(E/'controls/input_sources.json')
    assert sources['no_raw_scans_acquired'] and sources['no_gt_in_native_input']
    for item in sources['files']:
        manifest = W/item['archive_manifest']
        assert hashlib.sha256(manifest.read_bytes()).hexdigest() == item['archive_manifest_sha256']
        assert hashlib.sha256(restored(manifest.parent, item['name'])).hexdigest() == item['sha256']
    native = list(map(json.loads, restored(E/'controls', 'native.stdout.jsonl').splitlines()))
    terminals = {(v['frame'], v['mode']): v for v in native if v['type'] == 'terminal'}
    profiles = defaultdict(list)
    for row in native:
        if row['type'] == 'profile': profiles[row['frame']].append(row)
    assert list(terminals) == [(i, m) for i in protocol['frames'] for m in (0, 1)]
    full = {v['frame']: v for v in map(json.loads, restored(W/'experiments/results/inten_current_correspondences/baseline', 'native.stdout').splitlines())}
    expected_first = {v['frame']: v['first_systems'] for v in map(json.loads, restored(W/'experiments/results/inten_facade_loss/controls', 'native.stdout.log').splitlines()) if v['mode'] == 0}
    report = read(E/'analysis/report.json')
    counts = Counter()
    for case in report['cases']:
        i = case['frame']; baseline, candidate = (terminals[(i, m)] for m in (0, 1))
        assert baseline['output'] == full[i]['s2s']['output']
        assert baseline['first_systems'] == expected_first[i]
        assert {k: v for k, v in baseline['solver'].items() if k != 'returned_pairs'} == {k: v for k, v in full[i]['s2s'].items() if k != 'returned_pairs'}
        for c in (0, 1, 3): assert candidate['first_systems'][c] == baseline['first_systems'][c]
        for t in (baseline, candidate):
            assert t['solver']['returned_pairs'] == sum(v['n'] for v in t['first_systems'])
        mask = []
        for row in profiles[i]:
            costs = row['costs_m2']
            best = min(range(len(costs)), key=lambda k: (costs[k], abs(k-60), k)) if costs else 60
            decision = bool(costs and 0 < best < 120 and abs((best-60)*.05) > 1. and costs[best] < .5*costs[60])
            assert row['best_index'] == best and row['excluded'] == decision
            if decision: mask.append(row['cluster'])
        assert mask == case['excluded_clusters']
        assert case['profiled_clusters'] == len(profiles[i])
        assert all(case[m]['first_pairs'] == [v['n'] for v in terminals[(i, j)]['first_systems']]
                   for j, m in enumerate(('baseline', 'candidate')))
        d = case['candidate']['length_error']-case['baseline']['length_error']
        label = 'equal' if abs(d) <= 1e-12 else 'lower' if d < 0 else 'higher'
        assert label == case['length_error_vs_baseline']; counts[label] += 1
    assert dict(counts) == {k: v for k, v in report['length_error_counts'].items() if v}
    assert report['exit_code'] == 0 and report['frames'] == protocol['frames'] and report['native_calls'] == len(terminals)
    assert report['profiled_clusters'] == sum(map(len, profiles.values()))
    for key in ('baseline_outputs_first_systems_and_full_solver_exact', 'non_facade_first_equations_exact',
                'all_first_equations_independently_reconstructed', 'forced_return_count_independently_checked',
                'all_finite_updates_backward_checked'): assert report[key]
    if history:
        paths = subprocess.check_output(['git', 'diff', '--name-only', BASE, '--'], cwd=W, text=True).splitlines()
        permitted = ('experiments/results/inten_cluster_motion/', 'tests/test_inten_cluster_motion_study.py')
        for path in paths:
            assert path.startswith(permitted) or path in ('papers/inten_loam/README.md', 'docs/roadmap.md'), path
        previous = subprocess.check_output(['git', 'show', BASE+':papers/inten_loam/README.md'], cwd=W)
        assert (W/'papers/inten_loam/README.md').read_bytes().startswith(previous)
    return dict(exit_code=0, base_commit=BASE, native_calls=len(terminals), full_baseline_control_exact=True,
                all_frozen_profile_decisions_exact=True, all_archives_byte_roundtrip_verified=True,
                history_diff_checked=history, live_production_pins_checked=history,
                archived_production_source_matches_execution=True, no_raw_scans_acquired=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.write_text(json.dumps(validate(), indent=2)+'\n')
    print('Cluster-motion evidence and historical preservation passed')
