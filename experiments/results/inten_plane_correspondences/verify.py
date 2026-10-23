"""Verify preservation against the actual parent of the plane study."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import subprocess

E = Path(__file__).resolve().parent
W = E.parents[2]
BASE = 'eee61c7f20dc9223d4a099fe0075c672479fc2ec'


def read(path):
    return json.loads(Path(path).read_text())


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def require(ok, reason):
    if not ok:
        raise ValueError(reason)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    expected = read(W/'experiments/results/shared_near_range/historical_aggregate_sha256.json')
    actual = {p.relative_to(W).as_posix(): sha(p) for p in (W/'experiments/results').glob('*_matrix.json')}
    require(actual == expected, 'historical aggregates changed')

    def before(path):
        return subprocess.check_output(['git', 'show', BASE+':'+path.relative_to(W).as_posix()], cwd=W)

    publication = {}
    for name in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
                 'docs/assets/paper/full_variant_results.csv', 'evaluation/data/paper_reported_numbers.json'):
        p = W/name
        require(before(p) == p.read_bytes(), 'publication changed: '+name)
        publication[name] = sha(p)
    binaries = {}
    for filename, key in [('shared_near_range/baseline.json', 'binary'), ('shared_near_range/candidate.json', 'binary'),
                          ('inten_natural_traces/binary.json', 'binary'), ('inten_seed_intervention/binary.json', 'path'),
                          ('inten_plane_correspondences/binary.json', 'path')]:
        pin = read(W/'experiments/results'/filename)
        require(sha(pin[key]) == pin['sha256'], 'pinned binary changed')
        binaries[pin[key]] = pin['sha256']
    pin = read(E/'binary.json')
    require(sha(pin['library']['path']) == pin['library']['sha256'], 'library changed')
    changed = subprocess.check_output(['git', 'diff', '--name-only', BASE], cwd=W, text=True).splitlines()
    timestamps = {}
    timestamp_docs = ('docs/decisions.md', 'docs/experiments.md', 'docs/interfaces.md', 'docs/paper_roadmap.md',
        'docs/paper_tracks.md', 'docs/paper_assets.md', 'docs/paper_captions.md', 'docs/paper_comparison.md',
        'docs/reproduction_status.md', 'docs/variant_analysis.md')
    for name in changed:
        if name.startswith('experiments/results/inten_plane_correspondences/') or name in ('docs/roadmap.md', 'papers/inten_loam/README.md'):
            continue
        p = W/name; old, new = before(p), p.read_bytes()
        if name == 'experiments/results/index.json':
            a, b = json.loads(old), json.loads(new)
            a.pop('generated_at'); b.pop('generated_at')
            require(a == b, 'non-timestamp index change')
        else:
            require(name in timestamp_docs, 'unexpected existing file change: '+name)
            def strip(raw):
                return b'\n'.join(x for x in raw.splitlines() if not x.startswith((b'_Generated at ', b'> Generated: ')))
            require(strip(old) == strip(new), 'non-timestamp documentation change: '+name)
        timestamps[name] = dict(timestamp_only=True, before_sha256=hashlib.sha256(old).hexdigest(), after_sha256=sha(p))
    spec = importlib.util.spec_from_file_location('plane_invariants', E/'run.py')
    driver = importlib.util.module_from_spec(spec); spec.loader.exec_module(driver); driver.invariant()
    sequences = {}
    for seq in ('01', '02', '10'):
        folder = E/'sequences'/seq
        data = read(folder/'geometry.json')['summary']
        require(data['native_output_and_first_systems_exact'], 'parent control mismatch')
        require(read(folder/'native_command.json')['exit_code'] == 0, 'native execution failed')
        inputs = read(folder/'inputs.json')
        require(inputs['files'] == read(W/'experiments/results/inten_natural_traces/sequences'/seq/'inputs.json')['files'], 'official inputs differ')
        cleanup = read(folder/'cleanup.json')
        require(cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after'] and
                not Path(cleanup['owned_input_dir']).exists(), 'scans remain')
        require(read(folder/'archive.json')['byte_roundtrip_verified'], 'archive incomplete')
        sequences[seq] = dict(native_exit_code=0, parent_output_and_first_systems_exact=True,
            pair_count=data['case_count'], official_input_hashes_match=True, owned_scans_absent=True)
    report = read(E/'report/summary.json')
    require(report['exit_code'] == 0 and report['geometry_reproduced_from_archives'], 'offline geometry not reproduced')
    require(read(E/'report_execution/command.json')['exit_code'] == 0, 'report failed')
    require(read(E/'collection_execution/command.json')['exit_code'] == 0, 'collector failed')
    require(read(E/'scope_execution/command.json')['exit_code'] == 0 and read(E/'scope/summary.json')['exit_code'] == 0, 'row scope execution failed')
    for seq in ('01', '02', '10'):
        require(read(E/'scope'/(seq+'_scope.json'))['native_rejection_universe_matches'], 'row scope universe mismatch')
    result = dict(exit_code=0, base_commit=BASE, historical_aggregate_count=len(expected), historical_aggregate_hashes_match=True,
        publication_sha256=publication, binary_sha256=binaries, sequences=sequences, timestamp_only_changes=timestamps,
        scope='All pre-existing native code, manifests, tools and evidence preserved against parent153. Only new study, method README, roadmap and verified generator timestamps differ.')
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    print('preservation passed', flush=True)


if __name__ == '__main__':
    main()
