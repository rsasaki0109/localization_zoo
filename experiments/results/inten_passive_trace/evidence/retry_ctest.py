from pathlib import Path
import json
import subprocess
import time

r = Path(__file__).resolve().parent
repo = r / 'worktree'
out = r / 'native_checks_retry'
out.mkdir()
tmp = out / 'tmp'
tmp.mkdir()
source = '67cb05b24a7566c6cf00415784f36cb03593ea71'
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-e', 'PYTHONDONTWRITEBYTECODE=1', '-e', 'TEST_TMPDIR=' + str(tmp),
          '-w', str(repo), 'localization-zoo-dev']
probe = '''from pathlib import Path
import json, os
p = Path('/tmp/posture_profile.yaml')
tmp = Path(os.environ['TEST_TMPDIR'])
info = dict(uid=os.getuid(), original_file=str(p), original_file_uid=p.stat().st_uid,
            original_file_mode=oct(p.stat().st_mode & 0o777),
            original_file_writable=os.access(p,os.W_OK),
            selected_tmpdir=str(tmp), selected_tmpdir_writable=os.access(tmp,os.W_OK))
print(json.dumps(info))
assert not info['original_file_writable'] and info['selected_tmpdir_writable']
s = Path('/usr/src/googletest/googletest/src/gtest.cc').read_text()
k = s.index('std::string TempDir()')
chunk = s[k:s.index('// Class ScopedTrace',k)]
assert 'GTEST_OS_LINUX' in chunk and 'GetEnv("TEST_TMPDIR")' in chunk
print(chunk)
'''
commands = [
    ('native_source_guard', ['git','diff','--exit-code',source,'--','common','evaluation/src',
     'evaluation/CMakeLists.txt','papers/*/src','papers/*/include','papers/*/test',
     'papers/*/CMakeLists.txt','CMakeLists.txt']),
    ('environment_probe', docker + ['/opt/localization-venv/bin/python','-c',probe]),
    ('imu_source_identity', ['git','diff','--exit-code','7505a2914328d53ac9b319d5f8042189b378d457',
                             '--','papers/imu_motion_health','papers/attitude_estimation']),
    ('posture_confirmation', docker + ['ctest','--test-dir','build','--output-on-failure','-R','^test_posture_confirmation$']),
    ('ctest', docker + ['ctest','--test-dir','build','--output-on-failure']),
]
receipts=[]
try:
    for name, command in commands:
        start=time.time()
        with (out/(name+'.log')).open('w') as log:
            result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
        receipts.append(dict(name=name,command=command,cwd=str(repo),source_commit=source,
            started_unix=start,finished_unix=time.time(),exit_code=result.returncode))
        (out/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
        print(name,result.returncode,flush=True)
        assert result.returncode==0,(name,result.returncode)
    (out/'LastTest.log').write_bytes((repo/'build/Testing/Temporary/LastTest.log').read_bytes())
    (out/'exit.json').write_text(json.dumps(dict(exit_code=0,source_commit=source,
        initial_ctest_exit_code=8,initial_attempt=str(r/'native_checks'),
        adjustment='Only TEST_TMPDIR points to a fresh user-owned directory. No code, test, threshold or binary changed.'))+'\n')
except BaseException as error:
    (out/'exit.json').write_text(json.dumps(dict(exit_code=1,error=str(error),source_commit=source))+'\n')
    raise
