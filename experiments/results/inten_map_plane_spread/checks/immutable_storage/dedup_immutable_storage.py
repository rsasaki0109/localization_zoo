"""Coalesce identical immutable historical evidence; preserve every logical file."""
from pathlib import Path
import hashlib,json,os,subprocess,time
R=Path(__file__).resolve().parent;canonical=Path('/workspace/localization-env/inten-facade-pca/worktree');target=Path('/workspace/localization-env/inten-ground-pca/worktree')
def git(w,*a):return subprocess.check_output(['git','--no-optional-locks',*a],cwd=w,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert git(target,'rev-parse','HEAD')=='327c5b0a3ce38dcefc7e554d6ccaecadb3aea20c'
assert not git(target,'status','--porcelain') and not git(canonical,'status','--porcelain')
plan=[]
# Validate the entire batch before the first replacement. No native code,
# builds, binaries, shared index, top-level aggregates or live study files.
for rel in git(target,'ls-files','experiments/results','docs/benchmarks').splitlines():
 p=target/rel;q=canonical/rel
 if p.is_symlink() or q.is_symlink() or not p.is_file() or not q.is_file() or p.stat().st_size<100000 or p.stat().st_ino==q.stat().st_ino:continue
 if rel=='experiments/results/index.json' or Path(rel).parent==Path('experiments/results'):continue
 assert p.stat().st_mode==q.stat().st_mode and p.stat().st_size==q.stat().st_size
 h=sha(p);assert h==sha(q),rel
 plan.append(dict(path=rel,sha256=h,bytes=p.stat().st_size,mode=p.stat().st_mode,original_inode=p.stat().st_ino,canonical_inode=q.stat().st_ino))
receipt=dict(started_unix=time.time(),target=str(target),canonical=str(canonical),target_commit=git(target,'rev-parse','HEAD'),canonical_commit=git(canonical,'rev-parse','HEAD'),scope='Only byte/hash/mode-identical immutable historical scientific evidence. Every logical file/path/content/mode preserved; no native source, build, binary, index, top-level aggregate or live study touched.',files=plan,completed=False)
(R/'historical_storage_dedup.json').write_text(json.dumps(receipt,indent=2)+'\n')
for item in plan:
 p=target/item['path'];q=canonical/item['path'];assert sha(p)==sha(q)==item['sha256']
 tmp=p.with_name(p.name+'.storage-link');assert not tmp.exists();os.link(q,tmp);os.replace(tmp,p)
 assert p.stat().st_mode==item['mode'] and sha(p)==item['sha256']
assert not git(target,'status','--porcelain') and not git(canonical,'status','--porcelain')
receipt.update(completed=True,exit_code=0,finished_unix=time.time(),logical_bytes_preserved=sum(x['bytes'] for x in plan),both_original_worktrees_clean=True)
(R/'historical_storage_dedup.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('All logical files/bytes/modes preserved; original worktrees clean; coalesced bytes',receipt['logical_bytes_preserved'])
