"""Retain every verified result byte without duplicating complete worker subtrees."""
from pathlib import Path
import gzip,hashlib,importlib.util,json,lzma,shutil
E=Path(__file__).resolve().parent
def sha(data):return hashlib.sha256(data).hexdigest()
def read(path):return json.loads(path.read_text())
def main(source):
    command=read(source/'verify_compact_command.json');assert command['exit_code']==0
    raw=(source/'summary.json').read_bytes();report=json.loads(raw)
    assert report['exit_code']==0 and report['all_558_historical_native_outcomes_exact']
    assert set(report['groups'])=={'old','01','02','08'}
    workers={}
    stage=source/'execution_archive_stage';stage.mkdir()
    for name in ('verify_compact_command.json','verify_compact.stdout.log','verify_compact.stderr.log'):
        shutil.copyfile(source/name,stage/name)
    folder=stage/'workers';folder.mkdir()
    for group in ('old','01','02','08'):
        result=source/'workers'/(group+'.json');original=result.read_bytes();payload=json.loads(original)
        receipt=read(source/'workers'/(group+'_command.json'));assert receipt['exit_code']==0
        assert payload['result']==report['groups'][group] and receipt['output_sha256']==sha(original)
        rebuilt=json.dumps(dict(group=group,result=report['groups'][group],producer_sha256=payload['producer_sha256'],
            native_archive_sha256=payload['native_archive_sha256']),indent=2,allow_nan=False).encode()+b'\n'
        assert rebuilt==original,'Worker subtree byte round-trip differs'
        workers[group]=dict(bytes=len(original),sha256=sha(original),producer_sha256=payload['producer_sha256'],native_archive_sha256=payload['native_archive_sha256'])
        for suffix in ('_command.json','.stdout.log','.stderr.log'):
            shutil.copyfile(source/'workers'/(group+suffix),folder/(group+suffix))
    compressed=lzma.compress(raw,format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=9|lzma.PRESET_EXTREME)
    assert lzma.decompress(compressed)==raw
    assert shutil.disk_usage(E).free>2*len(compressed)+1024*1024,'Insufficient workspace for report plus Git object and reserve'
    target=E/'report';target.mkdir()
    (target/'summary.json.xz').write_bytes(compressed)
    manifest=dict(path='summary.json.xz',bytes=len(raw),sha256=sha(raw),stored_sha256=sha(compressed),
        compression='XZ/CRC64 preset9 EXTREME; complete original JSON bytes, no numerical normalization',
        worker_reports=workers,
        worker_roundtrip="json.dumps(dict(group=group,result=report['groups'][group],producer_sha256=header['producer_sha256'],native_archive_sha256=header['native_archive_sha256']),indent=2,allow_nan=False).encode()+b'\\n'",
        pack_source_sha256=sha(Path(__file__).read_bytes()),report_reader_sha256=sha((E/'report_io.py').read_bytes()))
    (target/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    s=importlib.util.spec_from_file_location('support_report_archive',E/'archive.py');archive=importlib.util.module_from_spec(s);s.loader.exec_module(archive)
    archive.archive(stage,target/'execution')
    s=importlib.util.spec_from_file_location('support_report_readback',E/'report_io.py');io=importlib.util.module_from_spec(s);s.loader.exec_module(io)
    assert io.read_report()==report
    print('Complete global/worker numerical output and every actual exit retained losslessly.')
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);a=p.parse_args();main(a.source)
