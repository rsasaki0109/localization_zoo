"""Run all fixed natural-seed terminal detector interventions; keep every case."""
from pathlib import Path
import json
import tempfile
import numpy as np
from run import E, W, R, build, read, write, run, sha, module


def main():
    assert read(R/'baseline_sealed.json')['exit_code']==0
    stage=R/'terminal';stage.mkdir()
    assert sha(E/'terminal_native.cpp')==read(E/'terminal_protocol.json')['native_source_sha256']
    command=read(R/'staging/compile_native_command.json')['command']
    command=[str(E/'terminal_native.cpp') if a==str(E/'diagnostic_native.cpp') else str(R/'terminal.o') if a==str(R/'native.o') else a for a in command]
    assert run('compile_native',command,stage)==0
    command=read(R/'staging/compile_probe_command.json')['command']
    command=[str(E/'terminal_probe.cpp') if a==str(E/'probe.cpp') else str(R/'terminal.o') if a==str(R/'native.o') else str(R/'terminal_probe') if a==str(R/'probe') else a for a in command]
    assert run('compile_probe',command,stage)==0
    write(stage/'binary.json',dict(path=str(R/'terminal_probe'),sha256=sha(R/'terminal_probe'),object_sha256=sha(R/'terminal.o'),linked_static_libraries=read(R/'staging/binary.json')['linked_static_libraries']))
    traces={r['frame_index']:r for r in map(json.loads,(R/'staging/native.stdout').read_text().splitlines())}
    masks={(r['frame'],r['stage']):[c[0] for c in r['clusters'] if c[4]] for r in map(json.loads,(R/'staging/masks.jsonl').read_text().splitlines())}
    raw=R/'kitti/dataset/sequences/01/velodyne';lines=[]
    for i in range(1,len(traces)):
        source=masks[i,0];target=masks.get((i-1,2),[])
        labels=' '.join(map(str,[len(source),*source,len(target),*target]))
        seed=' '.join(format(v,'.17g') for v in traces[i]['s2s']['input'])
        for mode in range(3):lines.append(f'{i} {mode} {raw/(str(i-1).zfill(6)+".bin")} {raw/(str(i).zfill(6)+".bin")} {labels} {seed}')
    payload=('\n'.join(lines)+'\n').encode();(stage/'input.txt').write_bytes(payload)
    assert run('native',build.docker()+[str(R/'terminal_probe')],stage,payload)==0
    assert sha(R/'terminal_probe')==read(stage/'binary.json')['sha256']
    restore=module('terminal_restore',W/'experiments/results/inten_natural_traces/diagnose.py').restore
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp=Path(temp);restore(W/'experiments/results/inten_facade_pca/sequences/01',temp)
        s=read(temp/'candidate/summary.json');a=np.loadtxt(temp/'candidate'/s['artifacts']['evaluated_gt_path']).reshape(-1,3,4)
        gt=np.tile(np.eye(4),(len(a),1,1));gt[:,:3,:]=a
    data=list(map(json.loads,(stage/'native.stdout').read_text().splitlines()))
    assert [(r['frame'],r['mode']) for r in data]==[(i,m) for i in range(1,len(traces)) for m in range(3)]
    metrics=[]
    for start in range(0,len(data),3):
        rows=data[start:start+3];i=rows[0]['frame'];base=rows[0]
        assert base['output']==traces[i]['s2s']['output'],('pair output changed',i)
        assert [c['n'] for c in base['first_systems']]==traces[i]['s2s']['iterations'][0]['class_pairs']
        rel=np.linalg.inv(gt[i-1])@gt[i];length=float(np.linalg.norm(rel[:3,3]));record=dict(frame=i,gt_step_m=length,moving=length>=.1,arms=[])
        for row in rows:
            for c in (0,1,3):assert row['first_systems'][c]==base['first_systems'][c],(i,row['mode'],c)
            T=np.array(row['output']).reshape(4,4);ratio=float(np.linalg.norm(T[:3,3])/length)
            record['arms'].append(dict(mode=row['mode'],length_ratio=ratio,length_error=abs(ratio-1),vector_error=float(np.linalg.norm(T[:3,3]-rel[:3,3])/length),first_pairs=[c['n'] for c in row['first_systems']]))
        record['baseline_bad_length']=record['moving'] and record['arms'][0]['length_error']>.2
        for v in record['arms'][1:]:
            for metric in ('length_error','vector_error'):
                d=v[metric]-record['arms'][0][metric];v[metric+'_vs_base']='equal' if abs(d)<=1e-12 else 'lower' if d<0 else 'higher'
        metrics.append(record)
    with (stage/'metrics.jsonl').open('x') as out:
        for r in metrics:out.write(json.dumps(r,separators=(',',':'),allow_nan=False)+'\n')
    summary={}
    for label,values in [('all_moving',[r for r in metrics if r['moving']]),('baseline_bad_length',[r for r in metrics if r['baseline_bad_length']]),('other_moving',[r for r in metrics if r['moving'] and not r['baseline_bad_length']])]:
        summary[label]={}
        for mode in (1,2):
            summary[label][str(mode)]={}
            for metric in ('length_error','vector_error'):
                summary[label][str(mode)][metric]={name:sum(r['arms'][mode][metric+'_vs_base']==name for r in values) for name in ('lower','higher','equal')}
            summary[label][str(mode)]['new_bad_length_frames']=[r['frame'] for r in values if r['arms'][0]['length_error']<=.2 and r['arms'][mode]['length_error']>.2]
            summary[label][str(mode)]['remaining_bad_length_frames']=[r['frame'] for r in values if r['arms'][mode]['length_error']>.2]
    write(stage/'report.json',dict(exit_code=0,native_calls=len(data),all_natural_baseline_outputs_exact=True,non_facade_first_equations_exact=True,summary=summary,
          endpoints=[r for r in metrics if r['frame'] in read(E/'protocol.json')['selected_endpoints']],limits=read(E/'terminal_protocol.json')['analysis']))
    archive=module('terminal_archive',W/'experiments/results/inten_natural_traces/run_sequences.py').archive
    archive(stage,E/'terminal')
    write(R/'terminal_complete.json',dict(exit_code=0))


if __name__=='__main__':main()
