"""Verify diagnostic archives and preservation of all historical scientific outputs."""
import tempfile
import subprocess
import hashlib
from run import E, W, R, read, write, sha, module

PARENT = '9028e44cfe6241c2e20b22f2039d42c69bd8594b'


def main():
    native_paths = ['common', 'evaluation/src', 'evaluation/CMakeLists.txt',
                    'CMakeLists.txt', ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
                    ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'ros2', 'python']
    assert subprocess.run(['git', 'diff', '--exit-code', PARENT, '--', *native_paths], cwd=W).returncode == 0
    assert subprocess.run(['git', 'diff', '--exit-code', PARENT, '--', 'experiments/results',
                           ':(exclude)experiments/results/inten_current_correspondences', 'docs/benchmarks'], cwd=W).returncode == 0
    canonical = ['README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
                 'evaluation/data/paper_reported_numbers.json', 'experiments/results/index.json']
    pins = {}
    for rel in canonical:
        original = subprocess.check_output(['git', 'show', PARENT+':'+rel], cwd=W)
        assert original == (W/rel).read_bytes(), rel
        pins[rel] = hashlib.sha256(original).hexdigest()
    old = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', PARENT, 'experiments/results'], cwd=W, text=True).splitlines()
    old_aggregates = [s for s in old if s.count('/') == 2 and s.endswith('_matrix.json')]
    assert len(list((W/'experiments/results').glob('*_matrix.json'))) == len(old_aggregates)
    restore = module('verify_restore', W/'experiments/results/inten_natural_traces/diagnose.py').restore
    archives = []
    for name in ['baseline', 'analysis', 'fetch', 'ablations', 'motion_initial_failure', 'motion', 'figures', 'figures_execution']:
        with tempfile.TemporaryDirectory(dir=R) as temp:
            restore(E/name, __import__('pathlib').Path(temp))
        archives.append(name)
    p = read(E/'protocol.json')
    assert sha(W/'papers/inten_loam/src/inten_loam_paper.cpp') == p['native_source_sha256']
    assert sha(E/'diagnostic_native.cpp') == p['diagnostic_source_sha256']
    assert sha(E/'ablation_native.cpp') == read(E/'ablation_protocol.json')['ablation_native_sha256']
    assert read(E/'baseline/report.json')['all_continuous_solver_traces_and_poses_exact']
    assert read(E/'ablations/report.json')['natural_pair_outputs_exact']
    assert read(E/'ablations/report.json')['non_facade_first_equations_exact']
    for path in [E/'baseline/native_command.json', E/'analysis/analysis_command.json',
                 E/'ablations/native_command.json', E/'motion/native_command.json']:
        assert read(path)['exit_code'] == 0
    assert read(E/'motion_initial_failure/native_command.json')['exit_code'] == 1
    write(R/'preservation.json', dict(exit_code=0, parent_commit=PARENT,
        production_source_and_interfaces_unchanged=True, historical_results_and_benchmarks_unchanged=True,
        old_aggregates_unchanged=len(old_aggregates), no_new_accuracy_aggregate=True,
        canonical_sha256=pins, all_archives_byte_roundtrip_verified=archives,
        known_initial_motion_failure_preserved=True))


if __name__ == '__main__':
    main()
