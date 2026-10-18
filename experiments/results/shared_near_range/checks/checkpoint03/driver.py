"""Validate the closed03 checkpoint/documents without touching live05."""
import importlib.util
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

REPO=Path('/workspace/localization_zoo')
sys.path.insert(0,str(REPO/'experiments/results/shared_near_range'))
from support import E,R,SOURCE,PROTOCOL,CONTAINER_PY,docker,run,write,digest
FOLDER=R/'checkpoint03_resume/docs_checks'
FOLDER.mkdir()
prefix=docker()+['env','MPLCONFIGDIR=/tmp/localization-range-plots','XDG_CACHE_HOME=/tmp/localization-range-cache',CONTAINER_PY]
for label,argv in [
 ('refresh_study_docs',['evaluation/scripts/refresh_study_docs.py']),
 ('generate_full_variant_table',['evaluation/scripts/generate_full_variant_table.py']),
 ('generate_paper_ratio_table',['evaluation/scripts/generate_paper_ratio_table.py']),
 ('python',['-m','unittest','discover','-s','tests']),
 ('showcase',['evaluation/scripts/validate_showcase.py']),
 ('leaderboard',['evaluation/scripts/generate_leaderboard.py','--check']),
 ('paper_ready',['evaluation/scripts/generate_paper_ready_evidence.py','--check'])
]:
    run(label,prefix+argv,FOLDER)
run('preservation',[sys.executable,str(E/'verify_preservation.py'),'--output',str(FOLDER/'preservation.json')],FOLDER)
run('closed_native_evidence_unchanged',['git','diff','--exit-code','eebaf3d7559a9e8ed98e59e93814f66838f6b5f6','--',str(E/'sequences/02'),str(E/'sequences/03')],FOLDER)
paths=[str(p) for root in [E/'diagnostics/02',E/'checkpoints/03'] for p in root.rglob('*') if p.is_file() and p.name!='README.md']
run('raw_diagnostics_checkpoint_unchanged',['git','diff','--exit-code','8f5841e3946995cedfa9c0e2fc484dbd2c02373e','--',*paths],FOLDER)
run('live_recovery_source_unchanged',['git','diff','--exit-code','d8278c749cfd1840ef7bc0efa20ae4de9df21c94','--',str(E/'recover_interrupted05.py'),str(E/'recovery05_protocol.json')],FOLDER)
run('full_native_source_identity',['git','diff','--exit-code',SOURCE,'--','common','evaluation/src','evaluation/CMakeLists.txt',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',':(glob)papers/*/CMakeLists.txt','CMakeLists.txt'],FOLDER)
spec=importlib.util.spec_from_file_location('sealed_finish_docs',E/'finish_docs.py')
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
checks=[]
for method in ['aloam','ct_icp','floam','lego_loam','mesh_loam']:
    path=REPO/'papers'/method/'README.md'
    before=subprocess.check_output(['git','show','8f5841e3946995cedfa9c0e2fc484dbd2c02373e:'+path.relative_to(REPO).as_posix()],cwd=REPO,text=True)
    now=path.read_text()
    for sequence in ['00','01']:
        pattern=r'<!-- SHARED_NEAR_RANGE_'+sequence+r'_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_'+sequence+r'_DIAGNOSTICS -->'
        old=re.findall(pattern,before,re.S)
        if any(block not in now for block in old): raise RuntimeError('Prior diagnosis block changed')
    stripped=module.strip_old_blocks(now)
    for sequence in ['02']+(['03'] if method=='mesh_loam' else []):
        pattern=r'<!-- SHARED_NEAR_RANGE_'+sequence+r'_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_'+sequence+r'_DIAGNOSTICS -->'
        block=re.search(pattern,now,re.S).group()
        if block not in stripped: raise RuntimeError('Completion renderer removes persistent diagnosis')
    checks.append(dict(method=method,prior_diagnostics_unchanged=True,new_diagnostics_survive_completion_renderer=True))
state=json.loads((R/'completion_state.json').read_text())
seals={name:digest(E/name)==expected for name,expected in state['sealed_file_sha256'].items()}
if not all(seals.values()): raise RuntimeError('Sealed source/config modified')
write(FOLDER/'diagnostic_preservation.json',dict(checks=checks,sealed_sha256_matches=seals,exit_code=0))
run('branch_whitespace',['git','diff','--check',PROTOCOL['base_commit']],FOLDER)
shutil.copy2(__file__,FOLDER/'driver.py')
shutil.copy2(R/'checkpoint03_resume/document.py',FOLDER/'documentation_driver.py')
write(FOLDER/'documentation_command.json',dict(command=['/workspace/localization-env/host-venv/bin/python',str(R/'checkpoint03_resume/document.py')],cwd=str(REPO),exit_code=0,stdout='Documented committed cleaned03 checkpoint, saved02 paths and recovery scope.\n',receipt_source='Actual Codex exec_command response before this check driver; no execution timing was captured.'))
write(FOLDER/'exit.json',dict(exit_code=0,scope='Committed cleaned04/00/01/02/03 and saved02 diagnostics only. Live05 and full177 completion excluded; native source0b retained.'))
