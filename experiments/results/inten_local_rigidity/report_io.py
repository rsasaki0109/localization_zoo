"""Read every complete derived byte, verifying compressed and original hashes."""
from pathlib import Path
import hashlib,json,lzma
E=Path(__file__).resolve().parent

def read_file(name):
    meta=json.loads((E/'report/manifest.json').read_text())['files'][name]
    data=(E/'report'/meta['path']).read_bytes()
    if hashlib.sha256(data).hexdigest()!=meta['stored_sha256']:raise ValueError('Stored derived evidence changed')
    raw=lzma.decompress(data)
    if len(raw)!=meta['bytes'] or hashlib.sha256(raw).hexdigest()!=meta['sha256']:
        raise ValueError('Complete derived evidence changed')
    return raw

def read_report():return json.loads(read_file('summary.json'))
