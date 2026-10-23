"""Preserve every original native build log byte using lossless gzip."""
from pathlib import Path
import gzip,hashlib,json,shutil,subprocess,sys
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    own=Path(__file__).resolve()
    if subprocess.check_output(['git','show','HEAD:'+own.relative_to(W).as_posix()],cwd=W)!=own.read_bytes():raise RuntimeError('Commit log packaging source first')
    folder=E/'checks/candidate_build';originals=R/'native_build_logs';originals.mkdir()
    for label in ('configure','build','ctest','runner'):
        if json.loads((folder/(label+'_command.json')).read_text())['exit_code']!=0:raise RuntimeError('Native build/test failed')
    records=[]
    for path in sorted(folder.iterdir()):
        if path.suffix not in ('.stdout','.stderr'):continue
        raw=path.read_bytes();copy=originals/path.name;shutil.copyfile(path,copy)
        target=path.with_suffix(path.suffix+'.gz');target.write_bytes(gzip.compress(raw,mtime=0))
        if gzip.decompress(target.read_bytes())!=raw or copy.read_bytes()!=raw:raise RuntimeError('Log byte roundtrip failed')
        records.append(dict(original_relative_path=str(path.relative_to(W)),original_private_path=str(copy),raw_sha256=sha(copy),raw_bytes=len(raw),
            gzip_path=str(target.relative_to(W)),gzip_sha256=sha(target),byte_roundtrip_equal=True))
        path.unlink()
    (folder/'log_archive.json').write_text(json.dumps(dict(exit_code=0,command=[sys.executable,str(own)],cwd=str(W),
        source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),files=records,
        policy='Preserve every byte, including CMake trailing spaces. No sanitation; original raw logs retained privately, compressed repository copies roundtrip exactly. All four configure/build/CTest/runner exits are zero.'),indent=2)+'\n')
    print('All native build logs retained byte-for-byte')


if __name__=='__main__':main()
