from pathlib import Path
import json,os,hashlib,subprocess,time
R=Path(__file__).resolve().parent;canonical=Path('/workspace/localization-env/inten-current-correspondences/worktree')
targets={
'inten-mapped-prior-rigid':'e057e74619e870341a9b57ec0b13b0bd0ecd7f29',
'inten-row-support':'dec360f5193a060ae1a6a9c5d72232f9485f83d5',
'inten-plane-correspondences':'5e1b93ce62f21109c8061eca300d5f13f3230297',
'inten-collapse-boundaries':'eee61c7f20dc9223d4a099fe0075c672479fc2ec'}
def git(w,*a):return subprocess.check_output(['git','--no-optional-locks',*a],cwd=w,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert not git(canonical,'status','--porcelain')
plans=[]
for name,head in targets.items():
 w=R.parent/name/'worktree';assert git(w,'rev-parse','HEAD')==head and not git(w,'status','--porcelain')
 for rel in git(w,'ls-files','experiments/results','docs/benchmarks').splitlines():
  p=w/rel;q=canonical/rel
  if p.is_symlink() or q.is_symlink() or not p.is_file() or not q.is_file() or p.stat().st_size<100000 or p.stat().st_ino==q.stat().st_ino:continue
  if Path(rel).parent==Path('experiments/results'):continue
  assert p.stat().st_mode==q.stat().st_mode and p.stat().st_size==q.stat().st_size
  h=sha(p);assert h==sha(q),str(p)
  plans.append(dict(target=str(w),path=rel,sha256=h,bytes=p.stat().st_size,mode=p.stat().st_mode))
proof=dict(started_unix=time.time(),canonical=str(canonical),targets=targets,files=plans,scope='Byte/hash/mode-identical immutable historical evidence only. Every logical path/byte/mode preserved. No native source, build, binary, index, top-level aggregate or active study touched.',complete=False)
(R/'historical_storage.json').write_text(json.dumps(proof,indent=2)+'\n')
for v in plans:
 p=Path(v['target'])/v['path'];q=canonical/v['path'];tmp=p.with_name(p.name+'.storage-link');assert not tmp.exists();os.link(q,tmp);os.replace(tmp,p)
 assert sha(p)==v['sha256'] and p.stat().st_mode==v['mode']
for name in targets:assert not git(R.parent/name/'worktree','status','--porcelain')
assert not git(canonical,'status','--porcelain')
proof.update(complete=True,exit_code=0,finished_unix=time.time(),all_worktrees_clean=True,logical_bytes_preserved=sum(v['bytes'] for v in plans))
(R/'historical_storage.json').write_text(json.dumps(proof,indent=2)+'\n')
print('Immutable logical evidence preserved',len(plans),proof['logical_bytes_preserved'])
