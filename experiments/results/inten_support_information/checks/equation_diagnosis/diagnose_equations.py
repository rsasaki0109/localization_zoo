"""Locate a frozen strict-equation failure; this is not validation success."""
from pathlib import Path
import hashlib,importlib.util,inspect,json,lzma,multiprocessing as mp
import numpy as np
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information')
S=Path('/tmp/localization-zoo-inten-support-information-20261023')/'equation_diagnosis'
S.mkdir(exist_ok=True)
spec=importlib.util.spec_from_file_location('support_frozen_equation_diagnosis',E/'verify_full_field.py')
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)
original_close=v.checker.close
clouds={};exclusions={};group_name=None
def close(a,b,why):
    if not np.allclose(a,b,rtol=1e-10,atol=1e-12):
        local=inspect.currentframe().f_back.f_locals
        if 'record' in local and 'mask' in local:
            record=local['record'];frame=record['frame'];cls=record['class']
            stem=f"{group_name}_{frame}_{record['mode']}_{int(record['translation_only'])}_{record['iteration']}_{cls}"
            x=local['x'][local['mask']];gradient=local['a'][local['mask']];r=local['r'][local['mask']]
            J=np.column_stack((gradient,np.cross(x,gradient)))
            weights=np.ones(len(r));large=np.abs(r)>.1;weights[large]=.1/np.abs(r[large])
            H=np.zeros((6,6));g=np.zeros(6);cost=0.
            for row,weight,residual in zip(J,weights,r):
                weighted=weight*row;H+=np.outer(weighted,row);g+=weighted*residual;cost+=weight*residual*residual
            serial={'H':H,'g':g,'cost':cost}[local['key']]
            aa=np.asarray(a);bb=np.asarray(b);bad=np.argwhere(~np.isclose(aa,bb,rtol=1e-10,atol=1e-12))
            details=dict(scope='Diagnosed equation failure, not successful full validation',group=group_name,frame=frame,
                mode=record['mode'],translation_only=record['translation_only'],iteration=record['iteration'],feature_class=cls,
                partition=local['name'],key=local['key'],why=why,
                vectorized_max_abs_error=float(np.max(np.abs(aa-bb))),serial_max_abs_error=float(np.max(np.abs(serial-bb))),
                serial_matches_original_tolerance=bool(np.allclose(serial,bb,rtol=1e-10,atol=1e-12)),
                failing_entries=[dict(index=i.tolist(),vectorized=float(aa[tuple(i)]),native=float(bb[tuple(i)]),serial=float(np.asarray(serial)[tuple(i)])) for i in bad])
            (S/(stem+'.details.json')).write_text(json.dumps(details,indent=2)+'\n')
            (S/(stem+'.query.json')).write_text(json.dumps(record,separators=(',',':'))+'\n')
            np.savez_compressed(S/(stem+'.clouds.npz'),source=clouds[frame][cls],target=clouds[frame-1][cls])
            print(json.dumps(details),flush=True)
    original_close(a,b,why)
v.checker.close=close
def extra_stream(group):
    folder=E/'groups'/group;entry=json.loads((folder/'archive.json').read_text())['files']['support.extra.jsonl']
    path=folder/entry['path'];assert hashlib.sha256(path.read_bytes()).hexdigest()==entry['stored_sha256']
    with lzma.open(path,'rt') as stream:
        for line in stream:yield json.loads(line)['raw_line']
def worker(raw):
    record=json.loads(raw)
    excluded=exclusions.get((record['frame'],record['mode']),set()) if record['translation_only'] else set()
    try:v.checker.reconstruct(record,clouds,excluded,{},{} )
    except (ValueError,AssertionError) as error:
        return dict(failed=True,why=str(error),frame=record['frame'],mode=record['mode'],iteration=record['iteration'],feature_class=record['class'])
    return None
def diagnose(group):
    global group_name,clouds,exclusions
    group_name=group;v.current_group=group
    v.node_fields[group]={f:v.inputs.F.node_map(field) for f,field in v.inputs.fields(group).items()}
    clouds={};exclusions={}
    for raw in extra_stream(group):
        if '"type":"support_features"' in raw[:80]:
            row=json.loads(raw);clouds[row['frame']]=[np.array(x,float).reshape(-1,6) for x in row['classes']]
    parent=E.parent/'inten_prior_control/groups'/group
    entry=json.loads((parent/'archive.json').read_text())['files']['native.stdout.jsonl']
    with lzma.open(parent/entry['path'],'rt') as stream:
        for line in stream:
            if '"type":"terminal"' in line[:80]:
                row=json.loads(line)
                if row['mode'] in (7,15):exclusions[row['frame'],row['mode']]=set(row['trace']['s2s_consensus']['excluded_clusters'])
    jobs=(raw for raw in extra_stream(group) if '"type":"support_query"' in raw[:80])
    with mp.get_context('fork').Pool(4) as pool:
        for result in pool.imap_unordered(worker,jobs,chunksize=4):
            if result:
                pool.terminate();(S/'located_failure.json').write_text(json.dumps(result,indent=2)+'\n')
                return result
    v.node_fields.pop(group);return None
if __name__=='__main__':
    for group in ('old','01','02','08'):
        result=diagnose(group)
        if result:print('Frozen strict equation discrepancy reproduced and isolated.',flush=True);break
    else:raise AssertionError('Expected strict equation failure did not reproduce')
