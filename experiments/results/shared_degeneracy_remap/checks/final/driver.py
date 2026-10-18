from pathlib import Path
import json,subprocess,time,sys
repo=Path('/workspace/localization_zoo')
out=repo/'experiments/results/shared_degeneracy_remap/checks/final'
out.mkdir(parents=True,exist_ok=False)
head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1','-e','MPLCONFIGDIR=/tmp/localization-remap-plots','-w',str(repo),'localization-zoo-dev','/opt/localization-venv/bin/python']
commands=[
 ('refresh_docs',docker+['evaluation/scripts/refresh_study_docs.py']),
 ('full_variant_table',docker+['evaluation/scripts/generate_full_variant_table.py']),
 ('paper_ratio_table',docker+['evaluation/scripts/generate_paper_ratio_table.py']),
 ('preservation',[sys.executable,'experiments/results/shared_degeneracy_remap/verify_preservation.py','--output',str(out/'preservation.json')]),
 ('python_unittest',docker+['-m','unittest','discover','-s','tests','-v']),
 ('showcase',docker+['evaluation/scripts/validate_showcase.py']),
 ('leaderboard',docker+['evaluation/scripts/generate_leaderboard.py','--check']),
 ('paper_ready',docker+['evaluation/scripts/generate_paper_ready_evidence.py','--check']),
 ('whitespace',['git','diff','--check']),
]
records=[]
for label,command in commands:
 start=time.time()
 with (out/(label+'.log')).open('w') as log:
  result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
 record={'label':label,'command':command,'cwd':str(repo),'checked_checkout_commit':head,'started_unix':start,'finished_unix':time.time(),'exit_code':result.returncode}
 records.append(record)
 (out/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
 print(json.dumps({'label':label,'exit_code':result.returncode}),flush=True)
 if result.returncode:raise SystemExit(result.returncode)
