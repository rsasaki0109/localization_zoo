from pathlib import Path
import hashlib,json,subprocess,time,traceback
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');tag=json.loads((r/'final_source_ready.json').read_text())['tag'];out=r/'release_final_validation';out.mkdir(exist_ok=False)
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip();records=[]
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','MPLCONFIGDIR=/tmp/localization-zoo-mpl-paper-speed','-w',str(repo),'localization-zoo-dev','/opt/localization-venv/bin/python']
try:
 assert json.loads((r/'final_comparison_ready.json').read_text())['exit_code']==0
 assert json.loads((r/(tag+'_final_mesh_profile_exit.json')).read_text())['exit_code']==0
 commands=[('source_verification',['git','diff','--exit-code',json.loads((r/'release_source_ready.json').read_text())['source_commit'],'--','papers/*/src/*','papers/*/include/*','papers/*/test/*','papers/*/CMakeLists.txt','evaluation/src/*','CMakeLists.txt','evaluation/CMakeLists.txt']),('refresh_study_docs',docker+['evaluation/scripts/refresh_study_docs.py']),('generate_full_variant_table',docker+['evaluation/scripts/generate_full_variant_table.py']),('generate_paper_ratio_table',docker+['evaluation/scripts/generate_paper_ratio_table.py']),('python_unittest',docker+['-m','unittest','discover','-s','tests']),('validate_showcase',docker+['evaluation/scripts/validate_showcase.py']),('generate_leaderboard_check',docker+['evaluation/scripts/generate_leaderboard.py','--check']),('generate_paper_ready_evidence_check',docker+['evaluation/scripts/generate_paper_ready_evidence.py','--check']),('whitespace',['git','diff','--check'])]
 for name,cmd in commands:
  started=time.time()
  with (out/(name+'.log')).open('w') as log:result=subprocess.run(cmd,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
  records.append({'label':name,'command':cmd,'cwd':str(repo),'checked_checkout_commit':source,'measured_candidate_source_commit':json.loads((r/'final_source_ready.json').read_text())['source_commit'],'release_source_commit':json.loads((r/'release_source_ready.json').read_text())['source_commit'],'started_unix':started,'finished_unix':time.time(),'exit_code':result.returncode});(out/'commands.json').write_text(json.dumps(records,indent=2)+'\n')
  if result.returncode:raise SystemExit(result.returncode)
 e=repo/'experiments/results/paper_pipeline_speed';history=json.loads((e/'historical_aggregate_sha256.json').read_text());changed=[name for name,digest in history.items() if hashlib.sha256((repo/name).read_bytes()).hexdigest()!=digest]
 base=json.loads((e/'protocol.json').read_text())['base_commit'];unchanged={}
 for name in ['evaluation/data/paper_reported_numbers.json','docs/assets/paper/paper_ratio_table.csv','README.md','docs/index.html']:
  old=subprocess.check_output(['git','show',base+':'+name],cwd=repo);unchanged[name]=old==(repo/name).read_bytes()
 (out/'preservation.json').write_text(json.dumps({'historical_aggregate_count':len(history),'changed_aggregates':changed,'unchanged_reference_and_published_tables':unchanged},indent=2)+'\n');assert not changed and all(unchanged.values())
 (out/'exit.json').write_text(json.dumps({'exit_code':0,'finished_unix':time.time()},indent=2)+'\n')
except BaseException as err:
 code=err.code if isinstance(err,SystemExit) and isinstance(err.code,int) else 1
 (out/'exit.json').write_text(json.dumps({'exit_code':code,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
