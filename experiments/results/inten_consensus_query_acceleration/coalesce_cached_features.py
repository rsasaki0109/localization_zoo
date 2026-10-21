"""Share only byte/mode/owner-identical immutable feature caches; preserve all paths."""
from pathlib import Path
import hashlib,json,os,stat,subprocess,sys
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    roots=['inten-cluster-motion','inten-translation-consensus','inten-fixed-rotation','inten-recursive-consensus','inten-recursive-consensus-accelerated']
    names=['features.txt','parity_features.txt','parity_features_corrected.txt','parity_features_balanced.txt','inputs/features.jsonl','parity_inputs/features.jsonl','parity_inputs_corrected/features.jsonl','parity_inputs_balanced/features.jsonl']
    assert not (R/'collector.lock').exists() and not (R.parent/'inten-recursive-consensus/collector.lock').exists()
    groups={};records=[]
    for root in roots:
        for name in names:
            p=R.parent/root/name
            if not p.is_file() or p.is_symlink():continue
            info=p.stat();key=(info.st_dev,info.st_size,stat.S_IMODE(info.st_mode),info.st_uid,info.st_gid,sha(p))
            if key not in groups:groups[key]=p;continue
            q=groups[key]
            if p.stat().st_ino==q.stat().st_ino:continue
            assert p.read_bytes()==q.read_bytes()
            tmp=p.with_name(p.name+'.shared-evidence');assert not tmp.exists()
            os.link(q,tmp);os.replace(tmp,p)
            assert sha(p)==key[-1] and stat.S_IMODE(p.stat().st_mode)==key[2]
            records.append(dict(path=str(p),canonical=str(q),bytes=info.st_size,mode=key[2],uid=info.st_uid,gid=info.st_gid,sha256=key[-1],all_bytes_modes_and_logical_paths_preserved=True))
    target=E/'checks/cache_storage.json'
    with target.open('x') as out:out.write(json.dumps(dict(exit_code=0,command=[sys.executable,str(Path(__file__).resolve())],cwd=str(W),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),files=records,scope='Only listed immutable full extracted-feature caches; no raw scans, sources, build objects, libraries, binaries or result files changed/deleted.'),indent=2)+'\n')
    print('Immutable feature cache bytes, modes, owners and every logical path preserved')
if __name__=='__main__':main()
