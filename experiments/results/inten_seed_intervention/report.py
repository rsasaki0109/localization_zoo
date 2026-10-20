"""Offline report for the fixed-pair seed intervention, not an estimator change."""
from pathlib import Path
from collections import Counter
import argparse,csv,hashlib,importlib.util,json,subprocess,sys,tempfile
import numpy as np

E=Path(__file__).resolve().parent
W=E.parents[2]
spec=importlib.util.spec_from_file_location('seed_observation',W/'experiments/results/inten_natural_traces/diagnose.py')
natural=importlib.util.module_from_spec(spec);spec.loader.exec_module(natural)


def read(p):return json.loads(Path(p).read_text())
def write(p,d):Path(p).write_text(json.dumps(d,indent=2,allow_nan=False)+'\n')
def require(ok,msg):
 if not ok:raise ValueError(msg)
def matrix(v):return np.asarray(v,dtype=float).reshape(4,4)
def stats(v):return natural.distribution(v)
def rotation(R):
 u,_,v=np.linalg.svd(R)
 return u@np.diag([1.,1.,float(np.linalg.det(u@v))])@v

def motion(T,gt,moving):
 if any(isinstance(v,str) for v in T):return dict(status='nonfinite',length_ratio=None,vector_error=None,rotation_error_rad=None,bad_length=None,bad_vector=None)
 T=matrix(T);step=float(np.linalg.norm(gt[:3,3]));require(np.isfinite(T).all(),'untagged nonfinite pose')
 ratio=float(np.linalg.norm(T[:3,3])/step) if moving else None
 vector=float(np.linalg.norm(T[:3,3]-gt[:3,3])/step) if moving else None
 Q=rotation(T[:3,:3]).T@rotation(gt[:3,:3])
 # atan2 remains zero for equal rounded matrices, unlike acos(trace(R.T*R)).
 angle=float(np.arctan2(np.linalg.norm([Q[2,1]-Q[1,2],Q[0,2]-Q[2,0],Q[1,0]-Q[0,1]])/2,
                        np.clip((np.trace(Q)-1)/2,-1,1)))
 return dict(status='finite',length_ratio=ratio,vector_error=vector,rotation_error_rad=angle,
             bad_length=abs(ratio-1)>.2 if moving else None,bad_vector=vector>.2 if moving else None)


def first_step(result,gt,moving):
 classes=result['first_systems'];values=[x for c in classes for k in ('H','g') for x in c[k]]
 if any(isinstance(x,str) for x in values):return dict(status='nonfinite_system',contributions=None)
 H=sum(np.asarray(c['H']).reshape(6,6) for c in classes)
 g=sum(np.asarray(c['g']) for c in classes)
 try:
  joint=np.linalg.solve(H+1e-6*np.eye(6),-g)
  parts=[np.linalg.solve(H+1e-6*np.eye(6),-np.asarray(c['g'])) for c in classes]
 except np.linalg.LinAlgError:return dict(status='singular_solve',contributions=None)
 if not np.isfinite(joint).all() or not np.isfinite(parts).all():return dict(status='nonfinite_solve',contributions=None)
 require(np.allclose(sum(parts),joint,atol=1e-8,rtol=1e-10),'nonadditive decomposition')
 t=matrix(result['seed'])[:3,3];step=float(np.linalg.norm(gt[:3,3]));direction=gt[:3,3]/step if moving else None
 def effect(delta):
  # Native left tangent at the current transformed sensor centre, first order.
  displacement=delta[:3]+np.cross(delta[3:],t)
  return dict(native_delta=delta.tolist(),sensor_origin_displacement_m=displacement.tolist(),
              along_gt_fraction=float(displacement@direction/step) if moving else None)
 return dict(status='finite',joint=effect(joint),contributions={name:dict(first_pairs=c['n'],**effect(d))
             for name,c,d in zip(('edge','ground','facade','intensity'),classes,parts)})


def sequence(seq,output):
 folder=E/'sequences'/seq;rows=[];details=[]
 with tempfile.TemporaryDirectory() as tmp:
  tmp=Path(tmp);archive=natural.restore(folder,tmp);cases=read(tmp/'cases.json')['cases'];previous=read(tmp/'summary.json')
  native=[json.loads(line) for line in (tmp/'native.stdout').read_text().splitlines()]
  with (W/'experiments/results/inten_natural_traces/diagnosis'/(seq+'_frames.csv')).open() as stream:
   full_steps={int(r['frame']):r for r in csv.DictReader(stream)}
  bad_s2s={i for i,r in full_steps.items() if r['s2s_bad_length']=='True'}
  entries={i for i in bad_s2s if i-1 not in bad_s2s}
  recoveries={i+1 for i in bad_s2s if i+1 not in bad_s2s and i+1 in full_steps}
  modes=('natural','previous_mapped','oracle_current_gt')
  require(len(native)==len(cases)*len(modes),'incomplete tuple')
  for i,case in enumerate(cases):
   truth=matrix(case['gt_relative']);moving=float(np.linalg.norm(truth[:3,3]))>=.1
   native_mode={}
   for j,mode in enumerate(modes):
    value=native[i*3+j];require(value['id']==case['id'] and value['mode']==mode,'case mismatch')
    seed=motion(value['seed'],truth,moving);out=motion(value['output'],truth,moving)
    if mode=='oracle_current_gt':require(seed['rotation_error_rad']<1e-12,'equal seed rotation metric is not zero')
    contributions=first_step(value,truth,moving)
    row=dict(id=case['id'],frame=case['frame'],mode=mode,moving=moving,
             s2s_bad_entry=case['frame'] in entries,s2s_recovery=case['frame'] in recoveries,
             natural_bad_length=case['labels']['s2s_bad_length']=='True',final_bad_length=case['labels']['bad_length']=='True',
             **{'seed_'+k:v for k,v in seed.items()},**{'output_'+k:v for k,v in out.items()},
             first_step_status=contributions['status'],
             first_step_along_gt_fraction=contributions.get('joint',{}).get('along_gt_fraction'))
    for name in ('edge','ground','facade','intensity'):
     row['first_'+name+'_along_gt_fraction']=(contributions['contributions'][name]['along_gt_fraction']
                                              if contributions.get('contributions') else None)
    rows.append(row);details.append(dict(id=case['id'],mode=mode,first_step=contributions))
    native_mode[mode]=row
   natural_row=native_mode['natural']
   require(natural_row['output_bad_length']==(case['labels']['s2s_bad_length']=='True') if moving else True,'natural bad label changed')
  groups={}
  for name in ('all_selected_moving','natural_bad_length','other_selected_moving','s2s_bad_entry','s2s_bad_interior'):
   groups[name]={}
   for mode in modes:
    part=[r for r in rows if r['mode']==mode and r['moving'] and
          (name=='all_selected_moving' or (name=='natural_bad_length' and r['natural_bad_length']) or
           (name=='other_selected_moving' and not r['natural_bad_length']) or
           (name=='s2s_bad_entry' and r['s2s_bad_entry']) or
           (name=='s2s_bad_interior' and r['natural_bad_length'] and not r['s2s_bad_entry']))]
    old=previous['groups'].get(name,{}).get(mode,dict(cases=len(part),bad_length_outputs=sum(r['output_bad_length'] is True for r in part)))
    require(len(part)==old['cases'] and sum(r['output_bad_length'] is True for r in part)==old['bad_length_outputs'],'length results changed')
    groups[name][mode]=dict(cases=len(part),bad_length_outputs=old['bad_length_outputs'],
        bad_vector_outputs=sum(r['output_bad_vector'] is True for r in part),
        good_seed_to_bad_output=sum(r['seed_bad_length'] is False and r['output_bad_length'] is True for r in part),
        bad_seed_to_good_output=sum(r['seed_bad_length'] is True and r['output_bad_length'] is False for r in part),
        first_step_statuses=dict(Counter(r['first_step_status'] for r in part)),
        statistics={k:stats(r[k] for r in part) for k in ('seed_length_ratio','output_length_ratio','output_vector_error',
                   'output_rotation_error_rad','first_step_along_gt_fraction',
                   'first_edge_along_gt_fraction','first_ground_along_gt_fraction','first_facade_along_gt_fraction','first_intensity_along_gt_fraction')})
  oracle_failures=[r for r in rows if r['mode']=='oracle_current_gt' and (r['output_bad_length'] or r['output_bad_vector'])]
  endpoints=[r for r in rows if any(abs(r['frame']-i)<=1 for i in entries|recoveries)]
  final_endpoints=[]
  for start,end in read(W/'experiments/results/inten_natural_traces/sequences'/seq/'motion_steps.json')['bad_length_step_frame_ranges']:
   selected_frames={start-1,start,start+1,end-1,end,end+1}
   final_endpoints.extend(r for r in rows if r['frame'] in selected_frames)
  with (output/(seq+'_cases.csv')).open('w') as stream:
   writer=csv.DictWriter(stream,fieldnames=list(rows[0]),lineterminator='\n');writer.writeheader();writer.writerows(rows)
  write(output/(seq+'_first_steps.json'),details)
  write(output/(seq+'_boundaries.json'),dict(s2s_boundaries=endpoints,final_boundaries=final_endpoints))
  return dict(sequence=seq,case_count=len(cases),native_evaluations=len(native),
              original_natural_replay_control=previous['natural_replay_max_absolute_differences'],
              source_archive_sha256=natural.sha(folder/'archive.json'),archive_files_verified=len(archive['files']),
              groups=groups,oracle_failure_cases=oracle_failures)


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output-dir',type=Path,required=True);a=p.parse_args()
 plan=read(E/'report_protocol.json')
 for name in ('report.py','report_protocol.json'):
  path=E/name;require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)==path.read_bytes(),'report source not committed')
 subprocess.run(['git','diff','--exit-code',plan['raw_result_commit'],'--',str(E/'sequences')],cwd=W,check=True)
 a.output_dir.mkdir(parents=True,exist_ok=False)
 results=[sequence(s,a.output_dir) for s in ('01','02','10')]
 write(a.output_dir/'summary.json',dict(schema_version=1,exit_code=0,source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),
     command=[sys.executable,*sys.argv],cwd=str(Path.cwd()),implementation_sha256=natural.sha(__file__),
     protocol_sha256=natural.sha(E/'report_protocol.json'),sequences=results,
     limits=plan['limits'],rotation_metric=plan['rotation_metric'],class_decomposition=plan['class_decomposition']))


if __name__=='__main__':main()
