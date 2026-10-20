"""Frozen PR159 normal-prefix S2M controls; stop and retain every failed command."""
from pathlib import Path
import hashlib,importlib.util,json,os,shlex,subprocess,tempfile,time
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent;PY='/workspace/localization-env/host-venv/bin/python'
P=json.loads((E/'boundary_protocol.json').read_text());ENV=dict(os.environ,OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])
def module(n,p):
 s=importlib.util.spec_from_file_location(n,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
build=module('boundary_build',E/'build.py');archive=module('boundary_archive',W/'experiments/results/inten_natural_traces/run_sequences.py');restore=module('boundary_restore',W/'experiments/results/inten_natural_traces/diagnose.py').restore
read=lambda p:json.loads(Path(p).read_text())
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def git(*a):return subprocess.check_output(['git',*a],cwd=W,text=True).strip()
def run(label,argv,folder,payload=None,binary=None):
 folder.mkdir(parents=True,exist_ok=True);assert not (folder/(label+'_command.json')).exists();start=time.time()
 with (folder/(label+'.stdout')).open('xb') as out,(folder/(label+'.stderr')).open('xb') as err:
  x=subprocess.run(argv,input=payload,cwd=W,env=ENV,stdout=out,stderr=err)
 d=dict(command=argv,cwd=str(W),source_commit=git('rev-parse','HEAD'),exit_code=x.returncode,started_unix=start,finished_unix=time.time(),environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')})
 if payload is not None:d['stdin_sha256']=hashlib.sha256(payload).hexdigest()
 if binary:d.update(binary_sha256=build.sha(binary),native_source_commit=P['parent_binary']['source_commit'])
 write(folder/(label+'_command.json'),d);print(label,x.returncode,flush=True);return x.returncode

def main():
 stage=R/'boundary_staging';stage.mkdir()
 old=build.OLD;metadata=old/'evaluation/CMakeFiles/pcd_dogfooding.dir';link=shlex.split((metadata/'link.txt').read_text());libs=[];pins={}
 for token in link[link.index('-o')+2:]:
  if token.startswith('-'):libs.append(token);continue
  p=Path(token);p=p if p.is_absolute() else (old/'evaluation'/p).resolve()
  if p==old/'papers/inten_loam/libinten_loam.a':p=Path('/workspace/localization-env/inten-facade-pca/build/papers/inten_loam/libinten_loam.a')
  if p.suffix=='.a':pins[str(p)]=build.sha(p)
  libs.append(str(p))
 binary=R/'boundary_probe';assert not binary.exists()
 # This stage is committed before adding a native parameter: headers match frozen PR159 ABI.
 command=build.docker()+['c++',*build.flags(old/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir'),'-I'+str(W/'evaluation/src'),str(E/'boundary_probe.cpp'),*libs,'-o',str(binary)]
 assert run('compile',command,stage)==0
 write(stage/'binary.json',dict(path=str(binary),sha256=build.sha(binary),native_source_commit=P['parent_binary']['source_commit'],linked_static_libraries=pins))
 for attempt in range(1,5):
  if run('fetch',[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),'--sequence','01','--workers','4'],R/'boundary_fetch'/str(attempt))==0:break
 else:raise RuntimeError('Official acquisition failed; preserve all partial inputs/receipts')
 raw=R/'kitti/dataset/sequences/01/velodyne';expected=read(W/'experiments/results/shared_near_range/sequences/01/inputs.json')
 actual={p.name:dict(sha256=build.sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()};assert actual==expected['files']
 write(stage/'inputs.json',dict(sequence='01',files=actual,matches_previous_official_inputs=True,owned_input_dir=str(raw)))
 payload=('\n'.join(map(str,P['frames']))+'\n').encode();(stage/'input.txt').write_bytes(payload)
 assert run('probe',build.docker(stage)+[str(binary),str(raw)],stage,payload,binary)==0
 with tempfile.TemporaryDirectory(dir=R) as temp:
  temp=Path(temp);restore(W/'experiments/results/inten_facade_pca/sequences/01',temp)
  summary=read(temp/'candidate/summary.json');truthfile=temp/'candidate'/summary['artifacts']['evaluated_gt_path']
  truth=np.loadtxt(truthfile).reshape(-1,3,4);poses=np.tile(np.eye(4),(len(truth),1,1));poses[:,:3,:]=truth
  frames={}
  with (temp/'candidate/trace.jsonl').open() as stream:
   next(stream)
   for line in stream:
    f=json.loads(line)
    if f['type']=='frame' and f['frame_index'] in P['frames'] or f['type']=='frame' and f['frame_index']+1 in P['frames']:frames[f['frame_index']]=f
  rows=[json.loads(x) for x in (stage/'probe.stdout').read_text().splitlines()];assert [r['frame'] for r in rows]==P['frames']
  report=[];maxima=dict(H=0.,g=0.,output=0.,continuous=0.)
  for row in rows:
   i=row['frame'];f=frames[i];ref=row['reference']
   for k in ('s2s','s2m'):
    assert ref[k]==f[k],('continuous solver trace changed',i,k)
   assert ref['pose']==f['pose'],('continuous final pose changed',i)
   cs=row['first_systems'];H=sum(np.asarray(c['H']).reshape(6,6) for c in cs);g=sum(np.asarray(c['g']) for c in cs)
   first=ref['s2m']['iterations'][0];assert [c['n'] for c in cs]==first['class_pairs']
   for k,a,b in [('H',H,np.asarray(first['H']).reshape(6,6)),('g',g,np.asarray(first['g'])),('output',np.asarray(row['diagnostic_output']),np.asarray(ref['s2m']['output']))]:
    maxima[k]=max(maxima[k],float(np.max(np.abs(a-b))));assert np.allclose(a,b,rtol=1e-9,atol=1e-7),(i,k)
   T=np.asarray(ref['s2m']['input']).reshape(4,4);t=T[:3,3];K=np.array([[0,-t[2],t[1]],[t[2],0,-t[0]],[-t[1],t[0],0]]);B=np.eye(6);B[:3,3:]=K;J=np.c_[np.eye(3),-K];centered=B.T@H@B
   S=centered[:3,:3]-centered[:3,3:]@np.linalg.solve(centered[3:,3:]+1e-6*np.eye(3),centered[3:,:3]);values,U=np.linalg.eigh((S+S.T)/2);info=(U*np.maximum(0,.03*max(0,values.max())-values))@U.T
   gt=np.linalg.inv(poses[i-1])@poses[i];length=np.linalg.norm(gt[:3,3]);prev=np.asarray(frames[i-1]['pose']).reshape(4,4);direction=prev[:3,:3]@gt[:3,3]/length;A=H+1e-6*np.eye(6)
   contributions=[float(direction@J@np.linalg.solve(A,-np.asarray(c['g']))/length*100) for c in cs]
   def ratio(T):return float(np.linalg.norm((np.linalg.inv(prev)@T)[:3,3])/length)
   report.append(dict(frame=i,classes=['edge','ground','facade','intensity'],class_pairs=[c['n'] for c in cs],class_first_forward_percent=contributions,first_forward_percent=sum(contributions),data_translation_eigenvalues=values.tolist(),data_min_over_max=float(values.min()/values.max()),hypothetical_ratio03_prior_norm=float(np.linalg.norm(info)),hypothetical_ratio03_prior_forward_information=float(direction@info@direction),input_length_ratio=ratio(T),output_length_ratio=ratio(np.asarray(ref['s2m']['output']).reshape(4,4)),s2s_input_length_ratio=float(np.linalg.norm(np.asarray(ref['s2s']['input']).reshape(4,4)[:3,3])/length),s2s_output_length_ratio=float(np.linalg.norm(np.asarray(ref['s2s']['output']).reshape(4,4)[:3,3])/length)))
  write(stage/'report.json',dict(exit_code=0,scope=P['scope'],trace_max_absolute_differences=maxima,cases=report))
 archive.archive(stage,E/'boundary');archive.archive(R/'boundary_fetch',E/'boundary_fetch')
 subprocess.run(['git','add','-f',str(E/'boundary'),str(E/'boundary_fetch')],cwd=W,check=True)
 subprocess.run(['git','commit','-m','Record current KITTI01 normal-prefix S2M onset controls'],cwd=W,check=True)
 assert run('push',['git','push','-u','origin','HEAD:refs/heads/codex/inten-s2m-prior'],R/'boundary_backup')==0
 write(R/'boundary_complete.json',dict(exit_code=0,commit=git('rev-parse','HEAD'),raw_retained_for_first_full_comparison=True))
if __name__=='__main__':main()
