"""Execute the committed fixed-pair seed experiment; not a generic runner."""
from pathlib import Path
import csv
import gzip
import hashlib
import importlib.util
import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
import traceback

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]
R = Path('/workspace/localization-env/inten-collapse-boundaries')
P = json.loads((E/'protocol.json').read_text())
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', **P['environment'])
OLD = W/'experiments/results/inten_natural_traces'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


analysis = module('natural_analysis', OLD/'diagnose.py')
archive = module('natural_archival', OLD/'run_sequences.py').archive
sys.path.insert(0, str(W/'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_kitti_poses, rotation_angle


def read(path):
    return json.loads(Path(path).read_text())


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, indent=2, allow_nan=False)+'\n')


def git(*args):
    return subprocess.check_output(['git',*args],cwd=W,text=True).strip()


def require(ok, reason):
    if not ok:
        raise ValueError(reason)


def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS',
            '-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker',
            '--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000',
            '-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1',
            '-w',str(W),'localization-zoo-dev']


def run(label, command, folder, allowed=(0,), payload=None):
    folder.mkdir(parents=True,exist_ok=True)
    require(not (folder/(label+'_command.json')).exists(), 'receipt exists')
    started = time.time()
    with (folder/(label+'.stdout')).open('wb') as out, (folder/(label+'.stderr')).open('wb') as err:
        result = subprocess.run(command,cwd=W,env=ENV,input=payload,stdout=out,stderr=err)
    write(folder/(label+'_command.json'),dict(command=command,cwd=str(W),source_commit=git('rev-parse','HEAD'),
          started_unix=started,finished_unix=time.time(),exit_code=result.returncode,environment=P['environment'],
          stdin_sha256=hashlib.sha256(payload).hexdigest() if payload else None))
    print(label,result.returncode,flush=True)
    require(result.returncode in allowed,label+' failed')
    return result.returncode


def invariant():
    require(analysis.sha(P['library']['path'])==P['library']['sha256'],'library changed')
    for name in ('run.py','probe.cpp','native_loader.h','protocol.json'):
        path=E/name
        require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)==path.read_bytes(),
                'uncommitted source '+name)
    require(not git('diff','--name-only',P['parent_result_commit'],'--','common','evaluation',
                    ':(glob)papers/*/src/**',':(glob)papers/*/include/**',
                    'experiments/results/inten_natural_traces'),'immutable inputs/source changed')
    text=(W/P['loader']['source']).read_text()
    require(hashlib.sha256(text.encode()).hexdigest()==P['loader']['sha256'],'loader source changed')
    for fragment in P['loader']['fragments']:
        a=text.index(fragment['start']);b=text.index(fragment['end'],a)
        if fragment['start'].startswith('struct'):b+=len(fragment['end'])
        raw=text[a:b]
        require(hashlib.sha256(raw.encode()).hexdigest()==fragment['sha256'] and raw in (E/'native_loader.h').read_text(),
                'loader fragment changed')


def backup(label):
    for attempt in range(1,4):
        if run('push',['git','push','origin','HEAD:refs/heads/codex/inten-collapse-boundaries'],
               R/'backups'/label/str(attempt),allowed=(0,1,128))==0:
            return
    raise RuntimeError('Backup failed; stop before next fetch')


def prepare(seq, stage, raw):
    with (OLD/'diagnosis'/(seq+'_frames.csv')).open() as stream:
        steps={int(r['frame']):r for r in csv.DictReader(stream)}
    bad={i for i,r in steps.items() if r['bad_length']=='True' or r['s2s_bad_length']=='True'}
    selected=({j for i in bad for j in (i-1,i,i+1)} | {1} |
              {b['last_pose_frame'] for b in read(OLD/'sequences'/seq/'motion_steps.json')['blocks']}) & set(steps)
    cases=[]
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp=Path(temp)
        analysis.restore(OLD/'sequences'/seq,temp)
        summary=read(temp/'on/summary.json')
        gt=np.asarray(load_kitti_poses(temp/'on'/summary['artifacts']['evaluated_gt_path']))
        previous_previous=None;previous=None;accepted=None;flows=0
        with (temp/'on/trace.jsonl').open() as stream:
            header=json.loads(next(stream))
            for line in stream:
                frame=json.loads(line)
                if frame['type']!='frame':continue
                i=frame['frame_index'];pose=np.asarray(frame['pose']).reshape(4,4)
                if i:
                    require(frame['input_index']==i,'GT index mismatch')
                    require(np.array_equal(np.asarray(frame['s2s']['input']).reshape(4,4),accepted),'prediction is not last accepted S2S')
                    flows+=1
                if i in selected:
                    natural=np.asarray(frame['s2s']['input']).reshape(4,4)
                    mapped=np.linalg.inv(previous_previous)@previous if previous_previous is not None else np.eye(4)
                    oracle=np.linalg.inv(gt[i-1])@gt[i]
                    cases.append(dict(id=f'{seq}_{i}',frame=i,labels=steps[i],
                        seeds={k:T.ravel().tolist() for k,T in zip(P['modes'],(natural,mapped,oracle))},
                        gt_relative=oracle.ravel().tolist(),expected_output=frame['s2s']['output'],
                        expected_first=frame['s2s']['iterations'][0],expected_features=frame['features']))
                previous_previous=previous;previous=pose
                accepted=np.asarray(frame['accepted_relative']).reshape(4,4)
    write(stage/'cases.json',dict(sequence=seq,case_count=len(cases),all_prediction_flow_checks=flows,
          effective_natural_configuration=dict(params=header['params'],loader=header['loader']),cases=cases))
    lines=[]
    for case in cases:
        i=case['frame']
        for mode in P['modes']:
            values=' '.join(format(float(v),'.17g') for v in case['seeds'][mode])
            lines.append(f"{case['id']} {mode} {raw/(f'{i-1:06d}.bin')} {raw/(f'{i:06d}.bin')} {values}")
    payload=('\n'.join(lines)+'\n').encode()
    (stage/'native_input.txt').write_bytes(payload)
    return cases,payload


def summarize(seq, cases, stage):
    native=[json.loads(l) for l in (stage/'native.stdout').read_text().splitlines()]
    require(len(native)==len(cases)*len(P['modes']),'missing/extra cases')
    rows=[];maxima=dict(output=0.,H=0.,g=0.);tol=P['control_tolerances'];index=0
    for case in cases:
        truth=np.asarray(case['gt_relative']).reshape(4,4);step=float(np.linalg.norm(truth[:3,3]));moving=step>=.1
        natural_output=np.asarray(case['expected_output']).reshape(4,4)
        for mode in P['modes']:
            result=native[index];index+=1
            require(result['id']==case['id'] and result['mode']==mode and result['seed']==case['seeds'][mode], 'case/seed mismatch')
            require(result['features']==[case['expected_features'][k] for k in ('edge','ground','facade','reflector')],'feature mismatch')
            if mode=='natural':
                output=np.asarray(result['output']).reshape(4,4)
                H=sum(np.asarray(c['H']) for c in result['first_systems'])
                g=sum(np.asarray(c['g']) for c in result['first_systems'])
                expected=case['expected_first']
                for key,a,b in (('output',output,natural_output),('H',H,np.asarray(expected['H'])),('g',g,np.asarray(expected['g'])):
                    maxima[key]=max(maxima[key],float(np.max(np.abs(a-b))))
                    require(np.allclose(a,b,atol=tol['output_atol'] if key=='output' else tol['system_atol'],
                                        rtol=tol['output_rtol'] if key=='output' else tol['system_rtol']), 'natural replay mismatch '+case['id']+' '+key)
                require([c['n'] for c in result['first_systems']]==expected['class_pairs'],'first counts mismatch')
            def metrics(values):
                if any(isinstance(v,str) for v in values):return dict(status='nonfinite',length_ratio=None,vector_error=None,rotation_error_rad=None,bad_length=None)
                T=np.asarray(values).reshape(4,4)
                require(np.isfinite(T).all(),'untagged nonfinite')
                ratio=float(np.linalg.norm(T[:3,3])/step) if moving else None
                return dict(status='finite',length_ratio=ratio,
                    vector_error=float(np.linalg.norm(T[:3,3]-truth[:3,3])/step) if moving else None,
                    rotation_error_rad=rotation_angle(T[:3,:3].T@truth[:3,:3]),
                    bad_length=abs(ratio-1)>.2 if moving else None)
            sm=metrics(result['seed']);om=metrics(result['output'])
            row=dict(id=case['id'],frame=case['frame'],mode=mode,moving=moving,
                     natural_bad_length=case['labels']['s2s_bad_length']=='True',
                     final_bad_length=case['labels']['bad_length']=='True',gt_step_m=step,
                     **{'seed_'+k:v for k,v in sm.items()},**{'output_'+k:v for k,v in om.items()},
                     first_pairs=sum(c['n'] for c in result['first_systems']),
                     output_minus_natural_translation_m=float(np.linalg.norm(np.asarray(result['output']).reshape(4,4)[:3,3]-natural_output[:3,3])) if om['status']=='finite' else None)
            rows.append(row)
    groups={}
    for label in ('all_selected_moving','natural_bad_length','other_selected_moving'):
        groups[label]={}
        for mode in P['modes']:
            part=[r for r in rows if r['mode']==mode and r['moving'] and
                  (label=='all_selected_moving' or r['natural_bad_length']==(label=='natural_bad_length'))]
            groups[label][mode]=dict(cases=len(part),nonfinite_outputs=sum(r['output_status']!='finite' for r in part),
                bad_length_outputs=sum(r['output_bad_length'] is True for r in part),
                seed_length_ratio=analysis.distribution(r['seed_length_ratio'] for r in part),
                output_length_ratio=analysis.distribution(r['output_length_ratio'] for r in part),
                output_vector_error=analysis.distribution(r['output_vector_error'] for r in part),
                first_pairs=analysis.distribution(r['first_pairs'] for r in part))
    with (stage/'comparison.csv').open('w') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(rows[0]),lineterminator='\n');writer.writeheader();writer.writerows(rows)
    write(stage/'summary.json',dict(sequence=seq,case_count=len(cases),native_calls=len(native),exit_code=0,
          natural_replay_max_absolute_differences=maxima,groups=groups,
          limits=P['limits'],unavailable='Counterfactual stop reasons and last-iteration counts are not exposed by the existing forced diagnostic API; not inferred.'))


def main():
    require(not (E/'sequences').exists() and not (R/'probe').exists() and not (R/'staging').exists(),'fresh execution required')
    invariant();state=dict(status='building',completed=[],source_commit=git('rev-parse','HEAD'));write(R/'active.json',state)
    run('pkg_config',docker()+['pkg-config','--cflags','--libs','pcl_io','pcl_filters','pcl_kdtree'],R/'build')
    flags=shlex.split((R/'build/pkg_config.stdout').read_text())
    command=docker()+['c++','-std=c++17','-O3','-DNDEBUG','-fopenmp',
        '-I'+str(W/'papers/inten_loam/include'),'-I'+str(W/'common/include'),'-I'+str(W/'evaluation/src'),
        str(E/'probe.cpp'),P['library']['path'],*flags,'-o',str(R/'probe')]
    run('compile',command,R/'build');pin=analysis.sha(R/'probe');write(E/'binary.json',dict(path=str(R/'probe'),sha256=pin,library=P['library']))
    archive(R/'build',E/'build');subprocess.run(['git','add','-f',str(E/'binary.json'),str(E/'build')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Record fixed-pair diagnostic build and immutable library pin'],cwd=W,check=True);backup('build')
    for seq in P['sequences']:
        invariant();require(analysis.sha(R/'probe')==pin,'probe changed');stage=R/'staging'/seq;stage.mkdir(parents=True)
        state.update(status='fetching',sequence=seq);write(R/'active.json',state)
        for attempt in range(1,4):
            if run('fetch',[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),'--sequence',seq,'--workers','4'],stage/'fetch'/str(attempt),allowed=(0,1))==0:break
        else:raise RuntimeError('fetch failed')
        raw=R/'kitti/dataset/sequences'/seq/'velodyne'
        inputs={p.name:dict(sha256=analysis.sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}
        require(inputs==read(OLD/'sequences'/seq/'inputs.json')['files'],'official hashes mismatch')
        write(stage/'inputs.json',dict(sequence=seq,files=inputs,matches_committed_natural_inputs=True,owned_input_dir=str(raw)))
        state.update(status='preparing');write(R/'active.json',state);cases,payload=prepare(seq,stage,raw)
        state.update(status='native');write(R/'active.json',state)
        run('native',docker()+[str(R/'probe')],stage,payload=payload)
        state.update(status='validating');write(R/'active.json',state);summarize(seq,cases,stage)
        invariant();require(analysis.sha(R/'probe')==pin,'probe changed during run')
        require({p.name:dict(sha256=analysis.sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'input changed before deletion')
        target=E/'sequences'/seq;archive(stage,target)
        shutil.rmtree(raw);write(target/'cleanup.json',dict(sequence=seq,hashes_verified_before_removal=True,owned_input_dir=str(raw),input_dir_exists_after=raw.exists(),removed_files=len(inputs)))
        subprocess.run(['git','add','-f',str(target)],cwd=W,check=True)
        subprocess.run(['git','commit','-m','Record fixed-pair initialization interventions on KITTI '+seq],cwd=W,check=True)
        state['completed'].append(seq);state.update(last_commit=git('rev-parse','HEAD'),status='committed');write(R/'active.json',state)
        backup(seq);shutil.rmtree(stage)
    state.update(status='complete',sequence=None);write(R/'active.json',state);write(R/'run_exit.json',dict(exit_code=0,completed=state['completed']))


if __name__=='__main__':
    lock=R/'collector.lock'
    with os.fdopen(os.open(lock,os.O_CREAT|os.O_EXCL|os.O_WRONLY,0o600),'w') as stream:stream.write(str(os.getpid())+'\n')
    try:main()
    except BaseException as error:
        write(R/'failure.json',dict(exit_code=1,error=str(error),traceback=traceback.format_exc()))
        raise
    finally:lock.unlink()
