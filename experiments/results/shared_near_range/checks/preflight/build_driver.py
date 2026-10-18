from pathlib import Path
import json,subprocess,time,shutil,hashlib
repo=Path('/workspace/localization_zoo'); E=repo/'experiments/results/shared_near_range'; R=Path('/workspace/localization-env/shared-near-range')
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','0:0','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(repo),'localization-zoo-dev']
receipts=[]
for name,argv in [('configure',['cmake','-S','.','-B','build','-DCMAKE_BUILD_TYPE=Release']),('build',['cmake','--build','build','-j1']),('ctest',['ctest','--test-dir','build','--output-on-failure'])]:
    command=prefix+argv;start=time.time()
    with (E/'checks/preflight'/f'{name}.log').open('w') as out:result=subprocess.run(command,cwd=repo,stdout=out,stderr=subprocess.STDOUT)
    receipts.append(dict(label=name,command=command,cwd=str(repo),source_commit=source,started_unix=start,finished_unix=time.time(),exit_code=result.returncode))
    (E/'checks/preflight/commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
    print(name,result.returncode,flush=True)
    if result.returncode:raise SystemExit(result.returncode)
shutil.copy2(repo/'build/evaluation/pcd_dogfooding',R/'candidate_pcd_dogfooding')
for name,commit in [('baseline','e13aab37258ce73c216302c1cc5b61a5d5385ed3'),('candidate',source)]:
    p=R/(name+'_pcd_dogfooding')
    (E/(name+'.json')).write_text(json.dumps(dict(source_commit=commit,binary=str(p),sha256=hashlib.sha256(p.read_bytes()).hexdigest()),indent=2)+'\n')
shutil.copy2(__file__,E/'checks/preflight/build_driver.py')
