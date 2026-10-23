"""Localize every inherited case and audit all passive geometric associations."""
from pathlib import Path
import csv, hashlib, importlib.util, json, lzma
import numpy as np
E=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('local_rigidity_common',E/'common.py')
common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
W,P,read,write,sha,module=(getattr(common,k) for k in ('W','P','read','write','sha','module'))
rig=module('local_rigidity_equations',E/'rigidity.py')

mixed=module('rigidity_frozen_arithmetic',P/'verify_mixed.py')
io=module('rigidity_parent_report',P/'report_io.py')

def bindings():
    protocol=read(E/'protocol.json')
    for name,digest in protocol['source_sha256'].items():
        assert sha(W/name)==digest,('Frozen source/evidence changed',name)
    return protocol

def stream(folder,name):
    meta=read(folder/'archive.json')['files'][name];path=folder/meta['path']
    assert meta['storage']=='xz' and sha(path)==meta['stored_sha256']
    digest=hashlib.sha256();size=0
    with lzma.open(path,'rb') as incoming:
        for line in incoming:
            digest.update(line);size+=len(line)
            yield json.loads(line)
    assert size==meta['bytes'] and digest.hexdigest()==meta['sha256']

def geometry(group):
    clouds={}
    for wrapped in stream(P/'groups'/group,'support.extra.jsonl'):
        raw=wrapped['raw_line']
        if '"type":"support_features"' not in raw[:80]:continue
        row=json.loads(raw);assert row['frame'] not in clouds
        clouds[row['frame']]=[np.array(c,float).reshape(-1,6) for c in row['classes']]
    return clouds

def phases(group):
    result={}
    for row in stream(P.parent/'inten_prior_control/groups'/group,'native.stdout.jsonl'):
        if row['type']!='terminal' or row['mode'] not in (6,7,14,15):continue
        trace=row['trace'];f=row['frame'];m=row['mode']
        for translation,solver in [(False,trace['s2s'])]+([(True,trace['s2s_consensus']['refinement'])] if m%2 else []):
            result[f,m,translation]=dict(seed=solver['input'],iterations=len(solver['iterations']),
                excluded=trace['s2s_consensus']['excluded_clusters'] if translation else [])
    return result

def point_terms(record,clouds,excluded,pca_cache,axis):
    captured=[];namespace=mixed.base.namespace;original=namespace['reduce_rows']
    def capture(x,a,r,valid):
        if not captured:captured.append((x.copy(),a.copy(),r.copy()))
        return original(x,a,r,valid)
    namespace['reduce_rows']=capture
    try:mixed.checker.reconstruct(record,clouds,set(excluded),pca_cache,{})
    finally:namespace['reduce_rows']=original
    assert len(captured)==1
    _,a,r=captured[0];w=np.ones(len(r));large=np.abs(r)>.1;w[large]=.1/np.abs(r[large])
    if axis is None:return np.full(len(r),np.nan),np.full(len(r),np.nan)
    projection=mixed.sum3(a*axis)
    h=(w*projection)*projection
    g=mixed.sum3(((w[:,None]*a)*r[:,None])*axis)
    valid=np.array([r[0] for r in record['rows']],bool);keep=np.array([r[1] for r in record['rows']],bool)
    for name,mask in (('all',valid),('kept',valid&keep),('removed',valid&~keep)):
        H=np.array(record[name]['H']).reshape(6,6)[:3,:3];gradient=np.array(record[name]['g'])[:3]
        mixed.checker.close(float(h[mask].sum()),float(axis@H@axis),'Point-axis H partition differs')
        mixed.checker.close(float(g[mask].sum()),float(axis@gradient),'Point-axis g partition differs')
    return g,h

def local_groups(source,valid,keep,g,h,defined_axis):
    groups=[]
    for cluster in sorted(set(map(int,source[valid,4]))):
        cluster_mask=source[:,4]==cluster
        for name,mask in (('all',valid&cluster_mask),('kept',valid&keep&cluster_mask),('removed',valid&~keep&cluster_mask)):
            points=source[mask,:3];gradient=g[mask];information=h[mask]
            groups.append(dict(cluster=cluster,partition=name,count=len(points),
                centroid=points.mean(axis=0).tolist() if len(points) else None,
                minimum=points.min(axis=0).tolist() if len(points) else None,
                maximum=points.max(axis=0).tolist() if len(points) else None,
                axis_g=float(gradient.sum()) if defined_axis else None,
                axis_H=float(information.sum()) if defined_axis else None))
    return groups

def group_result(group,parent,output):
    clouds=geometry(group);metadata=phases(group)
    expected={(s['frame'],s['mode'],s['translation_only'],s['iteration'],c):s for s in parent['steps'] for c in range(4)}
    near={(f,c):rig.neighbors(cloud[:,:3]) for f,classes in clouds.items() for c,cloud in enumerate(classes[:3])}
    accum={};boundaries=[];seen=set();pca_cache={};total=dict(class_observations=0,source_rows=0,valid_rows=0,removed_rows=0,possible_relations=0,eligible_relations=0)
    output.mkdir(parents=True,exist_ok=False)
    with (output/'points.csv').open('w',newline='') as csvfile:
        writer=csv.writer(csvfile,lineterminator='\n');writer.writerow(['frame','mode','translation_only','iteration','class','source_index','anchor_index','kept','axis_g','axis_H','eligible_neighbors','mean_absolute_length_error_m','maximum_absolute_length_error_m'])
        for wrapped in stream(P/'groups'/group,'support.extra.jsonl'):
            raw=wrapped['raw_line']
            if '"type":"support_query"' not in raw[:80]:continue
            record=json.loads(raw);f,m,tr,it,c=(record[k] for k in ('frame','mode','translation_only','iteration','class'))
            key=(f,m,tr,it,c);assert key in expected and key not in seen;seen.add(key)
            rows=record['rows'];valid=np.array([r[0] for r in rows],bool);keep=np.array([r[1] for r in rows],bool)
            counts=expected[key]['classes'][c]
            assert len(rows)==len(clouds[f][c]) and int(valid.sum())==counts['all']['count']
            assert int((valid&keep).sum())==counts['kept']['count'] and int((valid&~keep).sum())==counts['removed']['count']
            total['class_observations']+=1;total['source_rows']+=len(rows);total['valid_rows']+=int(valid.sum());total['removed_rows']+=int((valid&~keep).sum())
            if c==3:
                assert np.all(keep) and counts['removed']['count']==0
                continue
            source=clouds[f][c];target=clouds[f-1][c]
            anchors=np.array([r[2][0] if r[2] else -1 for r in rows],int)
            value=rig.measure(source[:,:3],target[:,:3],anchors,valid,near[f,c])
            total['possible_relations']+=value['possible_relations'];total['eligible_relations']+=len(value['error'])
            history=(f,m,tr,c);parts=accum.setdefault(history,{name:[] for name in ('all','kept','removed')})
            for name,mask in (('all',valid),('kept',valid&keep),('removed',valid&~keep)):
                parts[name].append(rig.summarize(value,mask))
            phase=metadata[f,m,tr];last=phase['iterations']-1
            if it not in (0,last):continue
            seed=np.array(phase['seed']).reshape(4,4)[:3,3];length=np.linalg.norm(seed);axis=seed/length if length else None
            g,h=point_terms(record,clouds,phase['excluded'],pca_cache,axis)
            for i in np.flatnonzero(valid):
                finite=lambda v:format(float(v),'.17g') if np.isfinite(v) else ''
                writer.writerow([f,m,int(tr),it,c,int(i),int(anchors[i]),int(keep[i]),finite(g[i]),finite(h[i]),int(value['count'][i]),finite(value['mean_absolute'][i]),finite(value['maximum_absolute'][i])])
            boundaries.append(dict(frame=f,mode=m,translation_only=tr,iteration=it,feature_class=c,
                axis=axis.tolist() if axis is not None else None,
                groups=local_groups(source,valid,keep,g,h,axis is not None)))
    assert seen==set(expected)
    histories=[]
    for (f,m,tr,c),parts in sorted(accum.items()):
        assert len(parts['all'])==metadata[f,m,tr]['iterations']
        for index in range(len(parts['all'])):
            a,k,r=(parts[name][index] for name in ('all','kept','removed'))
            for name in ('centers','centers_with_relations','relations','zero_source_length'):
                assert a[name]==k[name]+r[name]
            for name in ('source_length_sum','target_length_sum','signed_sum','absolute_sum','squared_sum'):
                mixed.checker.close(a[name],k[name]+r[name],'Witness center partition differs')
        histories.append(dict(frame=f,mode=m,translation_only=tr,feature_class=c,
            iterations=len(parts['all']),partitions={name:rig.finish(p) for name,p in parts.items()}))
    unexecuted=[dict(frame=f,mode=m,translation_only=tr) for (f,m,tr),v in sorted(metadata.items()) if not v['iterations']]
    return dict(group=group,counts=total,histories=histories,boundaries=boundaries,unexecuted=unexecuted,
        points=dict(bytes=(output/'points.csv').stat().st_size,sha256=sha(output/'points.csv')))

def analyze(output):
    protocol=bindings();parent=io.read_report();assert parent['exit_code']==0 and parent['all_558_historical_native_outcomes_exact']
    output.mkdir(parents=True,exist_ok=False);groups={}
    # Sequential groups avoid simultaneous copies of native archives or history arrays.
    for group in ('old','01','02','08'):
        groups[group]=group_result(group,parent['groups'][group],output/group)
        print(group+' complete',flush=True)
    assert sum(g['counts']['class_observations'] for g in groups.values())==parent['passive_class_queries']
    assert sum(g['counts']['removed_rows'] for g in groups.values())==parent['counts']['valid_removed']
    result=dict(exit_code=0,groups=groups,parent_report_manifest_sha256=sha(P/'report/manifest.json'),
        protocol_sha256=sha(E/'protocol.json'),settings=protocol['witness'],
        scope='Pose-free equation witness conditional on native anchor assignments. No static truth, selector, candidate, new solve or trajectory score.')
    write(output/'summary.json',result)
    print('All inherited cases and passive query populations retained; no motion candidate executed.')
    return result

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--output-dir',type=Path,required=True)
    analyze(parser.parse_args().output_dir)
