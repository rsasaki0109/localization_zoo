"""Recorded experiment controller; run with host-venv Python and Docker access.

The fixed native executable is pinned in candidate.json. Data live outside the
repository. Each sequence is committed after verification and owned-data cleanup.
"""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import argparse,csv,hashlib,json,os,shutil,subprocess,sys,threading,time,traceback

E=Path(__file__).resolve().parent
REPO=E.parents[2]
R=Path('/workspace/localization-env/shared-coarse-start')
PY='/workspace/localization-env/host-venv/bin/python'
CONTAINER_PY='/opt/localization-venv/bin/python'
WRAPPER=E/'native_wrapper.py'
SOURCE=json.loads((E/'candidate.json').read_text())['source_commit']
PROTOCOL=json.loads((E/'protocol.json').read_text())
ORDER=PROTOCOL['order']
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--resume-from',choices=ORDER,help='Resume after a fetch-stage failure; native artifacts in that sequence are refused.')
parser.add_argument('--check-resume-only',action='store_true',help='Validate committed checkpoints without writing state or starting processes.')
options=parser.parse_args()
if options.check_resume_only and not options.resume_from:parser.error('--check-resume-only requires --resume-from')
lock=threading.Lock()
state={'source_commit':SOURCE,'status':'starting','sequence':None,'runners':{},'completed':[]}

def write(path,payload):
    path.parent.mkdir(parents=True,exist_ok=True)
    temporary=path.with_name(path.name+'.tmp')
    temporary.write_text(json.dumps(payload,indent=2,allow_nan=False)+'\n')
    temporary.replace(path)

def update(**fields):
    with lock:
        state.update(fields)
        write(R/'active.json',state)

def digest(path):
    value=hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda:source.read(1024*1024),b''):value.update(chunk)
    return value.hexdigest()

def docker(native=None):
    args=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH',
      'docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1',
      '-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1']
    if native:args+=['-e','ZOO_COARSE_NATIVE_BINARY='+str(native)]
    return args+['-w',str(REPO),'localization-zoo-dev']

def run(label,command,folder,allowed=(0,)):
    folder.mkdir(parents=True,exist_ok=True)
    started=time.time()
    with lock:
        state['runners'][label]={'command':command,'started_unix':started}
        write(R/'active.json',state)
    with (folder/(label+'.log')).open('w') as log:
        result=subprocess.run(command,cwd=REPO,stdout=log,stderr=subprocess.STDOUT)
    receipt={'label':label,'command':command,'cwd':str(REPO),'implementation_commit':SOURCE,
             'started_unix':started,'finished_unix':time.time(),'exit_code':result.returncode}
    write(folder/(label+'_command.json'),receipt)
    with lock:
        state['runners'].pop(label,None)
        write(R/'active.json',state)
    if result.returncode not in allowed:raise RuntimeError(f'{label} exited {result.returncode}')
    return result.returncode

def audit(seq,folder,methods=('IMLS-SLAM','SuMa'),frozen=False):
    command=docker(R/'baseline_pcd_dogfooding' if frozen else None)+[CONTAINER_PY,
      'evaluation/scripts/audit_kitti_drift.py','--binary',str(WRAPPER),'--sequence',seq,
      '--data-root',str(R/'data'),'--work-dir',str(folder/'replays'),'--output',str(folder/'report.json'),'--jobs','2']
    for method in methods:command+=['--method',method]
    run('frozen_audit' if frozen else 'audit',command,folder,allowed=(0,1))
    return json.loads((folder/'report.json').read_text())

def method(summary):
    payload=json.loads(summary.read_text())
    item=next(x for x in payload['methods'] if x['name']=='SuMa')
    if item['status']!='ok':raise RuntimeError(f'failed SuMa result: {summary}')
    return item

def native_output(summary):
    return summary.parent/(summary.stem+'_native')/'dogfooding_results'

def candidate_and_control(seq,folder):
    manifest=REPO/'experiments'/f'suma_kitti_seq_{seq}_full_shared_coarse_start_matrix.json'
    variants=json.loads(manifest.read_text())['variants']
    assert len(variants)==1 and variants[0]['args']==PROTOCOL['suma_fixed_arguments']
    command=docker()+[CONTAINER_PY,'evaluation/scripts/run_experiment_matrix.py','--manifest',str(manifest),
      '--binary',str(WRAPPER),'--pcd-dir',str(R/'data/dogfooding_results'/f'kitti_seq_{seq}_full'),
      '--output-dir',str(E/'benchmarks'),'--docs-dir',str(E/'benchmarks/docs'),'--merge-existing-index']
    run('candidate',command,folder)
    summary=E/'benchmarks/runs'/manifest.stem/'shared_coarse_start_1x5x10/summary.json'
    method(summary)
    control=None
    if seq in ('01','02'):
        control=folder/'raw_control/summary.json';control.parent.mkdir()
        command=docker()+[str(WRAPPER),str(R/'data/dogfooding_results'/f'kitti_seq_{seq}_full'),
          str(REPO/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv'),'--methods','suma',
          '--summary-json',str(control),'--no-gt-seed','--suma-paper']
        run('raw_control',command,control.parent)
        method(control)
    return summary,control

def compare_frozen(new,old,folder):
    import numpy as np
    checks=[]
    for previous in old['rows']:
        current=next(row for row in new['rows'] if row['method']==previous['method'])
        deltas={key:abs(current[key]-previous[key]) for key in ('rerun_rpe_trans_pct','rerun_ate_m','frames')}
        name=current['method'].replace('-','_')+'.txt'
        new_poses=native_output(Path(current['summary_path']))/name
        old_poses=native_output(Path(previous['summary_path']))/name
        a=np.loadtxt(new_poses);b=np.loadtxt(old_poses)
        checks.append({'method':current['method'],'sequence':current['sequence'],
            'metric_abs_delta':deltas,'saved_pose_bytes_equal':new_poses.read_bytes()==old_poses.read_bytes(),
            'saved_pose_max_abs_delta':float(np.max(np.abs(a-b)))})
    write(folder/'frozen_comparison.json',checks)
    if any(any(value>1e-9 for value in row['metric_abs_delta'].values()) for row in checks):
        raise RuntimeError('shared startup refactor changed a printed existing result; see frozen comparison')


def diagnostics(summary,output):
    source=native_output(summary)
    gt=next(source.glob('*_evaluated_gt.txt'))
    run(output.stem,[PY,'evaluation/scripts/diagnose_kitti_steps.py','--estimate',str(source/'SuMa.txt'),
        '--gt',str(gt),'--summary',str(summary),'--method','SuMa','--output',str(output)],output.parent)

def committed_checkpoints(resume_from):
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
    def committed(path):
        return json.loads(subprocess.check_output(['git','show',revision+':'+path.relative_to(REPO).as_posix()],cwd=REPO))
    completed=ORDER[:ORDER.index(resume_from)]
    protocol=committed(E/'protocol.json')
    for seq in completed:
        folder=E/'sequences'/seq
        comparison=committed(folder/'comparison.json');cleanup=committed(folder/'cleanup.json');inputs=committed(folder/'inputs.json')
        assert comparison['source_commit']==SOURCE==protocol['implementation_commit']
        assert comparison['candidate_arguments']==protocol['suma_fixed_arguments']
        assert comparison['control']['frames']==comparison['candidate']['frames']==inputs['gt_frames']==inputs['scan_frames']
        assert cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after']
        assert Path(cleanup['owned_input_dir'])==R/'kitti/dataset/sequences'/seq/'velodyne'
        assert not Path(cleanup['owned_input_dir']).exists()
        alias=R/'data/dogfooding_results'/f'kitti_seq_{seq}_full'
        assert not alias.exists() and not alias.is_symlink()
    folder=E/'sequences'/resume_from
    if any((folder/name).exists() for name in ('inputs.json','audit','raw_control','comparison.json')):
        raise RuntimeError('Refusing fetch-stage resume with native artifacts or completed inputs')
    if (E/'benchmarks/runs'/f'suma_kitti_seq_{resume_from}_full_shared_coarse_start_matrix').exists():
        raise RuntimeError('Refusing resume with an existing candidate run')
    return completed,revision

def archive_failed_fetch(folder):
    receipt=folder/'fetch_command.json'
    if not receipt.exists() or json.loads(receipt.read_text())['exit_code']==0:return
    attempts=folder/'fetch_attempts';attempts.mkdir(exist_ok=True)
    number=max((int(p.name) for p in attempts.iterdir() if p.is_dir() and p.name.isdigit()),default=0)+1
    destination=attempts/f'{number:02d}';destination.mkdir()
    for name in ('fetch.log','fetch_command.json'):shutil.move(str(folder/name),str(destination/name))

def fetch(seq,folder):
    archive_failed_fetch(folder)
    command=[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),
             '--sequence',seq,'--workers','4']
    for attempt in range(3):
        if run('fetch',command,folder,allowed=(0,1))==0:return
        archive_failed_fetch(folder)
    raise RuntimeError('fetch failed after bounded retries; all failed receipts retained')

if options.resume_from:
    completed,checkpoint_revision=committed_checkpoints(options.resume_from)
    if options.check_resume_only:
        print(json.dumps({'checkpoint_revision':checkpoint_revision,'completed':completed,'resume_from':options.resume_from,'native_artifacts_absent':True}))
        raise SystemExit(0)
    state['completed']=completed
    state['resume_from']=options.resume_from
    state['checkpoint_revision']=checkpoint_revision
elif any((E/'sequences'/seq).exists() for seq in ORDER):
    raise RuntimeError('Fresh execution requires empty sequence artifact directories')

try:
    subprocess.run(['git','diff','--exit-code',SOURCE,'--','common','evaluation/src','papers/*/src','papers/*/include','papers/*/test','papers/*/CMakeLists.txt','CMakeLists.txt','ros2/localization_zoo_ros/CMakeLists.txt'],cwd=REPO,check=True)
    assert digest(R/'candidate_pcd_dogfooding')==json.loads((E/'candidate.json').read_text())['sha256']
    assert digest(R/'baseline_pcd_dogfooding')==json.loads((E/'baseline.json').read_text())['sha256']
    (R/'data/dogfooding_results').mkdir(parents=True,exist_ok=True)
    for seq in ORDER[len(state['completed']):]:
        update(status='running',sequence=seq)
        folder=E/'sequences'/seq;folder.mkdir(parents=True,exist_ok=options.resume_from==seq)
        fetch(seq,folder)
        raw=R/'kitti/dataset/sequences'/seq/'velodyne'
        assert raw.is_dir() and not raw.is_symlink()
        link=R/'data/dogfooding_results'/f'kitti_seq_{seq}_full'
        if not link.exists():link.symlink_to(raw,target_is_directory=True)
        assert link.is_symlink() and link.resolve()==raw.resolve()
        scans=sorted(raw.glob('*.bin'))
        with (REPO/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv').open() as stream:
            gt_frames=sum(1 for row in csv.DictReader(stream))
        assert len(scans)==gt_frames and [p.name for p in scans]==[f'{i:06d}.bin' for i in range(gt_frames)]
        hashes={p.name:{'sha256':digest(p),'bytes':p.stat().st_size} for p in sorted(raw.iterdir()) if p.is_file()}
        write(folder/'inputs.json',{'sequence':seq,'scan_frames':len(scans),'gt_frames':gt_frames,'input_dir':str(raw),
                                   'gt_csv_sha256':digest(REPO/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv'),'files':hashes})
        with ThreadPoolExecutor(max_workers=2) as pool:
            audit_future=pool.submit(audit,seq,folder/'audit')
            candidate_future=pool.submit(candidate_and_control,seq,folder)
            report=audit_future.result()
            summary,control=candidate_future.result()
        if control is None:
            control=Path(next(row['summary_path'] for row in report['rows'] if row['method']=='SuMa'))
            assert next(row['args'] for row in report['rows'] if row['method']=='SuMa')==['--no-gt-seed','--suma-paper']
        current,reference=method(summary),method(control)
        assert current['frames']==reference['frames']==gt_frames
        assert 'opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)' in current['note']
        drifted=[row['method'] for row in report['rows'] if row['audit_status']=='drift']
        if seq=='04' or drifted:
            old=audit(seq,folder/'frozen_baseline',methods=('IMLS-SLAM','SuMa') if seq=='04' else drifted,frozen=True)
            compare_frozen(report,old,folder)
        diagnostics(summary,folder/'candidate_drift.json')
        diagnostics(control,folder/'raw_control_drift.json')
        write(folder/'comparison.json',{'sequence':seq,'source_commit':SOURCE,'control_summary':str(control),
          'candidate_summary':str(summary),'control':reference,'candidate':current,
          'audit_exit_code':report['exit_code'],'audit_rows':report['rows'],
          'candidate_arguments':PROTOCOL['suma_fixed_arguments'],
          'scope':'Fixed-setting full sequence; raw control vs non-paper opt-in. No speed claim.'})
        update(status='verifying_cleanup')
        assert {p.name for p in raw.iterdir()}==set(hashes)
        for name,meta in hashes.items():assert digest(raw/name)==meta['sha256']
        link.unlink()
        removed_bytes=sum(meta['bytes'] for meta in hashes.values())
        shutil.rmtree(raw)
        write(folder/'cleanup.json',{'sequence':seq,'owned_input_dir':str(raw),'removed_files':len(hashes),
          'removed_bytes':removed_bytes,'hashes_verified_before_removal':True,'input_dir_exists_after':raw.exists()})
        subprocess.run(['git','add','-f',str(folder),str(E/'benchmarks')],cwd=REPO,check=True)
        subprocess.run(['git','commit','-m',f'Record fixed SuMa coarse-start evaluation and affected-method audit on KITTI {seq}'],cwd=REPO,check=True)
        with lock:state['completed'].append(seq)
        update(status='sequence_committed',last_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip())
    update(status='complete',sequence=None)
    write(R/'run_exit.json',{'exit_code':0,'completed':state['completed']})
except BaseException as error:
    update(status='failed',error=str(error))
    write(R/'run_exit.json',{'exit_code':1,'error':str(error),'sequence':state['sequence']})
    traceback.print_exc()
    raise
