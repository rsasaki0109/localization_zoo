import subprocess, pathlib, os, hashlib, json, gzip, stat
R=pathlib.Path(__file__).resolve().parent
OLD=R.parent/'inten-zero-consensus-control/worktree'
W=R/'worktree'
base='a4bb9bf1f09adefcc9ea80c631b56c3ead68fc0c'
assert subprocess.check_output(['git','status','--porcelain'],cwd=OLD)==b''
subprocess.run(['git','worktree','add','--no-checkout','-b','codex/inten-nonfacade-consensus-support',str(W),base],cwd=OLD,check=True)
subprocess.run(['git','read-tree',base],cwd=W,check=True)
rows=subprocess.check_output(['git','ls-files','--stage','-z'],cwd=W).split(b'\0')
normal=[]; shared=[]
source_suffix={'.py','.cpp','.h','.hpp','.c','.sh','.cmake','.txt','.md'}
for row in filter(None,rows):
    meta,name=row.split(b'\t'); mode,oid,stage=meta.split(); name=os.fsdecode(name)
    old=OLD/name; target=W/name
    evidence=name.startswith(('experiments/results/','docs/benchmarks/')) and pathlib.Path(name).suffix not in source_suffix and mode==b'100644'
    if not evidence: normal.append(os.fsencode(name));continue
    before=old.stat();assert stat.S_ISREG(before.st_mode) and not old.is_symlink()
    raw=old.read_bytes(); assert hashlib.sha1(b'blob '+str(len(raw)).encode()+b'\0'+raw).hexdigest()==oid.decode(),name
    target.parent.mkdir(parents=True,exist_ok=True); os.link(old,target)
    after=target.stat();assert (before.st_dev,before.st_ino,before.st_mode,before.st_uid,before.st_gid)==(after.st_dev,after.st_ino,after.st_mode,after.st_uid,after.st_gid)
    shared.append(dict(path=name,git_blob=oid.decode(),sha256=hashlib.sha256(raw).hexdigest(),bytes=len(raw),device=before.st_dev,inode=before.st_ino,mode=before.st_mode,uid=before.st_uid,gid=before.st_gid))
subprocess.run(['git','checkout-index','-z','--stdin'],cwd=W,input=b'\0'.join(normal)+b'\0',check=True)
assert subprocess.check_output(['git','status','--porcelain'],cwd=W)==b''
assert subprocess.check_output(['git','status','--porcelain'],cwd=OLD)==b''
receipt=dict(base=base,source=str(OLD),target=str(W),scope='Closed committed immutable evidence; every logical path retained. Source, builds, libraries, executables, indexes and active outputs excluded.',files=shared)
with gzip.GzipFile(filename=str(R/'worktree_storage.json.gz'),mode='wb',mtime=0) as out: out.write((json.dumps(receipt,indent=2)+'\n').encode())
print(json.dumps(dict(files=len(shared),logical_bytes=sum(x['bytes'] for x in shared),normal_files=len(normal),exit_code=0)))
