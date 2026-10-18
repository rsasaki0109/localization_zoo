from pathlib import Path
import gzip,hashlib,json,sys
folder=Path(__file__).resolve().parent
records=[]
for label in ('staged_whitespace_initial','staged_whitespace_final'):
 path=folder/(label+'.log');raw=path.read_bytes()
 packed=gzip.compress(raw,mtime=0)
 assert gzip.decompress(packed)==raw
 archive=path.with_suffix('.log.gz');archive.write_bytes(packed)
 records.append(dict(original_path=path.name,archive=archive.name,raw_sha256=hashlib.sha256(raw).hexdigest(),gzip_sha256=hashlib.sha256(packed).hexdigest(),raw_bytes=len(raw),exit_code=json.loads((folder/(label+'_command.json')).read_text())['exit_code']))
 path.unlink()
(folder/'archived_whitespace_logs.json').write_text(json.dumps(dict(command=[sys.executable,*sys.argv],archives=records,reason='Preserve exact CR-containing failure logs without introducing new Git whitespace errors.',exit_code=0),indent=2)+'\n')
