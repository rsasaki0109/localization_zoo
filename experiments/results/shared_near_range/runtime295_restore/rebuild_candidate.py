from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import time

repo = Path('/workspace/localization_zoo')
root = Path('/workspace/localization-env/shared-near-range')
out = root / 'restore295'
source = '0b079dbbcb7c6f58c1d8367ade881c15f003e525'
expected = 'e9ed1f650d42b73d1156e8b3eda3a145ebed4ad382323863f0d89d886d5d166d'
assert subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip() == source
assert subprocess.check_output(['git', 'status', '--porcelain'], cwd=repo) == b''
prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '0:0',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-w', str(repo), 'localization-zoo-dev']
receipts = []
for label, args in [('configure', ['cmake', '-S', '.', '-B', 'build', '-DCMAKE_BUILD_TYPE=Release']),
                    ('build', ['cmake', '--build', 'build', '-j1', '--target', 'pcd_dogfooding'])]:
    command = prefix + args
    started = time.time()
    log = out / ('candidate_' + label + '.log')
    with log.open('x') as stream:
        result = subprocess.run(command, cwd=repo, stdout=stream, stderr=subprocess.STDOUT)
    receipts.append(dict(label=label, command=command, cwd=str(repo), source_commit=source,
                         started_unix=started, finished_unix=time.time(), exit_code=result.returncode))
    (out / 'candidate_commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)
binary = repo / 'build/evaluation/pcd_dogfooding'
rebuilt = out / 'rebuilt_candidate_pcd_dogfooding'
assert not rebuilt.exists()
shutil.copy2(binary, rebuilt)
actual = hashlib.sha256(rebuilt.read_bytes()).hexdigest()
check = dict(source_commit=source, expected_sha256=expected, actual_sha256=actual,
             exact_match=actual == expected, rebuilt_binary=str(rebuilt), exit_code=0 if actual == expected else 3)
(out / 'candidate_hash.json').write_text(json.dumps(check, indent=2) + '\n')
print(json.dumps(check), flush=True)
if actual != expected:
    raise SystemExit(3)
assert not (root / 'candidate_pcd_dogfooding').exists()
shutil.copy2(rebuilt, root / 'candidate_pcd_dogfooding')
