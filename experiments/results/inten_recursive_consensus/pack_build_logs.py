"""Losslessly retain original build logs, including failed attempts and whitespace."""
from pathlib import Path
import gzip,hashlib,json,shutil,subprocess,sys
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    name=sys.argv[1];folder=E/'checks'/name;originals=R/'original_build_logs'/name;originals.mkdir(parents=True)
    records=[]
    for p in sorted(folder.iterdir()):
        if p.suffix not in ('.stdout','.stderr'):continue
        raw=p.read_bytes();copy=originals/p.name;shutil.copyfile(p,copy);target=p.with_suffix(p.suffix+'.gz')
        target.write_bytes(gzip.compress(raw,mtime=0));assert gzip.decompress(target.read_bytes())==copy.read_bytes()==raw
        records.append(dict(original_relative_path=str(p.relative_to(W)),private_original=str(copy),raw_sha256=sha(copy),raw_bytes=len(raw),gzip_path=str(target.relative_to(W)),gzip_sha256=sha(target),byte_roundtrip_equal=True))
        p.unlink()
    codes={p.name:json.loads(p.read_text())['exit_code'] for p in sorted(folder.glob('*_command.json'))}
    (folder/'log_archive.json').write_text(json.dumps(dict(exit_code=0,source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),command=[sys.executable,str(Path(__file__).resolve()),name],cwd=str(W),files=records,actual_build_exit_codes=codes,policy='Keep exact raw bytes privately and lossless gzip+mapping in repository; failed exits remain failed, no log sanitation.'),indent=2)+'\n')
if __name__=='__main__':main()
