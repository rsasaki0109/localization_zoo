"""Read the complete lossless verification report with stored/raw hash checks."""
from pathlib import Path
import hashlib,json,lzma
E=Path(__file__).resolve().parent
def read_report():
    folder=E/'report';meta=json.loads((folder/'manifest.json').read_text())
    data=(folder/meta['path']).read_bytes()
    if hashlib.sha256(data).hexdigest()!=meta['stored_sha256']:raise ValueError('Compressed report changed')
    raw=lzma.decompress(data)
    if len(raw)!=meta['bytes'] or hashlib.sha256(raw).hexdigest()!=meta['sha256']:raise ValueError('Complete report changed')
    report=json.loads(raw)
    for group,header in meta.get('worker_reports',{}).items():
        rebuilt=json.dumps(dict(group=group,result=report['groups'][group],producer_sha256=header['producer_sha256'],
            native_archive_sha256=header['native_archive_sha256']),indent=2,allow_nan=False).encode()+b'\n'
        if len(rebuilt)!=header['bytes'] or hashlib.sha256(rebuilt).hexdigest()!=header['sha256']:
            raise ValueError('Complete worker report changed')
    return report
