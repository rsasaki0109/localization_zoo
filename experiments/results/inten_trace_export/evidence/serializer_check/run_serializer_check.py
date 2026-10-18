from pathlib import Path
import subprocess,json,time
r=Path(__file__).resolve().parent;repo=r/'worktree';out=r/'serializer_check';out.mkdir();(out/'tmp').mkdir()
prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','TEST_TMPDIR='+str(out/'tmp'),'-w',str(repo),'localization-zoo-dev']
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
commands=[('compile',prefix+['g++','-std=c++17','-O3','-I/usr/include/eigen3','-Ipapers/inten_loam/include','evaluation/test/test_inten_trace_jsonl.cpp','-lgtest_main','-lgtest','-pthread','-o',str(out/'test_inten_trace_jsonl')]),('test',prefix+[str(out/'test_inten_trace_jsonl')])]
receipts=[]
for name,cmd in commands:
 start=time.time()
 with (out/(name+'.log')).open('w') as f:result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 receipts.append(dict(name=name,command=cmd,cwd=str(repo),source_commit=source,exit_code=result.returncode,started_unix=start,finished_unix=time.time()))
 (out/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
 if result.returncode:raise SystemExit(result.returncode)
print('Serializer checks passed')
