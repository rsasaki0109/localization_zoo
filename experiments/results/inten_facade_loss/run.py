"""Frozen full01 matched-seed loss controls; preserve failures and inputs."""
from pathlib import Path
import hashlib,importlib.util,json,os,shlex,subprocess,time

E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',
         PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
build=module('loss_build',E/'build.py')
read=lambda p:json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def git(*args):return subprocess.check_output(['git',*args],cwd=W,text=True).strip()
def run(label,command,folder,payload=None):
    folder.mkdir(parents=True,exist_ok=True);receipt=folder/(label+'_command.json')
    if receipt.exists():raise RuntimeError('Existing receipt: '+str(receipt))
    started=time.time()
    with (folder/(label+'.stdout.log')).open('xb') as out,(folder/(label+'.stderr.log')).open('xb') as err:
        p=subprocess.run(command,cwd=W,env=ENV,input=payload,stdout=out,stderr=err)
    write(receipt,dict(command=command,cwd=str(W),source_commit=git('rev-parse','HEAD'),started_unix=started,
        finished_unix=time.time(),exit_code=p.returncode,
        environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')},
        PATH_prefix=str(Path(PY).parent),stdin_sha256=hashlib.sha256(payload).hexdigest() if payload else None))
    print(label,p.returncode,flush=True);return p.returncode


def main():
    if git('status','--porcelain') or (R/'stage').exists():raise RuntimeError('Clean frozen source and fresh execution required')
    p=read(E/'protocol.json')
    if sha(W/'papers/inten_loam/src/inten_loam_paper.cpp')!=p['native_source_sha256'] or sha(E/'diagnostic_native.cpp')!=p['diagnostic_source_sha256'] or sha(E/'native_loader.h')!=p['loader_sha256']:
        raise RuntimeError('Frozen source changed')
    original=(W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text()
    if module('loss_instrument',E/'instrument.py').transformed(original)!=(E/'diagnostic_native.cpp').read_text():raise RuntimeError('Unexpected native transformation')
    stage=R/'stage';stage.mkdir()
    flags=build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags+=['-I'+str(W/'evaluation/src'),'-I'+str(E)]
    if run('compile_native',build.docker()+['c++',*flags,'-c',str(E/'diagnostic_native.cpp'),'-o',str(R/'native.o')],stage):raise RuntimeError('Compile failed')
    link=shlex.split((build.OLD/'evaluation/CMakeFiles/pcd_dogfooding.dir/link.txt').read_text());libs=[];pins={}
    for token in link[link.index('-o')+2:]:
        if token.startswith('-'):libs.append(token);continue
        path=Path(token);path=path if path.is_absolute() else (build.OLD/'evaluation'/path).resolve()
        if path==build.OLD/'papers/inten_loam/libinten_loam.a':continue
        if path.suffix=='.a':pins[str(path)]=sha(path)
        libs.append(str(path))
    if run('compile_probe',build.docker()+['c++',*flags,str(E/'probe.cpp'),str(R/'native.o'),*libs,'-o',str(R/'probe')],stage):raise RuntimeError('Probe link failed')
    write(stage/'binary.json',dict(path=str(R/'probe'),sha256=sha(R/'probe'),object_sha256=sha(R/'native.o'),
        linked_static_libraries=pins,compiler_metadata_sha256=sha(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make'),source_commit=git('rev-parse','HEAD')))
    for attempt in range(1,5):
        if run('fetch',[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),'--sequence','01','--workers','4'],R/'fetch'/str(attempt))==0:break
    else:raise RuntimeError('Official acquisition failed; retain partial inputs')
    raw=R/'kitti/dataset/sequences/01/velodyne';gt=W/'experiments/reference_data/kitti_seq_01_full_gt.csv'
    previous=read(W/'experiments/results/shared_near_range/sequences/01/inputs.json')
    inputs={f.name:dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted(raw.iterdir()) if f.is_file()}
    if inputs!=previous['files'] or sha(gt)!=previous['gt_csv_sha256']:raise RuntimeError('Official input hash mismatch')
    write(stage/'inputs.json',dict(sequence='01',owned_input_dir=str(raw),files=inputs,gt_csv_sha256=sha(gt),scan_frames=previous['scan_frames'],exact_previous_official_hashes=True))
    restore=module('loss_restore',W/'experiments/results/inten_natural_traces/diagnose.py').restore
    reference=R/'reference';reference.mkdir()
    restore(W/'experiments/results/inten_prediction_dor/sequences/01',reference)
    restore(W/'experiments/results/inten_prediction_dor/terminal',R/'preceding_terminal')
    traces={}
    with (reference/'baseline/trace.jsonl').open() as stream:
        for line in stream:
            row=json.loads(line)
            if row['type']=='header':write(stage/'reference_header.json',row)
            if row['type']=='frame':traces[row['frame_index']]=row
    if sorted(traces)!=list(range(previous['scan_frames'])):raise RuntimeError('Reference trace incomplete')
    lines=[]
    for i in range(1,len(traces)):
        seed=' '.join(format(v,'.17g') for v in traces[i]['s2s']['input'])
        for mode in (0,1):lines.append(f'{i} {mode} {raw/f"{i-1:06d}.bin"} {raw/f"{i:06d}.bin"} {seed}')
    payload=('\n'.join(lines)+'\n').encode();(stage/'input.txt').write_bytes(payload)
    if run('native',build.docker()+[str(R/'probe')],stage,payload):raise RuntimeError('Native terminal controls failed')
    if sha(R/'probe')!=read(stage/'binary.json')['sha256']:raise RuntimeError('Native producer changed')
    write(R/'native_complete.json',dict(exit_code=0,native_pairs=len(traces)-1,native_calls=2*(len(traces)-1),raw_retained=True))


if __name__=='__main__':main()
