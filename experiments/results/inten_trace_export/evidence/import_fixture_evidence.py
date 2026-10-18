from pathlib import Path
import gzip,hashlib,json,shutil,subprocess,time
r=Path(__file__).resolve().parent;repo=r/'worktree';e=repo/'experiments/results/inten_trace_export/evidence'
assert json.loads((r/'fixture_command/command.json').read_text())['exit_code']==0
assert json.loads((r/'fixture_run/summary.json').read_text())['exit_code']==0
assert json.loads((r/'focused_build/summary.json').read_text())['exit_code']==0
root=Path('/workspace/localization_zoo');original=Path('/workspace/localization-env/shared-near-range')
seals=json.loads((original/'completion_state.json').read_text())['sealed_file_sha256']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
sealcheck={name:dict(expected=digest,actual=sha(root/'experiments/results/shared_near_range'/name)) for name,digest in seals.items()}
assert all(x['expected']==x['actual'] for x in sealcheck.values())
pins={'baseline_pcd_dogfooding':'ede46d492a31f6dfea95b111bb8941974ad63a903c68e9ee860f7e2f60ca2450','candidate_pcd_dogfooding':'e9ed1f650d42b73d1156e8b3eda3a145ebed4ad382323863f0d89d886d5d166d'}
binaries={k:dict(expected=v,actual=sha(original/k)) for k,v in pins.items()};assert all(x['expected']==x['actual'] for x in binaries.values())
native=['common','evaluation/src','evaluation/CMakeLists.txt',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',':(glob)papers/*/CMakeLists.txt','CMakeLists.txt']
cmd=['git','diff','--exit-code','0b079dbbcb7c6f58c1d8367ade881c15f003e525','--',*native]
result=subprocess.run(cmd,cwd=root,text=True,capture_output=True);assert result.returncode==0,result.stdout
(r/'primary_integrity.json').write_text(json.dumps(dict(exit_code=0,command=cmd,cwd=str(root),source_exit_code=result.returncode,sealed_files=sealcheck,binaries=binaries),indent=2)+'\n')
subprocess.run(['/workspace/localization-env/host-venv/bin/python','experiments/results/inten_trace_export/verify_preservation.py','--output',str(r/'preservation_before_import.json')],cwd=repo,check=True)
manifest={}
for source,label in [(r/'focused_build','focused_build'),(r/'fixture_run','fixture'),(r/'fixture_command','fixture_command')]:
 dest=e/label;assert not dest.exists();dest.mkdir()
 for p in sorted(source.rglob('*')):
  if not p.is_file() or p.suffix=='.o' or p.name=='pcd_dogfooding':continue
  target=dest/p.relative_to(source);target.parent.mkdir(parents=True,exist_ok=True)
  data=p.read_bytes()
  if p.suffix=='.log':
   target=target.with_suffix(target.suffix+'.gz');stored=gzip.compress(data,mtime=0)
  else:stored=data
  target.write_bytes(stored)
  manifest[str(target.relative_to(e))]=dict(original_sha256=hashlib.sha256(data).hexdigest(),stored_sha256=sha(target),lossless=True)
for name in ['build_focused_runner.py','import_fixture_evidence.py','primary_integrity.json','preservation_before_import.json']:
 shutil.copyfile(r/name,e/name)
(e/'import_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
# Newly imported artifacts must match the closed private runs, including saved poses.
for path,entry in manifest.items():
 data=(e/path).read_bytes();assert hashlib.sha256(data).hexdigest()==entry['stored_sha256']
 if path.endswith('.log.gz'):data=gzip.decompress(data)
 assert hashlib.sha256(data).hexdigest()==entry['original_sha256']
subprocess.run(['git','add','-f',str(e)],cwd=repo,check=True)
subprocess.run(['git','diff','--cached','--check'],cwd=repo,check=True)
subprocess.run(['git','commit','-m','Retain closed trace-on/off parity and negative CLI execution evidence'],cwd=repo,check=True)
print('Evidence committed before numeric documentation')
