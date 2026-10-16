from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import sys
import time
import traceback

repo = Path('/workspace/localization_zoo')
r = Path('/workspace/localization-env/paper-bindings')
out = r / 'final_checks'
out.mkdir()
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1',
          '-e', 'MPLCONFIGDIR=/tmp/localization-zoo-mpl-bindings',
          '-w', str(repo), 'localization-zoo-dev', '/opt/localization-venv/bin/python']
commands = [
    ('unchanged_algorithms', ['git', 'diff', '--exit-code', '134ead2f', '--', 'common',
                             'papers/*/src', 'papers/*/include', 'papers/*/test',
                             'papers/*/CMakeLists.txt', 'evaluation/src', 'evaluation/scripts']),
    ('python_native_source_identity', ['git', 'diff', '--exit-code', '98c2f56f', '--',
                                       'python/src', 'python/CMakeLists.txt']),
    ('ros_native_source_identity', ['git', 'diff', '--exit-code', '13984552', '--',
                                    'ros2/localization_zoo_ros/src', 'ros2/localization_zoo_ros/CMakeLists.txt']),
    ('refresh_study_docs', docker + ['evaluation/scripts/refresh_study_docs.py']),
    ('generate_full_variant_table', docker + ['evaluation/scripts/generate_full_variant_table.py']),
    ('generate_paper_ratio_table', docker + ['evaluation/scripts/generate_paper_ratio_table.py']),
    ('python_unittest', docker + ['-m', 'unittest', 'discover', '-s', 'tests']),
    ('validate_showcase', docker + ['evaluation/scripts/validate_showcase.py']),
    ('generate_leaderboard_check', docker + ['evaluation/scripts/generate_leaderboard.py', '--check']),
    ('generate_paper_ready_evidence_check', docker + ['evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
    ('python_syntax', [sys.executable, '-m', 'py_compile', 'python/example_paper_odometry.py',
                       'python/test/test_paper_odometry.py', 'python/test/synthetic_paper_scan.py',
                       'ros2/localization_zoo_ros/test/test_paper_nodes.py',
                       'ros2/localization_zoo_ros/launch/paper_odometry.launch.py']),
    ('whitespace', ['git', 'diff', '--check']),
]
records = []
try:
    for label, command in commands:
        started = time.time()
        with (out / (label + '.log')).open('w') as log:
            env = dict(os.environ)
            env['PYTHONPYCACHEPREFIX'] = '/tmp/localization-zoo-bindings-syntax-cache'
            result = subprocess.run(command, cwd=repo, stdout=log, stderr=subprocess.STDOUT, env=env)
        records.append({'label': label, 'command': command, 'cwd': str(repo),
                        'checked_checkout_commit': head, 'started_unix': started,
                        'finished_unix': time.time(), 'exit_code': result.returncode})
        (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
        if result.returncode:
            raise SystemExit(result.returncode)
    e = repo / 'experiments/results/paper_pipeline_bindings'
    historical = json.loads((e / 'historical_aggregate_sha256.json').read_text())
    changed = [name for name, digest in historical.items()
               if hashlib.sha256((repo / name).read_bytes()).hexdigest() != digest]
    preserved = {}
    for name in ['evaluation/data/paper_reported_numbers.json', 'docs/assets/paper/paper_ratio_table.csv',
                 'docs/index.html']:
        preserved[name] = subprocess.check_output(['git', 'show', '134ead2f:' + name], cwd=repo) == (repo / name).read_bytes()
    old = subprocess.check_output(['git', 'show', '134ead2f:README.md'], cwd=repo, text=True)
    now = (repo / 'README.md').read_text()
    pattern = r'<!-- LEADERBOARD:START -->(.*?)<!-- LEADERBOARD:END -->'
    preserved['README leaderboard block'] = re.search(pattern, old, re.S).group(1) == re.search(pattern, now, re.S).group(1)
    expected = old.replace('pybind11 bindings for KISS-ICP, NDT, GICP and LiTAMIN2\n',
                           'pybind11 bindings for KISS-ICP, NDT, GICP, LiTAMIN2, and the stateful\n'
                           'SuMa, IMLS-SLAM, DiLO and LiTAMIN2 paper paths\n')
    preserved['README except authorized binding description'] = now == expected
    report = {'historical_aggregate_count': len(historical), 'changed_aggregates': changed,
              'unchanged_published_evidence': preserved}
    (out / 'preservation.json').write_text(json.dumps(report, indent=2) + '\n')
    assert not changed and all(preserved.values()), report
    (out / 'exit.json').write_text(json.dumps({'exit_code': 0, 'checked_checkout_commit': head}) + '\n')
except BaseException as error:
    code = error.code if isinstance(error, SystemExit) and isinstance(error.code, int) else 1
    (out / 'exit.json').write_text(json.dumps({'exit_code': code, 'error': str(error)}) + '\n')
    traceback.print_exc()
    raise
