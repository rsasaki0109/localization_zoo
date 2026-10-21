"""Share only identical small immutable evidence; retain all paths/bytes/modes."""
from pathlib import Path
import hashlib,json,os,subprocess,time
R=Path(__file__).resolve().parent;W=R/'worktree'
def git(w,*a):return subprocess.check_output(['git','--no-optional-locks',*a],cwd=w,text=True).strip()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    if git(W,'status','--porcelain'):raise RuntimeError('Canonical scientific snapshot must be clean')
    targets=[]
    for block in git(W,'worktree','list','--porcelain').split('\n\n'):
        fields={line.split(' ',1)[0]:line.split(' ',1)[1] for line in block.splitlines() if ' ' in line}
        path=Path(fields['worktree'])
        if path!=W and str(path).startswith('/workspace/localization-env/inten-'):
            if git(path,'status','--porcelain'):raise RuntimeError('Historical worktree is dirty: '+str(path))
            targets.append(dict(path=str(path),head=fields['HEAD']))
    allowed={'.json','.csv','.txt','.gz','.jsonl','.png','.log','.stdout','.stderr'}
    cached={};plans=[];skipped=0
    for target in targets:
        w=Path(target['path'])
        for rel in git(w,'ls-files','experiments/results','docs/benchmarks').splitlines():
            p=w/rel;q=W/rel
            if Path(rel).parent==Path('experiments/results') or Path(rel).suffix not in allowed:continue
            if p.is_symlink() or q.is_symlink() or not p.is_file() or not q.is_file():continue
            a,b=p.stat(),q.stat()
            if not (a.st_size>=100000) or a.st_ino==b.st_ino:continue
            if a.st_mode!=b.st_mode or a.st_size!=b.st_size:skipped+=1;continue
            expected=cached.setdefault(rel,sha(q))
            if sha(p)!=expected:skipped+=1;continue
            plans.append(dict(target=str(w),path=rel,sha256=expected,bytes=a.st_size,mode=a.st_mode))
    proof=dict(started_unix=time.time(),canonical=str(W),canonical_head=git(W,'rev-parse','HEAD'),targets=targets,
        files=plans,skipped_nonidentical=skipped,complete=False,
        policy='Only at-least-100kB byte/hash/mode-identical immutable evidence. Every logical path and byte/mode preserved. No code, build, binary, index, top-level aggregate, active study or scientific value changes.')
    output=R/'large_storage.json';output.write_text(json.dumps(proof,indent=2)+'\n')
    for item in plans:
        p=Path(item['target'])/item['path'];q=W/item['path'];temp=p.with_name(p.name+'.storage-link')
        if temp.exists() or sha(p)!=item['sha256'] or sha(q)!=item['sha256']:raise RuntimeError('Evidence changed during storage proof')
        os.link(q,temp);os.replace(temp,p)
        if sha(p)!=item['sha256'] or p.stat().st_mode!=item['mode']:raise RuntimeError('Storage sharing changed evidence')
    for target in targets:
        if git(Path(target['path']),'rev-parse','HEAD')!=target['head'] or git(Path(target['path']),'status','--porcelain'):raise RuntimeError('Historical worktree changed')
    if git(W,'status','--porcelain'):raise RuntimeError('Canonical worktree changed')
    proof.update(exit_code=0,complete=True,all_worktrees_clean=True,logical_bytes_preserved=sum(x['bytes'] for x in plans),finished_unix=time.time())
    output.write_text(json.dumps(proof,indent=2)+'\n')
    print('Historical logical evidence retained',len(plans),proof['logical_bytes_preserved'])


if __name__=='__main__':main()
