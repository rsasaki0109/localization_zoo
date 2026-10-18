import sys,shutil
from pathlib import Path
repo=Path('/workspace/localization_zoo');E=repo/'experiments/results/shared_near_range'
sys.path.insert(0,str(E))
from support import *
folder=E/'checks/00_primary/docs'
prefix=docker()+['env','MPLCONFIGDIR=/tmp/localization-range-plots',CONTAINER_PY]
for name,argv in [
 ('refresh_study_docs',['evaluation/scripts/refresh_study_docs.py']),
 ('generate_full_variant_table',['evaluation/scripts/generate_full_variant_table.py']),
 ('generate_paper_ratio_table',['evaluation/scripts/generate_paper_ratio_table.py']),
 ('python',['-m','unittest','discover','-s','tests']),
 ('showcase',['evaluation/scripts/validate_showcase.py']),
 ('leaderboard',['evaluation/scripts/generate_leaderboard.py','--check']),
 ('paper_ready',['evaluation/scripts/generate_paper_ready_evidence.py','--check'])]:
 run(name,prefix+argv,folder)
run('preservation',[sys.executable,str(E/'verify_preservation.py'),'--output',str(folder/'preservation.json')],folder)
run('branch_whitespace',['git','diff','--check',PROTOCOL['base_commit']],folder)
run('primary_evidence_unchanged',['git','diff','--exit-code','485fa00601644731827dae601eb3b43c7543efeb','--',str(E/'sequences/00/audit'),str(E/'sequences/00/inputs.json')],folder)
shutil.copy2(__file__,folder/'driver.py')
shutil.copy2('/workspace/localization-env/shared-near-range/document_00_primary.py',folder/'documentation_driver.py')
