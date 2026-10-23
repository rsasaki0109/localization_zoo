"""Complete lossless archives; XZ is selected before measurement for large JSONL."""
from pathlib import Path
import gzip,hashlib,json,lzma,shutil
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def archive(source,target):
    target.mkdir(parents=True,exist_ok=False);files={}
    for path in sorted(source.rglob('*')):
        if not path.is_file():continue
        relative=path.relative_to(source)
        entry=dict(original_path=str(path),bytes=path.stat().st_size,sha256=sha(path))
        destination=target/relative;destination.parent.mkdir(parents=True,exist_ok=True)
        if path.suffix=='.jsonl':
            destination=destination.with_suffix('.jsonl.xz')
            with path.open('rb') as src,lzma.open(destination,'wb',format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=6) as dst:shutil.copyfileobj(src,dst,1024*1024)
            digest=hashlib.sha256();size=0
            with lzma.open(destination,'rb') as stream:
                while data:=stream.read(1024*1024):digest.update(data);size+=len(data)
            assert size==entry['bytes'] and digest.hexdigest()==entry['sha256']
            storage='xz'
        elif path.suffix=='.log':
            destination=destination.with_suffix('.log.gz');destination.write_bytes(gzip.compress(path.read_bytes(),mtime=0))
            assert hashlib.sha256(gzip.decompress(destination.read_bytes())).hexdigest()==entry['sha256'];storage='gzip'
        else:
            shutil.copyfile(path,destination);assert sha(destination)==entry['sha256'];storage='verbatim'
        entry.update(storage=storage,path=destination.relative_to(target).as_posix(),stored_sha256=sha(destination));files[relative.as_posix()]=entry
        assert sha(path)==entry['sha256']
    (target/'archive.json').write_text(json.dumps(dict(files=files,byte_roundtrip_verified=True,exit_code=0),indent=2)+'\n')
