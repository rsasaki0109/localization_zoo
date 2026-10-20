"""Terminal matched S2M controls; actual accuracy uses the immutable real runner."""
from pathlib import Path
import hashlib,importlib.util,json,os,shlex,subprocess,tempfile,time
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent;PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])
def module(n,p):
 s=importlib.util.spec_from_file_location(n,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
b=module('build',E/'build.py');a=module('archive',W/'experiments/results/inten_natural_traces/run_sequences.py');restore=module('restore',W/'experiments/results/inten_natural_traces/diagnose.py').restore
read=lambda p:json.loads(Path(p).read_text())
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def git(*args):return subprocess.check_output(['git',*args],cwd=W,text=True).strip()
def run(label,argv,folder,payload=None,binary=None):
 folder.mkdir(parents=True,exist_ok=True);assert not (folder/(label+'_command.json')).exists();started=time.time()
 with (folder/(label+'.stdout')).open('xb') as out,(folder/(label+'.stderr')).open('xb') as err:r=subprocess.run(argv,input=payload,cwd=W,env=ENV,stdout=out,stderr=err)
 d=dict(command=argv,cwd=str(W),source_commit=git('rev-parse','HEAD'),exit_code=r.returncode,started_unix=started,finished_unix=time.time(),environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')})
 if payload is not None:d['stdin_sha256']=hashlib.sha256(payload).hexdigest()
 if binary:d.update(binary_sha256=b.sha(binary),diagnostic_source_commit=git('rev-parse','HEAD'),base_native_source_commit='9a01528b05128794df962b52ace59e727e834417')
 write(folder/(label+'_command.json'),d);print(label,r.returncode,flush=True);assert r.returncode==0,label+' failed: retain evidence';return r.returncode

def main():
 assert read(R/'boundary_complete.json')['exit_code']==0
 for name in ('spread.py','spread_probe.cpp','diagnostic_native.cpp','spread_protocol.json'):
  p=E/name;assert subprocess.check_output(['git','show','HEAD:'+p.relative_to(W).as_posix()],cwd=W)==p.read_bytes()
 original=(W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text();expected='// Diagnostic-only producer; see spread_protocol.json.\ndouble diagnostic_map_plane_spread = 0.0;\n'+original.replace('params_.s2m_plane_min_spread','::diagnostic_map_plane_spread');assert expected==(E/'diagnostic_native.cpp').read_text()
 stage=R/'spread_staging';stage.mkdir();old=b.OLD;meta=old/'evaluation/CMakeFiles/pcd_dogfooding.dir';link=shlex.split((meta/'link.txt').read_text());libs=[];pins={}
 for token in link[link.index('-o')+2:]:
  if token.startswith('-'):libs.append(token);continue
  p=Path(token);p=p if p.is_absolute() else (old/'evaluation'/p).resolve()
  if p==old/'papers/inten_loam/libinten_loam.a':continue
  if p.suffix=='.a':pins[str(p)]=b.sha(p)
  libs.append(str(p))
 binary=R/'spread_probe';assert not binary.exists()
 run('compile',b.docker()+['c++',*b.flags(old/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir'),'-I'+str(W/'evaluation/src'),str(E/'diagnostic_native.cpp'),str(E/'spread_probe.cpp'),*libs,'-o',str(binary)],stage)
 write(stage/'binary.json',dict(path=str(binary),sha256=b.sha(binary),source_commit=git('rev-parse','HEAD'),source_sha256={name:b.sha(E/name) for name in ('diagnostic_native.cpp','spread_probe.cpp','native_loader.h')},original_source_sha256=b.sha(W/'papers/inten_loam/src/inten_loam_paper.cpp'),linked_static_libraries=pins,scope=read(E/'spread_protocol.json')['diagnostic_producer']))
 raw=R/'kitti/dataset/sequences/01/velodyne';inputs=read(E/'boundary/inputs.json')['files'];assert {p.name:dict(sha256=b.sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs
 frames=read(E/'spread_protocol.json')['frames'];payload=('\n'.join(map(str,frames))+'\n').encode();(stage/'input.txt').write_bytes(payload)
 run('probe',b.docker(stage)+[str(binary),str(raw)],stage,payload,binary)
 rows=[json.loads(l) for l in (stage/'probe.stdout').read_text().splitlines()];oldrows=[json.loads(l) for l in (E/'boundary/probe.stdout').read_text().splitlines()];assert [r['frame'] for r in rows]==frames
 results=[];maxima=dict(H=0.,g=0.,output=0.);plane_reductions=0
 with tempfile.TemporaryDirectory(dir=R) as temp:
  temp=Path(temp);restore(W/'experiments/results/inten_facade_pca/sequences/01',temp);summary=read(temp/'candidate/summary.json');truth=np.loadtxt(temp/'candidate'/summary['artifacts']['evaluated_gt_path']).reshape(-1,3,4);poses=np.tile(np.eye(4),(len(truth),1,1));poses[:,:3,:]=truth
  previous={}
  with (temp/'candidate/trace.jsonl').open() as stream:
   for line in stream:
    r=json.loads(line)
    if r.get('type')=='frame' and r['frame_index']+1 in frames:previous[r['frame_index']+1]=np.asarray(r['pose']).reshape(4,4)
  for row,oldrow in zip(rows,oldrows):
   i=row['frame'];assert row['reference']==oldrow['reference'],('normal trace drift',i);assert row['normal_prefix_exact']
   for key in ('diagnostic_output',):assert row[key]==oldrow[key],('native base output drift',i)
   for now,before in zip(row['first_systems'],oldrow['first_systems']):
    assert now['n']==before['n']
    for k in ('H','g'):
     d=float(np.max(np.abs(np.asarray(now[k])-np.asarray(before[k]))));maxima[k]=max(maxima[k],d);assert d==0.,('native base system drift',i,k)
   for c,(base,candidate) in enumerate(zip(row['first_systems'],row['candidate_first_systems'])):
    if c in (0,3):assert base==candidate,('non-plane initial data changed',i,c)
    else:
     assert candidate['n']<=base['n'];plane_reductions+=base['n']-candidate['n']
   gt=np.linalg.inv(poses[i-1])@poses[i];length=float(np.linalg.norm(gt[:3,3]));prev=previous[i]
   def ratio(values):return float(np.linalg.norm((np.linalg.inv(prev)@np.asarray(values).reshape(4,4))[:3,3])/length)
   x,y=ratio(row['diagnostic_output']),ratio(row['candidate_output'])
   results.append(dict(frame=i,input_length_ratio=ratio(row['reference']['s2m']['input']),baseline_length_ratio=x,candidate_length_ratio=y,absolute_length_error_change=abs(y-1)-abs(x-1),baseline_class_pairs=[c['n'] for c in row['first_systems']],candidate_class_pairs=[c['n'] for c in row['candidate_first_systems']]))
 assert plane_reductions>0,'Intervention did not exercise plane acceptance'
 write(stage/'report.json',dict(exit_code=0,base_matches_immutable_native_controls_exact=True,max_absolute_differences=maxima,all_non_plane_initial_class_equations_exact=True,plane_support_removed=plane_reductions,lower_absolute_length_error=sum(r['absolute_length_error_change']<0 for r in results),higher_absolute_length_error=sum(r['absolute_length_error_change']>0 for r in results),equal_absolute_length_error=sum(r['absolute_length_error_change']==0 for r in results),cases=results,scope=read(E/'spread_protocol.json')['scope']))
 a.archive(stage,E/'spread');subprocess.run(['git','add','-f',str(E/'spread')],cwd=W,check=True);subprocess.run(['git','commit','-m','Record matched actual-map S2M spread controls including adverse endpoints'],cwd=W,check=True)
 run('push',['git','push','-u','origin','HEAD:refs/heads/codex/inten-map-plane-spread'],R/'spread_backup');write(R/'spread_complete.json',dict(exit_code=0,commit=git('rev-parse','HEAD')))

if __name__=='__main__':main()
