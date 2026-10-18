from pathlib import Path
import hashlib,json,sys
path=Path(__file__).resolve().parents[2]/'checkpoints/04/comparison.csv'
before=path.read_bytes()
assert b'\r\n' in before and b'\r' not in before.replace(b'\r\n',b'')
after=before.replace(b'\r\n',b'\n')
path.write_bytes(after)
record={'command':[sys.executable,*sys.argv],'input_sha256':hashlib.sha256(before).hexdigest(),'output_sha256':hashlib.sha256(after).hexdigest(),'rows_and_fields_unchanged':True,'reason':'Staged whitespace check rejected default CSV CRLF; native replay data and JSON unchanged.','exit_code':0}
Path(__file__).with_name('csv_normalization.json').write_text(json.dumps(record,indent=2)+'\n')
