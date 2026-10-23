"""Recover the original all-point intensity construction; never use reflector-only fields."""
from pathlib import Path
import hashlib,importlib.util,json
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
F=module('support_original_field_input',E.parent/'inten_intensity_continuity/verify.py')
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def fields(group):
    folder=E.parent/'inten_intensity_continuity/groups'/group
    recovery=json.loads((E/'field_source_recovery_protocol.json').read_text())
    pins=recovery['field_groups'][group]
    for name in ('archive.json','input_sources.json'):
        assert sha(folder/name)==pins[name],('Prior full-cloud field evidence changed',group,name)
    assert json.loads((folder/'native_command.json').read_text())['exit_code']==0
    previous=json.loads((folder/'input_sources.json').read_text())
    current=json.loads((E/'groups'/group/'input_sources.json').read_text())
    assert previous['feature_cache_sha256']==current['feature_cache_sha256']==pins['feature_cache_sha256']
    result={}
    for line in F.decode(folder,'native.stdout.jsonl').splitlines():
        row=json.loads(line)
        if row['type']=='field':
            frame=int(row['field']);assert frame not in result and row['knot']==.3
            F.node_map(row);result[frame]=row
    required=json.loads((E/'protocol.json').read_text())['input_pins'][group]['frames']
    assert set(required)<=set(result),'Missing all-point field'
    return {f:result[f] for f in required}
def rounded_nodes(points):
    nodes={}
    scaled=points[:,:3]/.3;absolute=np.abs(scaled);whole=np.floor(absolute)
    # Addition of .5 can itself round a value below the half up to an integer.
    # Compare fractional parts to implement C++ lround ties-away exactly.
    keys=(np.sign(scaled)*(whole+((absolute-whole)>=.5))).astype(int)
    for key,point in zip(keys,points):
        k=tuple(key);total,count=nodes.get(k,(0.,0))
        nodes[k]=(total+float(np.float32(point[5])),count+1)
    return nodes
def rebuild(group):
    previous=fields(group)
    meta=json.loads((E/'groups'/group/'input_sources.json').read_text())
    cache=Path(meta['feature_cache_private']);assert sha(cache)==meta['feature_cache_sha256']
    tokens=iter(cache.read_text().split());needed={f-1 for f in previous};clouds={}
    while True:
        frame=next(tokens,None)
        if frame is None:break
        frame=int(frame)
        for cls in range(5):
            count=int(next(tokens))
            if cls==4 and frame in needed:
                data=np.fromiter((float(next(tokens)) for _ in range(6*count)),float,count=6*count).reshape(-1,6)
                assert frame not in clouds;clouds[frame]=(count,rounded_nodes(data))
            else:
                for _ in range(6*count):next(tokens)
    assert set(clouds)==needed
    cases=[]
    for frame,field in previous.items():
        count,nodes=clouds[frame-1];recorded={tuple(r[:3]):(r[3],r[4]) for r in field['nodes']}
        assert count==field['features'] and set(nodes)==set(recorded)
        maximum=0.
        for key,(total,population) in nodes.items():
            other,n=recorded[key];assert population==n
            assert np.isclose(total,other,rtol=1e-10,atol=1e-12)
            maximum=max(maximum,abs(total-other))
        cases.append(dict(frame=frame,input_features=count,nodes=len(nodes),all_population_counts_exact=True,max_abs_intensity_sum_difference=maximum))
    assert sha(cache)==meta['feature_cache_sha256']
    return dict(group=group,feature_cache_sha256=meta['feature_cache_sha256'],cases=cases)
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    result=dict(exit_code=0,groups={g:rebuild(g) for g in ('old','01','02','08')},
        scope='Independent all-point cached-input lattice rebuild against previously committed original native grid exports; no native estimator rerun.')
    a.output.write_text(json.dumps(result,indent=2)+'\n')
    print('Every original all-point field construction independently verified.')
