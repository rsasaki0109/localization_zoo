from pathlib import Path
import gzip,hashlib,json,subprocess,sys
E=Path(__file__).resolve().parents[2];repo=E.parents[2]
base=json.loads((E/'protocol.json').read_text())['base_commit']
command=['git','diff','--check',base]
archive=Path(__file__).with_name('native_log_whitespace_initial.log.gz')
if '--from-archive' in sys.argv:
 raw=gzip.decompress(archive.read_bytes());exit_code=2
else:
 result=subprocess.run(command,cwd=repo,capture_output=True)
 assert result.returncode==2
 raw=result.stdout+result.stderr;exit_code=result.returncode
 archive.write_bytes(gzip.compress(raw,mtime=0));assert gzip.decompress(archive.read_bytes())==raw
paths=[]
for line in raw.decode().splitlines():
 if line.endswith('trailing whitespace.'):
  path=line.split(':',1)[0]
  assert path.endswith('.log') and ((path.startswith('experiments/results/shared_near_range/sequences/') and '/replays/' in path) or path.startswith('experiments/results/shared_near_range/checks/fixture_parity/'))
  paths.append(path)
record=dict(command=command,cwd=str(repo),exit_code=exit_code,raw_stdout_stderr_sha256=hashlib.sha256(raw).hexdigest(),archive=archive.name,affected_native_logs=sorted(set(paths)),metadata_recovery_from_archived_output='--from-archive' in sys.argv,reason='Raw native stdout tables include presentation padding; preserve their exact bytes and scope Git whitespace attributes only to native replay and fixture logs.')
Path(__file__).with_name('native_log_whitespace_initial_command.json').write_text(json.dumps(record,indent=2)+'\n')
print('Expected raw native-log whitespace errors preserved; all reported paths are replay logs.')
