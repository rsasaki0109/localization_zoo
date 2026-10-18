from pathlib import Path
import gzip,hashlib,json,shutil,subprocess
r=Path(__file__).resolve().parent;repo=r/'worktree';e=repo/'experiments/results/inten_trace_export/evidence'
assert json.loads((r/'skipped_input_retry_command/command.json').read_text())['exit_code']==0
assert json.loads((r/'skipped_input_retry/summary.json').read_text())['exit_code']==0
sha=lambda data:hashlib.sha256(data).hexdigest()
manifest={}
for src,label in [(r/'skipped_input_run','skipped_input_initial'),(r/'skipped_input_command','skipped_input_initial_command'),(r/'skipped_input_retry','skipped_input'),(r/'skipped_input_retry_command','skipped_input_command')]:
 dest=e/label;assert not dest.exists();dest.mkdir()
 for p in sorted(src.rglob('*')):
  if not p.is_file() or p.relative_to(src).parts[0]=='input':continue
  target=dest/p.relative_to(src);target.parent.mkdir(parents=True,exist_ok=True)
  raw=p.read_bytes();stored=gzip.compress(raw,mtime=0) if p.suffix=='.log' else raw
  if p.suffix=='.log':target=target.with_suffix('.log.gz')
  target.write_bytes(stored);manifest[str(target.relative_to(e))]=dict(original_sha256=sha(raw),stored_sha256=sha(stored),lossless=True)
# Supplement the closed real-fixture input inventory without rerunning its native calls.
p=repo/'evaluation/fixtures/mcd_kth_smoke/frame_timestamps.csv';before=subprocess.check_output(['git','show','91c5913917a3f3540a60783ef9b2207cdb4fc16d:'+str(p.relative_to(repo))],cwd=repo)
assert before==p.read_bytes()
(e/'fixture_timestamp_identity.json').write_text(json.dumps(dict(path=str(p.relative_to(repo)),executed_head='91c5913917a3f3540a60783ef9b2207cdb4fc16d',sha256=sha(before),identical_to_executed_commit=True,scope='Supplemental timestamp inventory for the closed real fixture. No native call rerun; original receipt is retained unchanged.'),indent=2)+'\n')
(e/'skipped_import_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
shutil.copyfile(r/'import_skipped_evidence.py',e/'import_skipped_evidence.py')
for name,entry in manifest.items():
 data=(e/name).read_bytes();assert sha(data)==entry['stored_sha256']
 if name.endswith('.log.gz'):data=gzip.decompress(data)
 assert sha(data)==entry['original_sha256']
subprocess.run(['git','add','-f',str(e)],cwd=repo,check=True)
subprocess.run(['git','diff','--cached','--check'],cwd=repo,check=True)
subprocess.run(['git','commit','-m','Retain skipped-input parity and the initial strict timestamp failure'],cwd=repo,check=True,stdout=subprocess.DEVNULL)
print(subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip())
