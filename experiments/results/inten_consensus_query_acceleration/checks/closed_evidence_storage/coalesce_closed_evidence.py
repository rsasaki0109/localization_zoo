"""Coalesce only committed immutable evidence in closed historical worktrees."""
from pathlib import Path
from collections import defaultdict
import hashlib,json,os,stat,subprocess,sys,time
R=Path(__file__).resolve().parent;BASE=R.parent
SKIP={'inten-recursive-consensus','inten-recursive-consensus-accelerated'}
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for chunk in iter(lambda:f.read(4*1024*1024),b''):h.update(chunk)
 return h.hexdigest()
def same(p,q):
 with p.open('rb') as a,q.open('rb') as b:
  while True:
   x,y=a.read(4*1024*1024),b.read(4*1024*1024)
   if x!=y:return False
   if not x:return True

def main():
 started=time.time();groups=defaultdict(list);worktrees=[]
 for root in sorted(BASE.iterdir()):
  if root.name in SKIP or not root.is_dir():continue
  w=root/'worktree'
  if not (w/'.git').is_file() or any(root.glob('*collector*.lock')):continue
  worktrees.append(w)
  for rel in ('experiments/results','docs/benchmarks'):
   p=w/rel
   if not p.is_dir():continue
   for d,ds,fs in os.walk(p,followlinks=False):
    ds[:]=[n for n in ds if n!='__pycache__']
    for n in fs:
     f=Path(d)/n;s=f.lstat()
     if not stat.S_ISREG(s.st_mode) or s.st_size<128*1024:continue
     relative=f.relative_to(w)
     if relative.parent==Path('experiments/results'):continue
     groups[(str(relative),s.st_dev,s.st_size,stat.S_IMODE(s.st_mode),s.st_uid,s.st_gid)].append((f,s.st_ino,w))
 candidates={k:v for k,v in groups.items() if len({item[1] for item in v})>1}
 print('Candidate groups',len(candidates),flush=True)
 # A path must still equal its committed index blob before any replacement.
 committed={}
 for w in worktrees:
  output=subprocess.check_output(['git','ls-files','--stage','-z','--','experiments/results','docs/benchmarks'],cwd=w)
  committed[w]={}
  for row in output.split(b'\0'):
   if not row:continue
   meta,name=row.split(b'\t',1);mode,blob,stage=meta.split()
   if stage==b'0':committed[w][os.fsdecode(name)]=blob.decode()
 records=[];content_groups=0
 for key,items in candidates.items():
  relative,dev,size,mode,uid,gid=key;canonical={}
  for p,inode,w in items:
   info=p.lstat();assert stat.S_ISREG(info.st_mode) and (info.st_dev,info.st_size,stat.S_IMODE(info.st_mode),info.st_uid,info.st_gid)==key[1:]
   expected=committed[w].get(relative)
   if not expected:continue
   blob=subprocess.check_output(['git','hash-object','--no-filters','--',str(p)],cwd=w,text=True).strip()
   if blob!=expected:continue
   digest=sha(p)
   if digest not in canonical:canonical[digest]=(p,p.stat().st_ino);content_groups+=1;continue
   q,qinode=canonical[digest]
   if p.stat().st_ino==qinode:continue
   assert same(p,q)
   before=p.stat();old_inode=before.st_ino;old_links=before.st_nlink
   temp=p.with_name(p.name+'.coalesced-immutable-evidence');assert not temp.exists()
   os.link(q,temp);os.replace(temp,p)
   after=p.stat();assert sha(p)==digest and same(p,q)
   assert (after.st_dev,after.st_size,stat.S_IMODE(after.st_mode),after.st_uid,after.st_gid)==key[1:]
   records.append(dict(path=str(p),canonical=str(q),sha256=digest,git_blob=blob,bytes=size,mode=mode,uid=uid,gid=gid,old_inode=old_inode,old_links=old_links,all_logical_paths_and_committed_bytes_preserved=True))
 result=dict(exit_code=0,command=[sys.executable,str(Path(__file__).resolve())],cwd=str(Path.cwd()),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R/'worktree',text=True).strip(),started_unix=started,finished_unix=time.time(),script_sha256=sha(Path(__file__)),files=records,scope='Only committed, byte/hash/mode/owner/device-identical immutable evidence within closed historical worktrees. Current and original producer worktrees excluded; no raw, source, build, object, library, binary, index or top-level aggregate touched. Atomic hardlinks preserve every logical path and file byte.')
 out=R/'closed_evidence_storage.json'
 with out.open('x') as f:f.write(json.dumps(result,indent=2)+'\n')
 print('Coalesced paths',len(records),'with every byte and mode preserved',flush=True)
if __name__=='__main__':main()
