"""Run required local contracts, preserve exact exits and canonical results."""
from pathlib import Path
import concurrent.futures,hashlib,json,os,shutil,subprocess,sys
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information');sys.path.insert(0,str(E))
from common import *
stage=S/'final_checks_stage';stage.mkdir();derived=S/'derived';derived.mkdir()
shutil.copyfile(Path(__file__),stage/'final_checks.py')
shutil.copyfile(S/'runtime_info.py',stage/'runtime_info.py')
prefix=['env','TMPDIR='+str(S),'PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages','/opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3']
commands={
 'all_python':prefix+['-m','unittest','discover','-s','tests','-v'],
 'showcase':prefix+['evaluation/scripts/validate_showcase.py'],
 'leaderboard':prefix+['evaluation/scripts/generate_leaderboard.py','--check'],
 'paper_ready':prefix+['evaluation/scripts/generate_paper_ready_evidence.py','--check'],
 'full_variant':prefix+['evaluation/scripts/generate_full_variant_table.py','--output-dir',str(derived)],
 'paper_ratio':prefix+['evaluation/scripts/generate_paper_ratio_table.py','--output-dir',str(derived)],
 'case_docs':prefix+[str(E/'report_docs.py')],
 'whitespace':['git','diff','--check'],
 'runtime':prefix+[str(S/'runtime_info.py')],
}
before={str(p.relative_to(E)):sha(p) for p in (E/'cases.md',E/'cases.summary.json')}
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 futures={pool.submit(run,label,cmd,stage):label for label,cmd in commands.items()}
 codes={futures[f]:f.result() for f in concurrent.futures.as_completed(futures)}
preserved={}
try:
 for f in sorted(derived.iterdir()):
  target=W/'docs/assets/paper'/f.name
  assert target.read_bytes()==f.read_bytes(),str(target)
  preserved[str(target.relative_to(W))]=sha(target)
 for name in ('evaluation/data/paper_reported_numbers.json','evaluation/fixtures/mcd_kth_smoke_golden.json','docs/index.html','README.md','evaluation/src/pcd_dogfooding.cpp','evaluation/src/inten_trace_jsonl.h'):
  assert subprocess.check_output(['git','show',BASE+':'+name],cwd=W)==(W/name).read_bytes(),name
  preserved[name]=sha(W/name)
 unchanged_scopes=('papers/inten_loam', 'evaluation/src', 'evaluation/data', 'tests', 'experiments/results')
 exclusions=(':(exclude)papers/inten_loam/README.md',':(exclude)tests/test_inten_support_information.py',':(exclude)experiments/results/inten_support_information')
 changed=git('diff','--name-only',BASE,'--',*unchanged_scopes,*exclusions)
 assert not changed,changed
 for name,digest in before.items():assert sha(E/name)==digest,name
 write(stage/'canonical_preservation.json',dict(exit_code=0,base=BASE,sha256=preserved,all_older_result_and_source_changes=changed,case_regeneration=before))
 codes['preservation']=0
except Exception as error:
 write(stage/'canonical_preservation.json',dict(exit_code=1,error=str(error),base=BASE));codes['preservation']=1
write(stage/'result.json',dict(exit_code=0 if all(v==0 for v in codes.values()) else 1,commands=codes))
module('support_final_checks_archive',E/'archive.py').archive(stage,E/'checks/final')
raise SystemExit(0 if all(v==0 for v in codes.values()) else 1)
