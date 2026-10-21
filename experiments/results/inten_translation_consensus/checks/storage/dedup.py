from pathlib import Path
import hashlib,json,os,subprocess,time
R=Path(__file__).resolve().parent;w=R/'worktree';old=Path('/workspace/localization-env/inten-cluster-motion/worktree')
def git(p,*a):return subprocess.check_output(['git','--no-optional-locks',*a],cwd=p,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert not git(w,'status','--porcelain') and not git(old,'status','--porcelain')
plan=[]
for rel in git(w,'ls-files','experiments/results','docs/benchmarks').splitlines():
 p=w/rel;q=old/rel
 if p.is_symlink() or q.is_symlink() or not p.is_file() or not q.is_file() or p.stat().st_size<100000 or p.stat().st_ino==q.stat().st_ino:continue
 if Path(rel).parent==Path('experiments/results'):continue
 assert p.stat().st_mode==q.stat().st_mode and p.stat().st_size==q.stat().st_size
 h=sha(p);assert h==sha(q)
 plan.append(dict(path=rel,sha256=h,bytes=p.stat().st_size,mode=p.stat().st_mode))
for x in plan:
 p=w/x['path'];q=old/x['path'];tmp=p.with_name(p.name+'.storage-link');assert not tmp.exists();os.link(q,tmp);os.replace(tmp,p)
 assert sha(p)==x['sha256'] and p.stat().st_mode==x['mode']
assert not git(w,'status','--porcelain') and not git(old,'status','--porcelain')
(R/'storage.json').write_text(json.dumps(dict(exit_code=0,canonical=str(old),target=str(w),files=plan,scope='Immutable identical historical evidence; every path/byte/mode preserved; sources/builds/binaries/index/top-level aggregates untouched'),indent=2)+'\n')
print('storage preservation verified',len(plan))
