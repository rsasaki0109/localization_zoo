"""Render every case/history from committed verified fixed-pose measurements."""
from pathlib import Path
import hashlib,importlib.util,json,lzma
import numpy as np
E=Path(__file__).resolve().parent
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
io=module('support_case_report_io',E/'report_io.py')
read=lambda p:json.loads(p.read_text())
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def number(x):return 'undefined' if x is None else format(float(x),'.9g')
def fraction(a,b):return a/b if b!=0 else None
def native_anchors(group):
    parent=E.parent/'inten_prior_control/groups'/group
    path=parent/read(parent/'archive.json')['files']['native.stdout.jsonl']['path']
    inputs={}
    with lzma.open(path,'rt') as stream:
        for line in stream:
            if '"type":"terminal"' not in line[:80]:continue
            r=json.loads(line);m=r['mode'];f=r['frame']
            if m not in (6,7,14,15):continue
            inputs[f,m,False]=dict(called=r['trace']['s2s']['called'],seed=r['trace']['s2s']['input'])
            if m%2:
                solver=r['trace']['s2s_consensus']['refinement']
                inputs[f,m,True]=dict(called=solver['called'],seed=solver['input'])
    anchors={};count=0
    folder=E/'groups'/group
    path=folder/read(folder/'archive.json')['files']['support.extra.jsonl']['path']
    with lzma.open(path,'rt') as stream:
        for line in stream:
            wrapped=json.loads(line);raw=wrapped['raw_line']
            if '"type":"support_query"' not in raw[:80]:continue
            r=json.loads(raw);count+=1;key=(r['frame'],r['mode'],r['translation_only'],r['class'])
            r={k:v for k,v in r.items() if k!='rows'}
            if key not in anchors:anchors[key]=[r,r]
            else:anchors[key][1]=r
    return inputs,anchors,count
def main():
    verified=io.read_report();assert verified['exit_code']==0
    prior_path=E.parent/'inten_prior_control/report/summary.json';prior=read(prior_path)
    primary=[];histories=[];tables=[];total=0
    for group,report in verified['groups'].items():
        inputs,anchors,count=native_anchors(group);total+=count
        assert count==report['passive_class_queries']
        old={r['frame']:r for r in prior['groups'][group]['cases']}
        population=read(E/'protocol.json')['input_pins'][group]['frames']
        for f in population:
            for m in (6,7,14,15):
                for translation in ((False,True) if m%2 else (False,)):
                    steps=sorted((s for s in report['steps'] if s['frame']==f and s['mode']==m and s['translation_only']==translation),key=lambda s:s['iteration'])
                    phase='refinement' if translation else 'joint';label=f'{group}/{f} · arm{m} · {phase}'
                    seed=np.array(inputs[f,m,translation]['seed']).reshape(4,4)[:3,3]
                    length=np.linalg.norm(seed);axis=seed/length if length>0 else None
                    entry=dict(group=group,frame=f,mode=m,translation_only=translation,iterations=len(steps),seed_translation=seed.tolist(),axis=axis.tolist() if axis is not None else None)
                    if not steps:
                        tables.extend(['### '+label,'','Native refinement was not executed; no iteration is omitted.',''])
                        histories.append(entry);continue
                    first,last=steps[0],steps[-1]
                    def information(s):
                        v=s['information'];a=v['all']['longitudinal_information'];k=v['kept']['longitudinal_information']
                        return dict(iteration=s['iteration'],all=a,kept=k,retained_fraction=fraction(k,a) if a is not None else None,
                            all_rhs=v['all']['longitudinal_rhs'],kept_rhs=v['kept']['longitudinal_rhs'])
                    entry.update(first=information(first),last=information(last))
                    ratios=[information(s)['retained_fraction'] for s in steps];ratios=[r for r in ratios if r is not None]
                    entry['minimum_retained_fraction']=min(ratios) if ratios else None
                    entry['maximum_retained_fraction']=max(ratios) if ratios else None
                    tables.extend(['### '+label,'',f'Iterations: {len(steps)}. Estimated seed translation: `{seed.tolist()}`.',
                        f"Centered longitudinal information all/kept: first {number(entry['first']['all'])}/{number(entry['first']['kept'])}; last {number(entry['last']['all'])}/{number(entry['last']['kept'])}. Retained fraction range over every iteration: {number(entry['minimum_retained_fraction'])}–{number(entry['maximum_retained_fraction'])}.",'',
                        '| Iteration | Class | Valid all | Kept | Removed | All cost | Removed cost | Removed axis Htt | All axis gt | Kept axis gt |',
                        '|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|'])
                    for which,index in ((0,first['iteration']),(1,last['iteration'])):
                        if which==1 and first['iteration']==last['iteration']:continue
                        for c,name in enumerate(('edge','ground','facade','intensity')):
                            r=anchors[f,m,translation,c][which];assert r['iteration']==index
                            projected={k:float(axis@np.array(r[k]['H']).reshape(6,6)[:3,:3]@axis) if axis is not None else None for k in ('all','kept','removed')}
                            gradient={k:float(axis@np.array(r[k]['g'])[:3]) if axis is not None else None for k in ('all','kept')}
                            values=[index,name,r['all']['count'],r['kept']['count'],r['removed']['count'],number(r['all']['cost']),number(r['removed']['cost']),number(projected['removed']),number(gradient['all']),number(gradient['kept'])]
                            tables.append('| '+' | '.join(map(str,values))+' |')
                    tables.append('');histories.append(entry)
                    if m==14 and not translation:
                        geometry=[anchors[f,m,False,c][0] for c in range(3)]
                        metrics=old[f]['metrics']['14']
                        primary.append(dict(group=group,frame=f,length_ratio=metrics['length_ratio'],
                            valid_geometry=sum(r['all']['count'] for r in geometry),removed_geometry=sum(r['removed']['count'] for r in geometry),
                            first_info_fraction=entry['first']['retained_fraction'],last_info_fraction=entry['last']['retained_fraction'],
                            minimum_info_fraction=entry['minimum_retained_fraction'],maximum_info_fraction=entry['maximum_retained_fraction']))
    assert total==verified['passive_class_queries'] and len(primary)==31
    text=['# Fixed-pose native support information: every case','',
        'All inherited cases and both joint/refinement histories of every predeclared passive arm remain. Counts below refer to valid native residuals, not every early reciprocal rejection. Every intermediate query, equation and information spectrum remains in the complete raw archives and lossless verification report. No passive value changes any estimate.','',
        'The axis is each native solver input estimated translation, expressed in target coordinates. Zero input translation gives undefined axis projections. Schur information uses the inherited centered transform and rotation-block regularization1e-6. Class Htt/gt projections hold rotation fixed; they are raw additive contributions, not additive Schur terms. Heterogeneous native intensity/geometric objective units do not define a physical uncertainty calibration. Costs alone are not motion-accuracy witnesses.','',
        'The primary overview uses the already predeclared arm14; length/GT is the unchanged prior committed conditional terminal result, included only as context. It is not a new trajectory score. `old` and `01` denote separate inherited KITTI01 histories.','',
        '| Case | Unchanged length/GT | Initial valid geometry | Initial removed | Initial kept/all Schur | Last kept/all Schur | Minimum | Maximum |',
        '|---|---:|---:|---:|---:|---:|---:|---:|']
    for r in primary:
        text.append('| '+f"{r['group']}/{r['frame']}"+' | '+' | '.join(number(r[k]) for k in ('length_ratio','valid_geometry','removed_geometry','first_info_fraction','last_info_fraction','minimum_info_fraction','maximum_info_fraction'))+' |')
    text.extend(['','## Every history and every feature class','',*tables])
    (E/'cases.md').write_text('\n'.join(text)+'\n')
    result=dict(exit_code=0,verified_report_manifest_sha256=sha(E/'report/manifest.json'),
        unchanged_parent_metrics_sha256=sha(prior_path),primary=primary,histories=histories,passive_class_queries=total)
    (E/'cases.summary.json').write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print('All cases, all passive histories and native class contributions regenerated.')
if __name__=='__main__':main()
