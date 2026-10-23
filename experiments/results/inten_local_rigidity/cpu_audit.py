"""Audit full derived-byte portability without altering frozen measurements."""
import concurrent.futures,csv,hashlib,io,json
from pathlib import Path
import numpy as np
import common
E,S=common.E,common.S
reader=common.module('rigidity_cpu_reader',E/'report_io.py')
original=common.module('rigidity_cpu_original_tolerances',common.P/'verify_mixed.py')

def check(core,output,baseline):
    actual=common.read(output/'summary.json');point_bytes={};deltas={}
    for group in ('old','01','02','08'):
        raw=(output/group/'points.csv').read_bytes();expected=reader.read_file(group+'/points.csv')
        point_bytes[group]=raw==expected
        left=list(csv.DictReader(io.StringIO(raw.decode())));right=list(csv.DictReader(io.StringIO(expected.decode())))
        assert len(left)==len(right)
        real=('axis_g','axis_H','mean_absolute_length_error_m','maximum_absolute_length_error_m')
        for a,b in zip(left,right):
            assert a.keys()==b.keys()
            for key in a:
                if key not in real:assert a[key]==b[key]
                else:assert (a[key]=='')==(b[key]=='')
        deltas[group]={}
        for key in real:
            a=np.array([float(row[key]) for row in left if row[key]!='']);b=np.array([float(row[key]) for row in right if row[key]!=''])
            original.checker.close(a,b,'CPU point values differ at original tolerance')
            deltas[group][key]=float(np.max(np.abs(a-b))) if len(a) else None
        actual['groups'][group]['points']=baseline['groups'][group]['points']
    original.inputs.F.equivalent(actual,baseline)
    normalized=common.read(output/'summary.json')
    exact=normalized==baseline and all(point_bytes.values())
    return dict(core=core,exit_code=0,complete_summary_and_point_bytes_exact=exact,point_bytes_exact=point_bytes,
        maximum_absolute_point_delta=deltas,original_rtol=1e-10,original_atol=1e-12,
        complete_summary_sha256=common.sha(output/'summary.json'),points={g:common.sha(output/g/'points.csv') for g in point_bytes})

def main():
    stage=S/'cpu_audit';stage.mkdir();baseline=reader.read_report();outputs={core:S/('cpu_'+core) for core in ('Haswell','Sandybridge')}
    commands={core:common.PREFIX[:2]+['OPENBLAS_CORETYPE='+core]+common.PREFIX[2:]+[str(E/'analyze.py'),'--output-dir',str(output)] for core,output in outputs.items()}
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        futures={core:pool.submit(common.run,core,command,stage) for core,command in commands.items()}
        codes={core:f.result() for core,f in futures.items()}
    result=dict(exit_code=1,commands=codes,controls={})
    try:
        assert all(v==0 for v in codes.values())
        result['controls']={core:check(core,output,baseline) for core,output in outputs.items()}
        result['exit_code']=0 if all(v['complete_summary_and_point_bytes_exact'] for v in result['controls'].values()) else 1
    except Exception as error:result['error']=str(error)
    common.write(stage/'result.json',result)
    common.module('rigidity_cpu_archive',common.P/'archive.py').archive(stage,E/'checks/cpu_audit')
    return result['exit_code']
if __name__=='__main__':raise SystemExit(main())
