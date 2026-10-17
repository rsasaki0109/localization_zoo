from pathlib import Path
import json,subprocess,time
repo=Path('/workspace/localization_zoo');out=repo/'experiments/results/shared_coarse_start/checks/development'
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','0:0','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1','-w',str(repo),'localization-zoo-dev']
commands=[('build',docker+['cmake','--build','build','-j1','--target','pcd_dogfooding','test_initial_registration','test_imls_paper_odometry','test_suma_paper']),('focused_ctest',docker+['ctest','--test-dir','build','--output-on-failure','-R','^(initial_registration|test_imls_paper_odometry|test_suma_paper)$'])]
records=[]
for label,command in commands:
 start=time.time()
 with (out/(label+'.log')).open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
 record={'label':label,'command':command,'cwd':str(repo),'checkout_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'started_unix':start,'finished_unix':time.time(),'exit_code':result.returncode};records.append(record);(out/'commands.json').write_text(json.dumps(records,indent=2)+'\n');print(json.dumps(record),flush=True)
 if result.returncode:raise SystemExit(result.returncode)
