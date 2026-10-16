"""Read only an initial bounded prefix of the official NCLT archive."""
import hashlib
import json
from pathlib import Path
import tarfile
import urllib.request

URL='https://s3.us-east-2.amazonaws.com/nclt.perl.engin.umich.edu/velodyne_data/2013-01-10_vel.tar.gz'
ROOT=Path('/workspace/localization-env/sensor-setup/nclt')
LIMIT=16*1024*1024
COUNT=3

class BoundedReader:
    def __init__(self,source):self.source=source;self.consumed=0
    def read(self,size=-1):
        remaining=LIMIT-self.consumed
        if remaining<=0:raise RuntimeError('NCLT compressed prefix limit reached')
        data=self.source.read(min(remaining,1024*1024 if size<0 else size))
        self.consumed+=len(data)
        return data

request=urllib.request.Request(URL,headers={'Range':f'bytes=0-{LIMIT-1}'})
ROOT.mkdir(parents=True,exist_ok=True)
rows=[]
with urllib.request.urlopen(request,timeout=30) as response:
    if response.status!=206:raise RuntimeError(f'Server must honor bounded Range, got {response.status}')
    stream=BoundedReader(response)
    with tarfile.open(fileobj=stream,mode='r|gz') as archive:
        for member in archive:
            if not member.isfile() or not member.name.endswith('.bin'):continue
            name=Path(member.name).name
            if not Path(name).stem.isdecimal() or not 0<member.size<10*1024*1024:raise RuntimeError('Unexpected NCLT scan entry')
            handle=archive.extractfile(member)
            data=handle.read(member.size)
            if len(data)!=member.size:raise RuntimeError('Incomplete NCLT entry')
            target=ROOT/name;target.write_bytes(data)
            rows.append({'archive_entry':member.name,'path':str(target),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
            if len(rows)==COUNT:break
    if len(rows)!=COUNT:raise RuntimeError('Insufficient scans in bounded prefix')
    consumed=stream.consumed
report={'url':URL,'compressed_byte_limit':LIMIT,'compressed_bytes_read':consumed,'frames':rows}
(ROOT/'download.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
