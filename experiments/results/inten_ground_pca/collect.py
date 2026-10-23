"""Fixed full-sequence comparison; never retune or silently restart native runs."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import tempfile
import time
import traceback

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
PY = '/workspace/localization-env/host-venv/bin/python'
P = json.loads((E/'protocol.json').read_text())
PINS = json.loads((E/'binaries.json').read_text())
ENV = dict(os.environ, PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1',
           MPLCONFIGDIR=str(R/'matplotlib'), XDG_CACHE_HOME=str(R/'cache'))
BRANCH = 'codex/inten-ground-pca'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
    return mod


archival = module('row_archival', W/'experiments/results/inten_natural_traces/run_sequences.py')
restore = module('row_restore', W/'experiments/results/inten_natural_traces/diagnose.py').restore
build = module('row_build', E/'build.py')
sha, docker = archival.sha, build.docker


def read(path):
    return json.loads(Path(path).read_text())


def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temp = path.with_name(path.name+'.tmp')
    temp.write_text(json.dumps(data, indent=2, allow_nan=False)+'\n'); temp.replace(path)


def git(*args):
    return subprocess.check_output(['git', *args], cwd=W, text=True).strip()


def require(ok, reason):
    if not ok:
        raise ValueError(reason)


def run(label, command, folder, allowed=(0,), payload=None, native_argv=None, binary=None):
    folder.mkdir(parents=True, exist_ok=True)
    receipt = folder/(label+'_command.json')
    require(not receipt.exists(), 'receipt already exists: '+str(receipt))
    started = time.time()
    outpath = folder/(label+'.jsonl' if payload is not None else label+'.log')
    with outpath.open('xb') as out, (folder/(label+'_stderr.log')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, input=payload, stdout=out, stderr=err)
    record = dict(command=command, cwd=str(W), source_commit=git('rev-parse','HEAD'),
        native_source_commit=P['native_source_commit'],
        started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
        environment=P['native_environment'])
    if native_argv is not None:
        record.update(native_argv=native_argv, native_cwd=str(folder), binary_sha256=sha(binary),
                      binary_source_commit=P['parent_binary']['source_commit'] if str(binary)==P['parent_binary']['path'] else record['native_source_commit'])
    if native_argv is not None:
        record['native_source_commit'] = record['binary_source_commit']
    if payload is not None:
        record['stdin_sha256'] = hashlib.sha256(payload).hexdigest()
    write(receipt, record)
    print(label, folder.name, result.returncode, flush=True)
    require(result.returncode in allowed, label+' failed; retain evidence and stop')
    return result.returncode


def invariant():
    require(git('branch','--show-current') == BRANCH, 'wrong study branch')
    require(not git('diff','--name-only',P['native_source_commit'],'--',
        'common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',
        ':(glob)papers/*/test/**','CMakeLists.txt'), 'compiled native source changed')
    for name in ('collect.py','protocol.json','binaries.json','build.py','compatibility_probe.cpp','native_loader.h'):
        path = E/name
        require(subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)==path.read_bytes(), 'uncommitted study source '+name)
    require(not git('diff','--name-only',P['prior_aborted_trial_commit'],'--',P['prior_aborted_trial']), 'aborted trial changed')
    for item in (PINS['runner'], PINS['probe'], P['parent_binary']):
        require(sha(item['path']) == item['sha256'], 'pinned executable changed')
    for path, digest in PINS['linked_static_libraries'].items():
        require(sha(path) == digest, 'linked dependency changed')
    for seq, m in P['manifests'].items():
        require(sha(W/m['path']) == m['sha256'], 'manifest changed '+seq)
    require(not git('diff','--name-only',P['parent_commit'],'--',
        'experiments/results/inten_natural_traces','experiments/results/inten_seed_intervention',
        'experiments/results/inten_plane_correspondences','experiments/results/shared_near_range','experiments/results/inten_row_support','experiments/results/inten_mapped_prior_rigid','experiments/results/inten_translation_prior'), 'parent evidence changed')


def backup(label):
    for attempt in range(1,4):
        if run('push', ['git','push','origin','HEAD:refs/heads/'+BRANCH], R/'backups'/label/str(attempt), allowed=(0,1,128)) == 0:
            return
    raise RuntimeError('Normal backup failed; stop before deletion/next acquisition')


def native(mode, stage, raw, gt, args, binary, trace=True):
    folder = stage/mode
    folder.mkdir()
    argv = [str(binary),str(raw),str(gt),'--methods','inten_loam','--summary-json',str(folder/'summary.json'),
            *args]
    if trace: argv += ['--inten-loam-paper-trace-jsonl',str(folder/'trace.jsonl')]
    run('native', docker(folder)+argv, folder, native_argv=argv, binary=binary)


def same_legacy(stage):
    folders = [stage/name for name in ('baseline','parent_compatibility')]
    data = [read(f/'summary.json') for f in folders]
    fields = ('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
    require(all(data[0]['methods'][0].get(k)==data[1]['methods'][0].get(k) for k in fields), 'full07 legacy accuracy/frame drift')
    names = sorted(p.name for p in (folders[0]/'dogfooding_results').glob('*.txt'))
    require(names and names==sorted(p.name for p in (folders[1]/'dogfooding_results').glob('*.txt')), 'legacy artifacts differ')
    for name in names:
        require((folders[0]/'dogfooding_results'/name).read_bytes()==(folders[1]/'dogfooding_results'/name).read_bytes(), 'legacy saved pose/GT drift')
    lines = 0
    with (folders[0]/'trace.jsonl').open('rb') as a, (folders[1]/'trace.jsonl').open('rb') as b:
        ha, hb = json.loads(next(a)), json.loads(next(b))
        require(ha['params']==hb['params'] and ha['loader']==hb['loader'], 'legacy effective settings differ')
        while True:
            x,y = a.readline(),b.readline()
            require(x==y, 'legacy passive trace body drift')
            if not x: break
            lines += 1
    write(stage/'legacy_compatibility.json',dict(exit_code=0,fields_equal=list(fields),saved_pose_gt_bytes_equal=names,
        trace_body_lines_byte_equal=lines,scope='Full07 accelerated default-off behavior against immutable parent runner'))


def default_pair_compatibility(seq, stage, raw):
    if seq not in ('01','02','10'):
        return
    parent = W/'experiments/results/inten_seed_intervention/sequences'/seq
    cases = read(parent/'cases.json')['cases']
    lines=[]
    for case in cases:
        i = case['frame']; values = ' '.join(format(v,'.17g') for v in case['seeds']['natural'])
        lines.append(f"{case['id']} natural {raw/(f'{i-1:06d}.bin')} {raw/(f'{i:06d}.bin')} {values}")
    folder=stage/'default_pair_compatibility';folder.mkdir()
    payload=('\n'.join(lines)+'\n').encode();(folder/'input.txt').write_bytes(payload)
    run('probe',docker(folder)+[PINS['probe']['path']],folder,payload=payload)
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp=Path(temp);restore(parent,temp)
        expected={r['id']:r for r in (json.loads(l) for l in (temp/'native.stdout').read_text().splitlines()) if r['mode']=='natural'}
    actual=[json.loads(l) for l in (folder/'probe.jsonl').read_text().splitlines()]
    require(len(actual)==len(cases), 'missing default-off compatibility pair')
    maxima=dict(output=0.,H=0.,g=0.)
    for row,case in zip(actual,cases):
        old=expected[case['id']]
        require(row['id']==old['id'] and row['seed']==old['seed'] and row['features']==old['features'], 'default-off pair identity/features changed')
        pairs=[('output',row['output'],old['output'])]
        for now,before in zip(row['first_systems'],old['first_systems']):
            require(now['n']==before['n'],'default-off class count changed')
            pairs += [(k,now[k],before[k]) for k in ('H','g')]
        for key,a,b in pairs:
            a,b=np.asarray(a),np.asarray(b);maxima[key]=max(maxima[key],float(np.max(np.abs(a-b))))
            require(np.allclose(a,b,atol=1e-8,rtol=1e-10),'default-off pair output/system drift')
    write(folder/'result.json',dict(exit_code=0,pairs=len(cases),max_absolute_differences=maxima,class_counts_exact=True,
        scope='Default-off, unaccelerated natural-seed fixed-pair controls against parent native execution, not full-sequence accuracy scores'))


def analyze(seq, stage, expected_inputs):
    configurations={}
    for mode in ('baseline','candidate'):
        folder=stage/mode
        run('validate',[PY,'evaluation/scripts/check_inten_trace.py','--trace',str(folder/'trace.jsonl'),
            '--summary',str(folder/'summary.json'),'--receipt',str(folder/'native_command.json'),
            '--output',str(folder/'trace_validation.json')],folder/'analysis')
        validation=read(folder/'trace_validation.json')
        require(validation['skipped_frames']==0 and validation['nonfinite_values']==0 and
            validation['processed_frames']==expected_inputs['scan_frames'],'incomplete/nonfinite native traversal')
        with (folder/'trace.jsonl').open() as stream:header=json.loads(next(stream))
        require(header['params']['accelerated'] is True,'acceleration policy changed')
        require(not header['params'].get('s2s_row_support',False),'row support must stay off')
        require(header['params'].get('s2s_mapped_prior',False) is True,'mapped-motion seed must stay enabled')
        require(header['params'].get('s2s_translation_prior_ratio',0)==.03,'translation prior must stay on')
        require(header['params'].get('s2s_ground_pca',False)==(mode=='candidate'),'ground PCA not effective')
        configurations[mode]=dict(params=header['params'],loader=header['loader'])
        summary=read(folder/'summary.json');evaluated=Path(summary['artifacts']['evaluated_gt_path'])
        evaluated=evaluated if evaluated.is_absolute() else folder/evaluated
        run('steps',[PY,'evaluation/scripts/diagnose_kitti_steps.py','--estimate',str(folder/'dogfooding_results/InTEn_LOAM.txt'),
            '--gt',str(evaluated),'--summary',str(folder/'summary.json'),'--method','InTEn-LOAM',
            '--output',str(folder/'motion_steps.json')],folder/'analysis')
    a,b=configurations['baseline'],configurations['candidate']
    baseline=dict(a['params']);candidate=dict(b['params']);baseline.pop('s2s_ground_pca',None);candidate.pop('s2s_ground_pca',None)
    require(baseline==candidate and a['loader']==b['loader'],'comparison changes more than ground PCA')
    write(stage/'effective_configuration.json',configurations)
    return configurations


def preflight():
    folder=R/'preflight';folder.mkdir()
    manifest=read(W/P['manifests']['07']['path'])
    args=next(v['args'] for v in manifest['variants'] if v['id']=='candidate')
    for mode,on in (('trace_off',False),('trace_on',True)):
        native(mode,folder,W/'evaluation/fixtures/mcd_kth_smoke',W/'evaluation/fixtures/mcd_kth_smoke_gt.csv',args,PINS['runner']['path'],trace=on)
    off,on=folder/'trace_off',folder/'trace_on'
    a,b=read(off/'summary.json'),read(on/'summary.json')
    keys=('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
    require(all(a['methods'][0].get(k)==b['methods'][0].get(k) for k in keys),'candidate CLI trace changes metrics')
    names=sorted(p.name for p in (off/'dogfooding_results').glob('*.txt'))
    require(names and all((off/'dogfooding_results'/n).read_bytes()==(on/'dogfooding_results'/n).read_bytes() for n in names),'candidate CLI trace changes pose bytes')
    with (on/'trace.jsonl').open() as stream:header=json.loads(next(stream))
    require(header['params'].get('s2s_ground_pca') is True and header['params'].get('s2s_translation_prior_ratio') == .03 and header['params'].get('s2s_mapped_prior') is True and header['params']['accelerated'] is True,'new option not effective in CLI')
    write(folder/'parity.json',dict(exit_code=0,fields_equal=list(keys),saved_pose_gt_bytes_equal=names,enabled_flag_effective=True))
    archival.archive(folder,E/'checks/cli_preflight')
    subprocess.run(['git','add','-f',str(E/'checks/cli_preflight')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Record candidate CLI trace parity before KITTI acquisition'],cwd=W,check=True)
    backup('preflight')


def main():
    require(not (E/'sequences').exists() and not (R/'staging').exists(),'fresh collection required; inspect any earlier attempt')
    invariant();state=dict(status='starting',completed=[],source_commit=git('rev-parse','HEAD'));write(R/'active.json',state)
    backup('source');preflight();canonical=None
    for seq in P['sequence_order']:
        invariant();stage=R/'staging'/seq;stage.mkdir(parents=True)
        state.update(status='fetching',sequence=seq);write(R/'active.json',state)
        for attempt in range(1,5):
            if run('fetch',[PY,'evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(R/'kitti'),
                '--sequence',seq,'--workers','4'],stage/'fetch'/str(attempt),allowed=(0,1))==0:break
        else:raise RuntimeError('Official acquisition failed; retain partial scans and all receipts')
        raw=R/'kitti/dataset/sequences'/seq/'velodyne';gt=W/f'experiments/reference_data/kitti_seq_{seq}_full_gt.csv'
        previous=read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json')
        inputs={p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}
        require(inputs==previous['files'] and sha(gt)==previous['gt_csv_sha256'],'official scan/GT hashes changed')
        write(stage/'inputs.json',dict(sequence=seq,files=inputs,gt_csv_sha256=sha(gt),scan_frames=previous['scan_frames'],
            owned_input_dir=str(raw),matches_previous_official_inputs=True))
        manifest=read(W/P['manifests'][seq]['path']);args={v['id']:v['args'] for v in manifest['variants']}
        state.update(status='native');write(R/'active.json',state)
        if seq=='07':
            parent_args=args['baseline'][:-2]  # Parent has no new override key.
            with ThreadPoolExecutor(max_workers=2) as pool:
                a=pool.submit(native,'baseline',stage,raw,gt,args['baseline'],PINS['runner']['path'])
                b=pool.submit(native,'parent_compatibility',stage,raw,gt,parent_args,P['parent_binary']['path'])
                a.result();b.result()
            same_legacy(stage)
            native('candidate',stage,raw,gt,args['candidate'],PINS['runner']['path'])
        else:
            with ThreadPoolExecutor(max_workers=2) as pool:
                futures=[pool.submit(native,mode,stage,raw,gt,args[mode],PINS['runner']['path']) for mode in ('baseline','candidate')]
                for future in futures:future.result()
        state.update(status='validating');write(R/'active.json',state)
        configurations=analyze(seq,stage,previous)
        if canonical is None:canonical=configurations
        else:require(canonical==configurations,'effective configuration changed between sequences')
        default_pair_compatibility(seq,stage,raw)
        invariant()
        require({p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'inputs changed')
        target=E/'sequences'/seq;archival.archive(stage,target)
        subprocess.run(['git','add','-f',str(target)],cwd=W,check=True)
        subprocess.run(['git','commit','-m','Record fixed full-sequence ground-PCA comparison on KITTI '+seq],cwd=W,check=True)
        backup(seq+'_raw')
        require({p.name:dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(raw.iterdir()) if p.is_file()}==inputs,'inputs changed before cleanup')
        shutil.rmtree(raw);write(target/'cleanup.json',dict(sequence=seq,owned_input_dir=str(raw),hashes_verified_before_removal=True,
            input_dir_exists_after=raw.exists(),removed_files=len(inputs)))
        subprocess.run(['git','add','-f',str(target/'cleanup.json')],cwd=W,check=True)
        subprocess.run(['git','commit','-m','Record verified official scan cleanup for KITTI '+seq],cwd=W,check=True)
        state['completed'].append(seq);state.update(status='committed',last_commit=git('rev-parse','HEAD'));write(R/'active.json',state)
        backup(seq);shutil.rmtree(stage)
    state.update(status='complete',sequence=None);write(R/'active.json',state);write(R/'collection_exit.json',dict(exit_code=0,completed=state['completed']))


if __name__=='__main__':
    lock=R/'collector.lock'
    with os.fdopen(os.open(lock,os.O_CREAT|os.O_EXCL|os.O_WRONLY,0o600),'w') as stream:stream.write(str(os.getpid())+'\n')
    try:main()
    except BaseException as error:
        write(R/'failure.json',dict(exit_code=1,error=str(error),traceback=traceback.format_exc()));raise
    finally:lock.unlink()
