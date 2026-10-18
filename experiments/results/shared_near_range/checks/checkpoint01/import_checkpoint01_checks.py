"""Import closed documentation checks only; never stage a live sequence."""
from pathlib import Path
import gzip
import hashlib
import json
import re
import subprocess

repo=Path('/workspace/localization_zoo')
e=repo/'experiments/results/shared_near_range'
r=Path('/workspace/localization-env/shared-near-range')
raw=r/'checkpoint01_docs'
target=e/'checks/checkpoint01'
assert not target.exists()
assert json.loads((raw/'exit.json').read_text())['exit_code']==0
checkpoint=json.loads((e/'checkpoints/01/summary.json').read_text())
assert checkpoint['completed_sequences']==['04','00','01']
generated=['docs/'+name+'.md' for name in ('decisions','experiments','interfaces',
    'paper_assets','paper_captions','paper_comparison','paper_roadmap','paper_tracks',
    'reproduction_status','variant_analysis')]+['experiments/results/index.json']
identity=[]
for name in generated:
    before=subprocess.check_output(['git','show',checkpoint['evidence_revision']+':'+name],cwd=repo,text=True)
    after=(repo/name).read_text()
    if name.endswith('.json'):
        a,b=json.loads(before),json.loads(after)
        a.pop('generated_at'); b.pop('generated_at')
    else:
        pattern=r'(?m)^(_Generated at |> Generated: )\S+'
        a,ca=re.subn(pattern,r'\1<TIMESTAMP>',before)
        b,cb=re.subn(pattern,r'\1<TIMESTAMP>',after)
        assert ca==cb==1,name
    assert a==b,name
    identity.append(dict(path=name,identical_except_generation_timestamp=True))
(raw/'generated_identity.json').write_text(json.dumps(dict(checks=identity,exit_code=0),indent=2)+'\n')
target.mkdir(parents=True)
files=[]
for path in sorted(raw.iterdir()):
    if not path.is_file(): continue
    data=path.read_bytes()
    dest=target/(path.name+'.gz' if path.suffix=='.log' else path.name)
    dest.write_bytes(gzip.compress(data,mtime=0) if path.suffix=='.log' else data)
    if path.suffix=='.log': assert gzip.decompress(dest.read_bytes())==data
    files.append(dict(source=str(path),destination=str(dest.relative_to(repo)),
        original_sha256=hashlib.sha256(data).hexdigest(),stored_sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
for name in ['document_checkpoint01.py','check_checkpoint01.py','import_checkpoint01_checks.py']:
    data=(r/name).read_bytes()
    (target/name).write_bytes(data)
    files.append(dict(source=str(r/name),destination=str((target/name).relative_to(repo)),
        original_sha256=hashlib.sha256(data).hexdigest(),stored_sha256=hashlib.sha256(data).hexdigest()))
documents=r/'checkpoint01_documents'
if documents.exists():
    for path in sorted(documents.iterdir()):
        if not path.is_file(): continue
        data=path.read_bytes()
        dest=target/(path.name+'.gz' if path.suffix=='.log' else path.name)
        dest.write_bytes(gzip.compress(data,mtime=0) if path.suffix=='.log' else data)
        files.append(dict(source=str(path),destination=str(dest.relative_to(repo)),
            original_sha256=hashlib.sha256(data).hexdigest(),stored_sha256=hashlib.sha256(dest.read_bytes()).hexdigest()))
(target/'import.json').write_text(json.dumps(dict(files=files,exit_code=0),indent=2)+'\n')
print('Imported closed checkpoint01 docs checks, exact drivers and lossless logs.')
