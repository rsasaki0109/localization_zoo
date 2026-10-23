"""Run full local contracts once and retain exact exits and unchanged benchmarks."""
from pathlib import Path
import concurrent.futures,hashlib,importlib.util,json,shutil,subprocess
E=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('rigidity_final_common',E/'common.py')
common=importlib.util.module_from_spec(spec);spec.loader.exec_module(common)
W,S,P,BASE=common.W,common.S,common.P,common.BASE
read,write,sha,git,run=common.read,common.write,common.sha,common.git,common.run

def main():
 stage=S/'final_checks';stage.mkdir();derived=S/'derived';derived.mkdir()
 prefix=common.PREFIX
 commands={
  'all_python':prefix+['-m','unittest','discover','-s','tests','-v'],
  'showcase':prefix+['evaluation/scripts/validate_showcase.py'],
  'leaderboard':prefix+['evaluation/scripts/generate_leaderboard.py','--check'],
  'paper_ready':prefix+['evaluation/scripts/generate_paper_ready_evidence.py','--check'],
  'full_variant':prefix+['evaluation/scripts/generate_full_variant_table.py','--output-dir',str(derived)],
  'paper_ratio':prefix+['evaluation/scripts/generate_paper_ratio_table.py','--output-dir',str(derived)],
  'case_docs':prefix+[str(E/'report_docs.py')],
  'whitespace':['git','diff','--check'],
  'runtime':prefix+[str(P/'local_runtime_info.py')],
 }
 before={str(p.relative_to(E)):sha(p) for p in (E/'cases.md',E/'locations.md.xz',E/'cases.summary.json.xz',E/'tables_manifest.json')}
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
  scopes=('papers/inten_loam','evaluation/src','evaluation/data','tests','experiments/results')
  excludes=(':(exclude)papers/inten_loam/README.md',':(exclude)tests/test_inten_local_rigidity.py',':(exclude)tests/test_inten_local_rigidity_tables.py',':(exclude)experiments/results/inten_local_rigidity')
  changed=git('diff','--name-only',BASE,'--',*scopes,*excludes);assert not changed,changed
  for name,digest in before.items():assert sha(E/name)==digest,name
  write(stage/'preservation.json',dict(exit_code=0,base=BASE,sha256=preserved,older_source_result_changes=changed,case_regeneration=before))
  codes['preservation']=0
 except Exception as error:
  write(stage/'preservation.json',dict(exit_code=1,error=str(error),base=BASE));codes['preservation']=1
 code=0 if all(v==0 for v in codes.values()) else 1
 write(stage/'result.json',dict(exit_code=code,commands=codes))
 common.module('rigidity_final_archive',P/'archive.py').archive(stage,E/'checks/final')
 return code
if __name__=='__main__':raise SystemExit(main())
