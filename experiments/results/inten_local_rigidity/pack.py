"""Losslessly retain the complete witness and every boundary point; no sampling."""
from pathlib import Path
import hashlib,json,lzma,shutil
import common
E=common.E

def pack(source,execution):
    assert common.read(execution/'analyze_command.json')['exit_code']==0
    report=common.read(source/'summary.json');assert report['exit_code']==0
    assert set(report['groups'])=={'old','01','02','08'}
    files=['summary.json']+[g+'/points.csv' for g in ('old','01','02','08')]
    files.extend('execution/'+n for n in ('analyze_command.json','analyze.stdout.log','analyze.stderr.log'))
    stage=source/'execution';stage.mkdir()
    for n in ('analyze_command.json','analyze.stdout.log','analyze.stderr.log'):
        shutil.copyfile(execution/n,stage/n)
    payload={};meta={}
    for name in files:
        raw=(source/name).read_bytes()
        data=lzma.compress(raw,format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=9|lzma.PRESET_EXTREME)
        assert lzma.decompress(data)==raw
        payload[name]=data
        meta[name]=dict(path=name+'.xz',bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest(),stored_sha256=hashlib.sha256(data).hexdigest())
    assert shutil.disk_usage(E).free>2*sum(map(len,payload.values()))+1024*1024,'Insufficient workspace for complete report, Git object and reserve'
    target=E/'report';target.mkdir()
    for name,data in payload.items():
        p=target/(name+'.xz');p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
    common.write(target/'manifest.json',dict(files=meta,compression='XZ CRC64 preset9 EXTREME; complete original bytes, full precision, no omitted rows',
        pack_source_sha256=common.sha(Path(__file__)),report_reader_sha256=common.sha(E/'report_io.py')))
    reader=common.module('local_rigidity_pack_reader',E/'report_io.py')
    assert reader.read_report()==report
    for name in files:assert reader.read_file(name)==(source/name).read_bytes()
    print('Every complete derived file and actual analysis receipt retained losslessly.')

if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--execution',type=Path,required=True)
    a=p.parse_args();pack(a.source,a.execution)
