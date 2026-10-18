"""Validate isolated PR138 integration; keep the active primary checkout frozen."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import time

R=Path('/workspace/localization-env/shared-near-range/merge138')
REPO=R/'worktree'
PRIMARY=Path('/workspace/localization_zoo')
E=REPO/'experiments/results/shared_near_range'
OUT=R/'checks'
OUT.mkdir()
BASE='fff8cfa76072a8d975bcccc1cf4aae390bcb6b4e'
OLD='c3698e6700199b352d846f70a6580ad007ea35cc'
PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',MPLCONFIGDIR=str(R/'mplconfig'),XDG_CACHE_HOME=str(R/'xdgcache'),PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'])
HEAD=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
records=[]

def write(name,value):
    (OUT/name).write_text(json.dumps(value,indent=2)+'\n')

def run(label,cmd,cwd=REPO):
    start=time.time()
    with (OUT/(label+'.log')).open('w') as log:
        p=subprocess.run(cmd,cwd=cwd,env=dict(os.environ,**ENV),stdout=log,stderr=subprocess.STDOUT)
    records.append(dict(label=label,command=cmd,cwd=str(cwd),environment_overrides=ENV,checked_checkout_commit=HEAD,started_unix=start,finished_unix=time.time(),exit_code=p.returncode))
    write('commands.json',records)
    print(label,p.returncode,flush=True)
    if p.returncode: raise RuntimeError(label+' failed')

try:
    run('native_main_identity',['git','diff','--exit-code',BASE,'--','common','evaluation/src','evaluation/scripts','evaluation/CMakeLists.txt',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',':(glob)papers/*/CMakeLists.txt','CMakeLists.txt','python','ros2'])
    names=['finish_when_complete.py','finish_docs.py','support.py','report.py','verify_preservation.py','run_all.py','protocol.json','selection.json','baseline.json','candidate.json','native_wrapper.py','recover_interrupted05.py','recovery05_protocol.json','diagnose_saved02.py']
    paths=[str(E/name) for name in names]+[str(E/name) for name in ['sequences','checkpoints','diagnostics']]+['experiments/results/runs','experiments/results/mesh_inten_kitti_coverage']
    run('historical_study_unchanged',['git','diff','--exit-code',OLD,'--',*paths])
    for name in ['refresh_study_docs','generate_full_variant_table','generate_paper_ratio_table']:
        run(name,[PY,'evaluation/scripts/'+name+'.py'])
    run('python_regressions',[PY,'-m','unittest','discover','-s','tests'])
    for name,flags in [('validate_showcase',[]),('generate_leaderboard',['--check']),('generate_paper_ready_evidence',['--check'])]:
        run(name,[PY,'evaluation/scripts/'+name+'.py',*flags])
    hashes=json.loads((E/'historical_aggregate_sha256.json').read_text())
    changed=[name for name,value in hashes.items() if hashlib.sha256((REPO/name).read_bytes()).hexdigest()!=value]
    published={name:subprocess.check_output(['git','show',BASE+':'+name],cwd=REPO)==(REPO/name).read_bytes() for name in ['README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json']}
    if changed or not all(published.values()): raise RuntimeError('Historical publication modified')
    blocks=[]
    for method in ['aloam','ct_icp','floam','lego_loam','mesh_loam','vlom']:
        path='papers/'+method+'/README.md'
        before=subprocess.check_output(['git','show',OLD+':'+path],cwd=REPO,text=True)
        now=(REPO/path).read_text()
        for match in re.finditer(r'<!-- (SHARED_NEAR_RANGE[A-Z0-9_]*) -->.*?<!-- /\1 -->',before,re.S):
            if match.group() not in now: raise RuntimeError('Historical method block changed: '+method)
        blocks.append(dict(method=method,all_old_shared_range_blocks_preserved=True))
    main_mesh=subprocess.check_output(['git','show',BASE+':papers/mesh_loam/README.md'],cwd=REPO,text=True)
    section='## Acceleration integration with shared components'
    if main_mesh[main_mesh.index(section):] not in (REPO/'papers/mesh_loam/README.md').read_text():
        raise RuntimeError('Merged Mesh acceleration compatibility section missing')
    write('preservation.json',dict(base=BASE,historical_aggregate_count=len(hashes),changed_aggregates=changed,published_files_identical=published,method_blocks=blocks,main_mesh_compatibility_section_preserved=True))
    primary_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=PRIMARY,text=True).strip()
    if primary_head!=OLD: raise RuntimeError('Frozen primary HEAD moved; inspect controller before continuing')
    run('primary_native_identity',['git','diff','--exit-code','0b079dbbcb7c6f58c1d8367ade881c15f003e525','--','common','evaluation/src','evaluation/CMakeLists.txt',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',':(glob)papers/*/CMakeLists.txt','CMakeLists.txt'],cwd=PRIMARY)
    pe=PRIMARY/'experiments/results/shared_near_range'
    sealed=json.loads((Path('/workspace/localization-env/shared-near-range')/'completion_state.json').read_text())['sealed_file_sha256']
    pins={name:hashlib.sha256((pe/name).read_bytes()).hexdigest()==value for name,value in sealed.items()}
    for name in ['recover_interrupted05.py','recovery05_protocol.json']:
        pins[name]=subprocess.check_output(['git','show',OLD+':'+(pe/name).relative_to(PRIMARY).as_posix()],cwd=PRIMARY)==(pe/name).read_bytes()
    binaries={}
    for name in ['baseline','candidate']:
        spec=json.loads((pe/(name+'.json')).read_text())
        binaries[name]=hashlib.sha256(Path(spec['binary']).read_bytes()).hexdigest()==spec['sha256']
    alive={str(pid):Path('/proc/'+str(pid)+'/cmdline').exists() for pid in [2309,2310]}
    if not all(pins.values()) or not all(binaries.values()) or not all(alive.values()): raise RuntimeError('Frozen source/binaries/processes changed')
    write('primary_integrity.json',dict(primary_head=primary_head,sealed_and_recovery_files_match=pins,binary_hashes_match=binaries,process_exists=alive,scope='Only hashes/source/status; no scan reads, new native run or active checkout mutation.'))
    run('branch_whitespace',['git','diff','--check',BASE])
    write('exit.json',dict(exit_code=0,checked_checkout_commit=HEAD))
except BaseException as error:
    write('exit.json',dict(exit_code=1,checked_checkout_commit=HEAD,error=str(error)))
    raise
