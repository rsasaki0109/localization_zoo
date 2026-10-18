"""Revalidate bindings after integrating shared components; keep actual exits."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

REPO = Path(__file__).resolve().parents[3]
BASE = '7505a2914328d53ac9b319d5f8042189b378d457'
PY = '/opt/localization-venv/bin/python'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--phase', choices=('native', 'docs'), required=True)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
out = args.output_dir.resolve()
out.mkdir(parents=True, exist_ok=False)
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', 'MPLCONFIGDIR=/tmp/bindings-refresh',
          '-w', str(REPO), 'localization-zoo-dev']
commands = [('native_core_identity', ['git', 'diff', '--exit-code', BASE, '--',
            'common', 'papers/*/src', 'papers/*/include', 'papers/*/test',
            'papers/*/CMakeLists.txt', 'evaluation/src', 'evaluation/scripts'])]
if args.phase == 'native':
    targets = ['localization_zoo_py', 'paper_odometry_reference', 'test_suma_paper',
               'test_imls_paper_odometry', 'test_dilo_paper', 'test_litamin2_odometry',
               'test_common_degeneracy', 'test_initial_registration', 'test_range_filter']
    commands += [
        ('configure', docker + ['cmake', '-S', '.', '-B', 'build',
         '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_PYTHON_BINDINGS=ON',
         '-Dpybind11_DIR=/opt/localization-venv/lib/python3.10/site-packages/pybind11/share/cmake/pybind11',
         '-DPYTHON_EXECUTABLE=' + PY, '-DPython3_EXECUTABLE=' + PY]),
        ('build_affected_targets', docker + ['cmake', '--build', 'build', '-j2', '--target', *targets]),
        ('ctest_affected', docker + ['ctest', '--test-dir', 'build', '--output-on-failure',
         '-R', '^(test_paper_python_bindings|test_suma_paper|test_imls_paper_odometry|test_dilo_paper|test_litamin2_odometry|common_degeneracy|initial_registration|range_filter)$']),
        ('existing_python_example', docker + [PY, 'python/example_synthetic.py']),
    ]
    for method in ('suma_paper', 'imls_slam_paper', 'dilo_paper', 'litamin2_paper'):
        commands.append(('example_' + method, docker + [PY, 'python/example_paper_odometry.py',
                        method, '--output', str(out / (method + '_poses.csv'))]))
else:
    for name in ('refresh_study_docs', 'generate_full_variant_table', 'generate_paper_ratio_table'):
        commands.append((name, docker + [PY, 'evaluation/scripts/' + name + '.py']))
    commands.append(('python_unittest', docker + [PY, '-m', 'unittest', 'discover', '-s', 'tests']))
    for name, flags in [('validate_showcase', []), ('generate_leaderboard', ['--check']),
                        ('generate_paper_ready_evidence', ['--check'])]:
        commands.append((name, docker + [PY, 'evaluation/scripts/' + name + '.py', *flags]))
    commands.append(('whitespace', ['git', 'diff', '--check', BASE]))

records = []
try:
    for label, command in commands:
        started = time.time()
        with (out / (label + '.log')).open('w') as log:
            result = subprocess.run(command, cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
        records.append(dict(label=label, command=command, cwd=str(REPO),
                            checked_checkout_commit=head, started_unix=started,
                            finished_unix=time.time(), exit_code=result.returncode))
        (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
        print(label, result.returncode, flush=True)
        if result.returncode:
            raise SystemExit(result.returncode)
    if args.phase == 'native':
        report = REPO / 'build/python/paper_python_fixture.json'
        (out / report.name).write_bytes(report.read_bytes())
        paths = [*REPO.glob('build/python/localization_zoo*.so'),
                 REPO / 'build/python/paper_odometry_reference',
                 *REPO.glob('evaluation/fixtures/mcd_kth_smoke/*/cloud.pcd')]
        (out / 'hashes.json').write_text(json.dumps({str(p.relative_to(REPO)):
            hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}, indent=2) + '\n')
    else:
        historical = json.loads((Path(__file__).parent / 'historical_aggregate_sha256.json').read_text())
        changed = [name for name, digest in historical.items()
                   if hashlib.sha256((REPO / name).read_bytes()).hexdigest() != digest]
        files = ['evaluation/data/paper_reported_numbers.json', 'docs/assets/paper/paper_ratio_table.csv',
                 'docs/index.html']
        preserved = {name: subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=REPO)
                     == (REPO / name).read_bytes() for name in files}
        old = subprocess.check_output(['git', 'show', BASE + ':README.md'], cwd=REPO, text=True)
        now = (REPO / 'README.md').read_text()
        block = r'<!-- LEADERBOARD:START -->(.*?)<!-- LEADERBOARD:END -->'
        preserved['README leaderboard'] = re.search(block, old, re.S)[1] == re.search(block, now, re.S)[1]
        expected = old.replace('pybind11 bindings for KISS-ICP, NDT, GICP and LiTAMIN2\n',
            'pybind11 bindings for KISS-ICP, NDT, GICP, LiTAMIN2, and the stateful\n'
            'SuMa, IMLS-SLAM, DiLO and LiTAMIN2 paper paths\n')
        preserved['README except binding description'] = expected == now
        report = dict(base=BASE, historical_aggregate_count=len(historical),
                      changed_aggregates=changed, unchanged_published_evidence=preserved)
        (out / 'preservation.json').write_text(json.dumps(report, indent=2) + '\n')
        assert not changed and all(preserved.values()), report
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=0, checked_checkout_commit=head)) + '\n')
except BaseException as error:
    code = error.code if isinstance(error, SystemExit) and isinstance(error.code, int) else 1
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=code, error=str(error))) + '\n')
    raise
