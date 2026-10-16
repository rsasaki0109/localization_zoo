from pathlib import Path
import json,subprocess,time
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');out=r/'python_docs_checks';out.mkdir(exist_ok=False)
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(repo),'localization-zoo-dev','/opt/localization-venv/bin/python']
commands=[('refresh_study_docs',['evaluation/scripts/refresh_study_docs.py']),('generate_full_variant_table',['evaluation/scripts/generate_full_variant_table.py']),('generate_paper_ratio_table',['evaluation/scripts/generate_paper_ratio_table.py']),('python_unittest',['-m','unittest','discover','-s','tests']),('validate_showcase',['evaluation/scripts/validate_showcase.py']),('generate_leaderboard_check',['evaluation/scripts/generate_leaderboard.py','--check']),('generate_paper_ready_evidence_check',['evaluation/scripts/generate_paper_ready_evidence.py','--check'])]
records=[];source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
for name,args in commands:
 started=time.time();cmd=docker+args
 with (out/(name+'.log')).open('w') as f:result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 records.append({'label':name,'command':cmd,'cwd':str(repo),'source_commit':source,'started_unix':started,'finished_unix':time.time(),'exit_code':result.returncode})
 (out/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
 if result.returncode:raise SystemExit(result.returncode)
(out/'exit.json').write_text(json.dumps({'exit_code':0,'source_commit':source,'finished_unix':time.time()},indent=2)+'\n')
