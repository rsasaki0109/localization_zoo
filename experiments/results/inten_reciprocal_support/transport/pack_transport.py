from pathlib import Path
import hashlib,json,lzma,shutil,sys
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree');E=W/'experiments/results/inten_reciprocal_support'
sys.path.insert(0,str(E))
from common import S,write,sha
stage=S/'transport';stage.mkdir();records={}
science_sha=sha(E/'protocol.json')
write(stage/'plan.json',dict(scientific_protocol_sha256=science_sha,format='XZ/CRC64, preset6; raw SHA256 and complete original gzip archives retained',unchanged='All native outputs, source, inputs, settings, verifier mathematics and progression gates'))
for group in ('old','01','02','08'):
    before=E/'groups'/group;meta=json.loads((before/'archive.json').read_text());entry=meta['files']['native.stdout.jsonl']
    raw=S/('native_'+group)/'native.stdout.jsonl';assert sha(raw)==entry['sha256'] and raw.stat().st_size==entry['bytes']
    out=stage/(group+'.jsonl.xz')
    with raw.open('rb') as source,lzma.open(out,'wb',format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=6) as target:
        shutil.copyfileobj(source,target,1024*1024)
    digest=hashlib.sha256();total=0
    with lzma.open(out,'rb') as stream:
        while data:=stream.read(1024*1024):digest.update(data);total+=len(data)
    assert digest.hexdigest()==entry['sha256'] and total==entry['bytes']
    records[group]=dict(raw_sha256=entry['sha256'],raw_bytes=entry['bytes'],xz_sha256=sha(out),xz_bytes=out.stat().st_size,original_manifest_sha256=sha(before/'archive.json'))
    print(group,'XZ byte restoration verified',flush=True)
# All XZ encodings restore before replacing any uncommitted repository archive.
saved=S/'original_gzip_archives';saved.mkdir()
for group,record in records.items():
    old=E/'groups'/group;original=saved/group;shutil.move(str(old),original)
    new=old;new.mkdir()
    meta=json.loads((original/'archive.json').read_text())
    for name,entry in meta['files'].items():
        if name=='native.stdout.jsonl':continue
        assert entry['storage'] in ('verbatim','gzip')
        target=new/entry['path'];target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(original/entry['path'],target)
    shutil.copyfile(stage/(group+'.jsonl.xz'),new/'native.stdout.jsonl.xz')
    shutil.copyfile(original/'archive.json',new/'original_gzip_archive.json')
    entry=meta['files']['native.stdout.jsonl']
    entry.pop('parts');entry.update(storage='xz',path='native.stdout.jsonl.xz',stored_sha256=record['xz_sha256'])
    write(new/'archive.json',meta)
assert sha(E/'protocol.json')==science_sha
transport=E/'transport';transport.mkdir();shutil.copyfile(Path(__file__),transport/'pack_transport.py')
protocol=dict(exit_code=0,scientific_protocol_sha256=science_sha,groups=records,
    source_sha256={n:sha(E/n) for n in ('verify_portable.py','transport/pack_transport.py')},
    original_gzip_archives_retained_private=str(saved),all_raw_bytes_restore_exactly=True,
    reason='New gzip logs exceed remaining overlay capacity when also stored as Git objects. XZ changes transport only; scientific native source, outputs, inputs, solver/mask verifier and gate are unchanged.')
write(E/'transport_recovery_protocol.json',protocol);write(stage/'result.json',protocol)
print('All original gzip archives preserved; canonical raw measurement SHA256 values are unchanged.')
