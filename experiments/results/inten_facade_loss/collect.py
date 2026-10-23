"""One fixed facade-Cauchy S2S facade gate on every full KITTI00-10 sequence."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import gzip
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import time
import traceback

E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
PY='/workspace/localization-env/host-venv/bin/python'
P=json.loads((E/'full_protocol.json').read_text());PINS=json.loads((E/'binaries.json').read_text())
ENV=dict(os.environ,PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',MPLCONFIGDIR=str(R/'matplotlib'),XDG_CACHE_HOME=str(R/'cache'))
BRANCH='codex/inten-facade-loss'

def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
archival=module('archive',W/'experiments/results/inten_natural_traces/run_sequences.py')
build=module('build',E/'build.py');sha,docker=archival.sha,build.docker

def read(p):return json.loads(Path(p).read_text())
def write(p,v):
    p.parent.mkdir(parents=True,exist_ok=True);t=p.with_name(p.name+'.tmp');t.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n');t.replace(p)
def git(*args):return subprocess.check_output(['git',*args],cwd=W,text=True).strip()
def require(ok,reason):
    if not ok:raise ValueError(reason)

def run(label,argv,folder,allowed=(0,),native_argv=None):
    folder.mkdir(parents=True,exist_ok=True);receipt=folder/(label+'_command.json');require(not receipt.exists(),'Existing receipt: '+str(receipt));started=time.time()
    with (folder/(label+'.log')).open('xb') as out,(folder/(label+'_stderr.log')).open('xb') as err:
        result=subprocess.run(argv,cwd=W,env=ENV,stdout=out,stderr=err)
    record=dict(command=argv,cwd=str(W),source_commit=git('rev-parse','HEAD'),started_unix=started,finished_unix=time.time(),exit_code=result.returncode,environment=P['native_environment'])
    if native_argv is not None:record.update(native_argv=native_argv,native_cwd=str(folder),binary_sha256=sha(PINS['runner']['path']),native_source_commit=P['native_source_commit'])
    write(receipt,record);print(label,folder.name,result.returncode,flush=True)
    require(result.returncode in allowed,label+' failed: retain evidence and stop');return result.returncode

def backup(label):
    for attempt in range(1,4):
        if run('push',['git','push','origin','HEAD:refs/heads/'+BRANCH],R/'backups'/label/str(attempt),allowed=(0,1,128))==0:return
    raise RuntimeError('Backup failed; stop before raw cleanup or further acquisition')

def invariant():
    require(git('branch','--show-current')==BRANCH,'Wrong branch')
    require(not git('diff','--name-only',P['native_source_commit'],'--','common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**','CMakeLists.txt'),'Frozen native source changed')
    for name in ('collect.py','full_protocol.json','binaries.json','build_candidate.py','native_loader.h'):
        path=E/name;require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)==path.read_bytes(),'Uncommitted collector source '+name)
    for item in (PINS['runner'],):require(sha(item['path'])==item['sha256'],'Pinned executable changed')
    for path,digest in PINS['linked_static_libraries'].items():require(sha(path)==digest,'Pinned native library changed')
    for meta in P['manifests'].values():require(sha(W/meta['path'])==meta['sha256'],'Manifest changed')
    historical=read(E/'historical_aggregate_sha256.json');require(all(sha(W/name)==digest for name,digest in historical.items()),'Historical aggregate changed')
    # Preserve all inherited scientific trees, including the earlier failed trial.
    changed=git('diff','--name-only',P['parent_commit'],'--','experiments/results').splitlines()
    require(all(n.startswith(str(E.relative_to(W))+'/') for n in changed),'Inherited evidence changed')

def native(mode,stage,raw,gt,args,trace=True):
    folder=stage/mode;folder.mkdir()
    argv=[PINS['runner']['path'],str(raw),str(gt),'--methods','inten_loam','--summary-json',str(folder/'summary.json'),*args]
    if trace:argv+=['--inten-loam-paper-trace-jsonl',str(folder/'trace.jsonl')]
    run('native',docker(folder)+argv,folder,native_argv=argv)

def preflight():
    stage=R/'preflight';stage.mkdir();m=read(W/P['manifests']['07']['path']);args=m['variants'][1]['args']
    for mode,on in [('trace_off',False),('trace_on',True)]:native(mode,stage,W/'evaluation/fixtures/mcd_kth_smoke',W/'evaluation/fixtures/mcd_kth_smoke_gt.csv',args,on)
    off,on=stage/'trace_off',stage/'trace_on';a,b=read(off/'summary.json'),read(on/'summary.json');fields=('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
    require(all(a['methods'][0].get(k)==b['methods'][0].get(k) for k in fields),'Trace changes candidate metrics')
    names=sorted(p.name for p in (off/'dogfooding_results').glob('*.txt'));require(names and all((off/'dogfooding_results'/n).read_bytes()==(on/'dogfooding_results'/n).read_bytes() for n in names),'Trace changes candidate pose bytes')
    with (on/'trace.jsonl').open() as stream:h=json.loads(next(stream))
    require(h['params'].get('s2s_facade_cauchy') is True,'Predicted-DOR option ineffective')
    write(stage/'parity.json',dict(exit_code=0,fields_equal=list(fields),saved_pose_gt_bytes_equal=names,enabled_flag_effective=True))
    archival.archive(stage,E/'checks/cli_preflight');subprocess.run(['git','add','-f',str(E/'checks/cli_preflight')],cwd=W,check=True);subprocess.run(['git','commit','-m','Verify facade-Cauchy candidate CLI trace parity'],cwd=W,check=True);backup('preflight')

def same_parent(seq,stage):
    old=W/'experiments/results/inten_facade_pca/sequences'/seq;current=stage/'baseline';a,b=read(old/'candidate/summary.json'),read(current/'summary.json');fields=('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
    require(all(a['methods'][0].get(k)==b['methods'][0].get(k) for k in fields),'Historical reference metric drift')
    names=sorted(p.name for p in (old/'candidate/dogfooding_results').glob('*.txt'));require(names and all((old/'candidate/dogfooding_results'/n).read_bytes()==(current/'dogfooding_results'/n).read_bytes() for n in names),'Historical reference pose/GT drift')
    meta=read(old/'archive.json')['files']['candidate/trace.jsonl'];require(meta['storage']=='ordered_gzip_shards','Unexpected inherited trace storage')
    old_root=a['pcd_dir'].rstrip('/')+'/'
    new_root=b['pcd_dir'].rstrip('/')+'/'
    lines=0;first=True
    with (current/'trace.jsonl').open('rb') as fresh:
        for part in meta['parts']:
            path=old/part['path'];require(sha(path)==part['gzip_sha256'],'Inherited trace shard changed')
            # Shards may cut a line. Reassemble the original byte stream below.
        class Parts:
            def __iter__(self):
                remainder=b''
                for part in meta['parts']:
                    with gzip.open(old/part['path'],'rb') as stream:
                        while True:
                            chunk=stream.read(1024*1024)
                            if not chunk:break
                            chunks=(remainder+chunk).split(b'\n');remainder=chunks.pop()
                            for line in chunks:yield line+b'\n'
                require(not remainder,'Inherited trace lacks terminal newline')
        for historical in Parts():
            actual=fresh.readline();require(bool(actual),'Fresh trace truncated')
            if first:
                h,k=json.loads(historical),json.loads(actual);require(h['params']==k['params'] and h['loader']==k['loader'],'Historical reference effective settings drift');first=False
            else:
                record=json.loads(historical)
                normalized=historical
                if record['type']=='frame':
                    expected=old_root+f"{record['input_index']:06d}.bin/cloud.pcd"
                    require(record['scan_path']==expected,'Unexpected historical scan identity')
                    normalized=historical.replace(('\"scan_path\":\"'+old_root).encode(),('\"scan_path\":\"'+new_root).encode(),1)
                require(actual==normalized,'Historical reference numerical trace/body drift');lines+=1
        require(not fresh.read(1),'Fresh trace has extra records')
    write(stage/'legacy_compatibility.json',dict(exit_code=0,scope='Every fresh baseline uses the freshly compiled default-off candidate binary and reproduces its prior candidate all trace numeric bytes after the declared scan-root-only byte substitution, saved poses/GT and accuracy metrics exactly. Header argv paths differ; original traces are retained unchanged.',frames_and_end_records=lines,trace_body_exact_after_scan_root_substitution=True,scan_root_substitution=dict(historical=old_root,current=new_root),saved_pose_gt_bytes_equal=names,accuracy_fields_equal=list(fields)))

def analyze(seq,stage,expected):
    configs={}
    for mode in ('baseline','candidate'):
        f=stage/mode
        run('validate',[PY,'evaluation/scripts/check_inten_trace.py','--trace',str(f/'trace.jsonl'),'--summary',str(f/'summary.json'),'--receipt',str(f/'native_command.json'),'--output',str(f/'trace_validation.json')],f/'analysis')
        v=read(f/'trace_validation.json');require(v['skipped_frames']==0 and v['nonfinite_values']==0 and v['processed_frames']==expected['scan_frames'],'Incomplete/nonfinite traversal')
        with (f/'trace.jsonl').open() as stream:h=json.loads(next(stream))
        p=h['params'];require(p['accelerated'] is True and p.get('s2s_mapped_prior') is True and p.get('s2s_translation_prior_ratio')==.03 and p.get('s2s_ground_pca') is True and p.get('s2s_facade_pca') is True and p['huber']==.1 and not p.get('s2s_predicted_dor_facade',False) and not p.get('s2s_row_support',False),'Inherited profile changed')
        require(p['s2m_plane_min_spread']==0. and p.get('s2s_facade_cauchy',False)==(mode=='candidate'),'Fixed facade-Cauchy option or spread differs');configs[mode]=dict(params=p,loader=h['loader'])
        s=read(f/'summary.json');gt=Path(s['artifacts']['evaluated_gt_path']);gt=gt if gt.is_absolute() else f/gt
        run('steps',[PY,'evaluation/scripts/diagnose_kitti_steps.py','--estimate',str(f/'dogfooding_results/InTEn_LOAM.txt'),'--gt',str(gt),'--summary',str(f/'summary.json'),'--method','InTEn-LOAM','--output',str(f/'motion_steps.json')],f/'analysis')
    a,b=[dict(configs[m]['params']) for m in ('baseline','candidate')];a.pop('s2s_facade_cauchy',None);b.pop('s2s_facade_cauchy',None);require(a==b and configs['baseline']['loader']==configs['candidate']['loader'],'More than facade-Cauchy facade gate changes')
    write(stage/'effective_configuration.json',configs);same_parent(seq,stage);return configs

def main():
    require(not (E/'sequences').exists() and not (R/'full_staging').exists(),'Fresh single collection required; never silently restart')
    invariant();state=dict(status='starting',completed=[],source_commit=git('rev-parse','HEAD'));write(R/'active.json',state);backup('source');preflight();canonical=None
    for seq in P['sequence_order']:
        invariant();stage=R/'full_staging'/seq;stage.mkdir(parents=True);state.update(status='fetching',sequence=seq);write(R/'active.json',state)
        expected_inputs=read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json')['files']
        require(shutil.disk_usage(R).free > sum(v['bytes'] for v in expected_inputs.values())+1024**3,'Insufficient disk reserve before acquisition; preserve all evidence and stop')
        for attempt in range(1,5):
            if run('fetch',[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),'--sequence',seq,'--workers','4'],stage/'fetch'/str(attempt),allowed=(0,1))==0:break
        else:raise RuntimeError('Official acquisition failed; retain partial scans and receipts')
        raw=R/'kitti/dataset/sequences'/seq/'velodyne';gt=W/f'experiments/reference_data/kitti_seq_{seq}_full_gt.csv';previous=read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json')
        inputs={p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()};require(inputs==previous['files'] and sha(gt)==previous['gt_csv_sha256'],'Official scan/GT hashes changed')
        write(stage/'inputs.json',dict(sequence=seq,files=inputs,gt_csv_sha256=sha(gt),scan_frames=previous['scan_frames'],owned_input_dir=str(raw),matches_previous_official_inputs=True))
        m=read(W/P['manifests'][seq]['path']);args={v['id']:v['args'] for v in m['variants']};state.update(status='native');write(R/'active.json',state)
        with ThreadPoolExecutor(max_workers=2) as pool:
            futures=[pool.submit(native,mode,stage,raw,gt,args[mode]) for mode in ('baseline','candidate')]
            for future in futures:future.result()
        state.update(status='validating');write(R/'active.json',state);configs=analyze(seq,stage,previous)
        if canonical is None:canonical=configs
        else:require(canonical==configs,'Effective configurations vary across sequences')
        invariant();require({p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'Input changed')
        target=E/'sequences'/seq;archival.archive(stage,target);subprocess.run(['git','add','-f',str(target)],cwd=W,check=True);subprocess.run(['git','commit','-m','Record fixed full facade-Cauchy facade comparison on KITTI '+seq],cwd=W,check=True);backup(seq+'_raw')
        require({p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'Input changed before cleanup');shutil.rmtree(raw)
        write(target/'cleanup.json',dict(sequence=seq,owned_input_dir=str(raw),hashes_verified_before_removal=True,input_dir_exists_after=raw.exists(),removed_files=len(inputs)));subprocess.run(['git','add','-f',str(target/'cleanup.json')],cwd=W,check=True);subprocess.run(['git','commit','-m','Record verified official scan cleanup for KITTI '+seq],cwd=W,check=True);backup(seq)
        state['completed'].append(seq);state.update(status='committed',last_commit=git('rev-parse','HEAD'));write(R/'active.json',state);shutil.rmtree(stage)
    state.update(status='complete',sequence=None);write(R/'active.json',state);write(R/'collection_exit.json',dict(exit_code=0,completed=state['completed']))

if __name__=='__main__':
    lock=R/'collector.lock'
    with os.fdopen(os.open(lock,os.O_CREAT|os.O_EXCL|os.O_WRONLY,0o600),'w') as stream:stream.write(str(os.getpid())+'\n')
    try:main()
    except BaseException as error:write(R/'failure.json',dict(exit_code=1,error=str(error),traceback=traceback.format_exc()));raise
    finally:lock.unlink()
