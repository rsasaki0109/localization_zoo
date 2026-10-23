"""Lossless interleaving against unchanged committed parent stdout, chosen prospectively."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,lzma,shutil
E=Path(__file__).resolve().parent
_s=importlib.util.spec_from_file_location('support_parent_archive',E.parent/'inten_prior_control/archive.py')
_p=importlib.util.module_from_spec(_s);_s.loader.exec_module(_p)
sha=_p.sha
def archive(source,target):
    native=source/'native.stdout.jsonl'
    if not native.exists():
        _p.archive(source,target);return
    group=target.name;parent=E.parent/'inten_prior_control/groups'/group
    meta=json.loads((parent/'archive.json').read_text())['files']['native.stdout.jsonl']
    compressed=parent/meta['path']
    assert sha(compressed)==meta['stored_sha256']
    extras=source/'support.extra.jsonl';parent_sha=hashlib.sha256();parent_bytes=0;ordinal=0;extra_count=0
    with native.open('rb') as incoming,lzma.open(compressed,'rb') as canonical,extras.open('xb') as out:
        for line in incoming:
            record=json.loads(line)
            if record['type'].startswith('support_'):
                out.write((json.dumps(dict(before_parent_line=ordinal,raw_line=line.decode('utf-8')),separators=(',',':'))+'\n').encode())
                extra_count+=1
            else:
                expected=next(canonical,None)
                assert line==expected,('Canonical native record differs',group,ordinal)
                parent_sha.update(line);parent_bytes+=len(line);ordinal+=1
        assert next(canonical,None) is None,'Missing canonical tail'
    assert parent_sha.hexdigest()==meta['sha256'] and parent_bytes==meta['bytes']
    descriptor=dict(storage='interleaved_parent_jsonl',parent_archive=str((parent/'archive.json').relative_to(E.parents[2])),
        parent_archive_sha256=sha(parent/'archive.json'),parent_name='native.stdout.jsonl',
        extra_name='support.extra.jsonl',parent_lines=ordinal,extra_records=extra_count,
        bytes=native.stat().st_size,sha256=sha(native),all_parent_native_stdout_bytes_exact=True)
    # Reconstruct without materializing the large stream a second time.
    digest=hashlib.sha256();size=0;pos=0
    with lzma.open(compressed,'rb') as canonical,extras.open('r') as inserted:
        for line in inserted:
            row=json.loads(line);assert row['before_parent_line']>=pos
            while pos<row['before_parent_line']:
                data=next(canonical);digest.update(data);size+=len(data);pos+=1
            data=row['raw_line'].encode();digest.update(data);size+=len(data)
        for data in canonical:digest.update(data);size+=len(data);pos+=1
    assert size==descriptor['bytes'] and digest.hexdigest()==descriptor['sha256']
    # Keep native stdout at its actual raw location. Archive every other source
    # artifact plus the complete inserted bytes and parent reference.
    stage=source/'referenced_archive_stage';stage.mkdir()
    for path in source.iterdir():
        if path.is_file() and path!=native:shutil.copyfile(path,stage/path.name)
    (stage/'native.stdout.reference.json').write_text(json.dumps(descriptor,indent=2)+'\n')
    _p.archive(stage,target)
