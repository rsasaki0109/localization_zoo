"""Preserve and export every generated table byte using lossless transport."""
from pathlib import Path
import hashlib,json,lzma,shutil
E=Path(__file__).resolve().parent
NAMES=('cases.md','cases.summary.json','locations.md')
def digest(data):return hashlib.sha256(data).hexdigest()

def read_file(name,folder=E):
    meta=json.loads((folder/'tables_manifest.json').read_text())['files'][name]
    data=(folder/meta['path']).read_bytes()
    if digest(data)!=meta['stored_sha256']:raise ValueError('Stored complete table changed')
    raw=lzma.decompress(data) if meta['storage']=='xz' else data
    if len(raw)!=meta['bytes'] or digest(raw)!=meta['sha256']:raise ValueError('Original complete table changed')
    return raw

def pack(source,target=E):
    payload={};files={}
    for name in NAMES:
        raw=(source/name).read_bytes();compressed=name!='cases.md'
        data=lzma.compress(raw,format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=9|lzma.PRESET_EXTREME) if compressed else raw
        assert (lzma.decompress(data) if compressed else data)==raw
        path=name+'.xz' if compressed else name
        payload[path]=data
        files[name]=dict(path=path,storage='xz' if compressed else 'plain',bytes=len(raw),sha256=digest(raw),stored_sha256=digest(data))
    changed={name:data for name,data in payload.items() if not (target/name).is_file() or (target/name).read_bytes()!=data}
    assert shutil.disk_usage(target.parent).free>2*sum(map(len,changed.values()))+1024*1024,'Insufficient space for complete tables plus Git objects and reserve'
    target.mkdir(parents=True,exist_ok=True)
    for name,data in changed.items():(target/name).write_bytes(data)
    (target/'tables_manifest.json').write_text(json.dumps(dict(files=files,
        compression='XZ CRC64 preset9 EXTREME, complete original bytes; no row omission, precision change or numerical normalization',
        pack_source_sha256=digest(Path(__file__).read_bytes())),indent=2)+'\n')
    for name in NAMES:assert read_file(name,target)==(source/name).read_bytes()
    print('Complete generated tables retained with byte-exact roundtrip.')

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path);p.add_argument('--output-dir',type=Path,required=True);p.add_argument('--export',action='store_true')
    a=p.parse_args()
    if a.export:
        a.output_dir.mkdir(parents=True,exist_ok=False)
        for name in NAMES:(a.output_dir/name).write_bytes(read_file(name))
    else:
        assert a.source is not None;pack(a.source,a.output_dir)
