import sys
from pathlib import Path
repo=Path('/workspace/localization_zoo')
sys.path.insert(0,str(repo/'experiments/results/shared_near_range'))
from support import *
folder=E/'checks/checkpoint04'
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
run('whitespace',['git','diff','--check'],folder)
import shutil
shutil.copy2(__file__,folder/'driver.py')
