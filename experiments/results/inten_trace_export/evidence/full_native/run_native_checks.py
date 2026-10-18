from pathlib import Path
import json,subprocess,time
R=Path(__file__).resolve().parent
repo=R/'worktree'; out=R/'native_checks';out.mkdir()
(out/'tmp').mkdir()
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1','-e','TEST_TMPDIR='+str(out/'tmp'),'-w',str(repo),'localization-zoo-dev']
commands=[('source_guard',['git','diff','--exit-code',source,'--','evaluation','common','papers','CMakeLists.txt']),('configure',docker+['cmake','-S','.','-B','build','-DCMAKE_BUILD_TYPE=Release']),('build',docker+['cmake','--build','build','-j2']),('ctest',docker+['ctest','--test-dir','build','--output-on-failure'])]
receipts=[]
for name,cmd in commands:
 (R/'active.json').write_text(json.dumps(dict(status=name,source=source))+'\n')
 start=time.time()
 with (out/(name+'.log')).open('w') as f: result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 receipts.append(dict(name=name,command=cmd,cwd=str(repo),source_commit=source,exit_code=result.returncode,started_unix=start,finished_unix=time.time()))
 (out/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
 if result.returncode:
  (out/'exit.json').write_text(json.dumps(dict(exit_code=result.returncode,stage=name))+'\n')
  raise SystemExit(result.returncode)
(out/'exit.json').write_text(json.dumps(dict(exit_code=0,source_commit=source))+'\n')
(R/'active.json').write_text(json.dumps(dict(status='native_checks_complete',source=source))+'\n')
print('Native build and CTest passed')
