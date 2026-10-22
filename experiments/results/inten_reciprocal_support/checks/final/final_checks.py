from pathlib import Path
import sys,concurrent.futures,subprocess,shutil
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_reciprocal_support');sys.path.insert(0,str(E))
from common import *
stage=S/'final_checks_stage';stage.mkdir();derived=S/'derived';derived.mkdir()
shutil.copyfile(Path(__file__),stage/'final_checks.py');shutil.copyfile(S/'compare_cpu_reports.py',stage/'compare_cpu_reports.py')
prefix=['env','PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages','/opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3']
commands={
 'all_python':prefix+['-m','unittest','discover','-s','tests','-v'],
 'showcase':prefix+['evaluation/scripts/validate_showcase.py'],
 'leaderboard':prefix+['evaluation/scripts/generate_leaderboard.py','--check'],
 'paper_ready':prefix+['evaluation/scripts/generate_paper_ready_evidence.py','--check'],
 'full_variant':prefix+['evaluation/scripts/generate_full_variant_table.py','--output-dir',str(derived)],
 'paper_ratio':prefix+['evaluation/scripts/generate_paper_ratio_table.py','--output-dir',str(derived)],
 'case_docs':prefix+[str(E/'report_docs.py')],
 'whitespace':['git','diff','--check'],
}
for cpu in ('Haswell','Sandybridge'):
    commands[cpu]=['env','OPENBLAS_CORETYPE='+cpu,*prefix,str(E/'verify_portable.py'),'--output',str(stage/(cpu+'.json'))]
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
    futures={pool.submit(run,label,cmd,stage):label for label,cmd in commands.items()}
    codes={futures[f]:f.result() for f in concurrent.futures.as_completed(futures)}
if all(codes[cpu]==0 for cpu in ('Haswell','Sandybridge')):
    codes['cpu_compare']=run('cpu_compare',prefix+[str(S/'compare_cpu_reports.py')],stage)
base='bab6ec9e5833fd6f59c1dba2bfa19f358b02129a';preserved={}
try:
    for f in sorted(derived.iterdir()):
        target=W/'docs/assets/paper'/f.name
        assert target.read_bytes()==f.read_bytes(),str(target)
        preserved[str(target.relative_to(W))]=sha(target)
    for name in ('evaluation/data/paper_reported_numbers.json','evaluation/fixtures/mcd_kth_smoke_golden.json','docs/index.html','README.md',
      'evaluation/src/pcd_dogfooding.cpp','papers/inten_loam/src/inten_loam_paper.cpp','papers/inten_loam/include/inten_loam/inten_loam_paper.h',
      'papers/inten_loam/test/test_inten_loam_paper.cpp','papers/inten_loam/include/inten_loam/detail/feature_tree.h'):
        assert subprocess.check_output(['git','show',base+':'+name],cwd=W)==(W/name).read_bytes(),name
        preserved[name]=sha(W/name)
    changed=git('diff','--name-only',base,'--','experiments/results',':(exclude)experiments/results/inten_gt_input_audit',':(exclude)experiments/results/inten_reciprocal_support')
    assert not changed,changed
    write(stage/'canonical_preservation.json',dict(exit_code=0,base=base,sha256=preserved,old_experiment_changes=changed))
    codes['preservation']=0
except Exception as error:
    write(stage/'canonical_preservation.json',dict(exit_code=1,error=str(error),base=base));codes['preservation']=1
write(stage/'result.json',dict(exit_code=0 if all(v==0 for v in codes.values()) else 1,commands=codes))
module('reciprocal_final_checks_archive',E/'archive.py').archive(stage,E/'checks/final')
raise SystemExit(0 if all(v==0 for v in codes.values()) else 1)
