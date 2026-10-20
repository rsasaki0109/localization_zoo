"""Execute the committed plane-correspondence experiment; not a generic runner."""
from pathlib import Path
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


E = Path(__file__).resolve().parent
W = E.parents[2]
R = Path('/workspace/localization-env/inten-plane-correspondences')
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
    with (folder/(label+('.jsonl' if label=='native' else '.stdout'))).open('wb') as out, (folder/(label+'.stderr')).open('wb') as err:
        result = subprocess.run(command,cwd=W,env=ENV,input=payload,stdout=out,stderr=err)
    write(folder/(label+'_command.json'),dict(command=command,cwd=str(W),source_commit=git('rev-parse','HEAD'),
          started_unix=started,finished_unix=time.time(),exit_code=result.returncode,environment=P['environment'],
          stdin_sha256=hashlib.sha256(payload).hexdigest() if payload else None))
    print(label,result.returncode,flush=True)
    require(result.returncode in allowed,label+' failed')
    return result.returncode


def invariant():
    require(analysis.sha(P['library']['path'])==P['library']['sha256'],'library changed')
    for path,digest in P['build_metadata'].items():
        require(analysis.sha(path)==digest,'build metadata changed')
    for name in ('run.py','probe.cpp','native_loader.h','native_plane.h','geometry.py','protocol.json'):
        path=E/name
        require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)==path.read_bytes(),
                'uncommitted source '+name)
    require(not git('diff','--name-only',P['parent_result_commit'],'--','common','evaluation',
                    ':(glob)papers/*/src/**',':(glob)papers/*/include/**',
                    'experiments/results/inten_natural_traces','experiments/results/inten_seed_intervention'),'immutable inputs/source changed')
    matcher=(W/P['plane_matcher']['source']).read_text()
    require(hashlib.sha256(matcher.encode()).hexdigest()==P['plane_matcher']['source_sha256'],'plane source changed')
    fragment=(E/'native_plane.h').read_text().split('\n',1)[1].rstrip('\n')
    require(fragment in matcher and hashlib.sha256(fragment.encode()).hexdigest()==P['plane_matcher']['fragment_sha256'],'plane fragment changed')
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
        if run('push',['git','push','origin','HEAD:refs/heads/codex/inten-plane-correspondences'],
               R/'backups'/label/str(attempt),allowed=(0,1,128))==0:
            return
    raise RuntimeError('Backup failed; stop before next fetch')


def prepare(seq, stage, raw):
    parent=W/'experiments/results/inten_seed_intervention/sequences'/seq
    data=read(parent/'cases.json');cases=data['cases']
    write(stage/'cases.json',data)
    lines=[]
    for case in cases:
        i=case['frame'];values=' '.join(format(float(v),'.17g') for v in case['seeds']['oracle_current_gt'])
        lines.append(f"{case['id']} oracle_current_gt {raw/(f'{i-1:06d}.bin')} {raw/(f'{i:06d}.bin')} {values}")
    payload=('\n'.join(lines)+'\n').encode()
    (stage/'native_input.txt').write_bytes(payload)
    return cases,payload


def summarize(seq, cases, stage):
    parent=W/'experiments/results/inten_seed_intervention/sequences'/seq
    with tempfile.TemporaryDirectory(dir=R) as temp:
        analysis.restore(parent,Path(temp))
        rows=[json.loads(line) for line in (Path(temp)/'native.stdout').read_text().splitlines()]
    expected={row['id']:row for row in rows if row['mode']=='oracle_current_gt'}
    geometry=module('plane_geometry',E/'geometry.py')
    result=geometry.analyze_sequence(cases,stage/'native.jsonl',expected,P['control_tolerances'])
    write(stage/'geometry.json',result)
    print('geometry',seq,result['summary'],flush=True)


def main():
    require(not (E/'sequences').exists() and not (R/'probe').exists() and not (R/'staging').exists(),'fresh execution required')
    invariant();state=dict(status='building',completed=[],source_commit=git('rev-parse','HEAD'));write(R/'active.json',state)
    build=Path(P['library']['path']).parent
    flags_text=(build/'CMakeFiles/test_inten_loam_paper.dir/flags.make').read_text()
    flags_text=flags_text.replace(str(build.parents[2]/'worktree'),str(W))
    flags=[]
    for line in flags_text.splitlines():
        if line.startswith(('CXX_INCLUDES =','CXX_DEFINES =','CXX_FLAGS =')):
            flags.extend(shlex.split(line.split('=',1)[1]))
    link=shlex.split((build/'CMakeFiles/test_inten_loam_paper.dir/link.txt').read_text())
    libs=[str(build/a) if not a.startswith(('/','-')) else a for a in link[link.index('libinten_loam.a'):]]
    command=docker()+['c++',*flags,'-I'+str(W/'evaluation/src'),str(E/'probe.cpp'),*libs,'-o',str(R/'probe')]
    run('compile',command,R/'build');pin=analysis.sha(R/'probe');write(E/'binary.json',dict(path=str(R/'probe'),sha256=pin,library=P['library']))
    archive(R/'build',E/'build');subprocess.run(['git','add','-f',str(E/'binary.json'),str(E/'build')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Record plane diagnostic build and immutable native library'],cwd=W,check=True);backup('build')
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
        subprocess.run(['git','add','-f',str(target)],cwd=W,check=True)
        subprocess.run(['git','commit','-m','Record actual plane correspondence controls on KITTI '+seq],cwd=W,check=True)
        backup(seq+'_raw')
        require({p.name:dict(sha256=analysis.sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'input changed before cleanup')
        shutil.rmtree(raw);write(target/'cleanup.json',dict(sequence=seq,hashes_verified_before_removal=True,owned_input_dir=str(raw),input_dir_exists_after=raw.exists(),removed_files=len(inputs)))
        subprocess.run(['git','add','-f',str(target)],cwd=W,check=True)
        subprocess.run(['git','commit','-m','Record verified scan cleanup for KITTI '+seq],cwd=W,check=True)
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
