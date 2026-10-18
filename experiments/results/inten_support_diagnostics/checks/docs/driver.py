from pathlib import Path
import subprocess,sys,json,time,hashlib,shutil
repo=Path('/workspace/localization-env/inten-support/worktree')
r=Path('/workspace/localization-env/inten-support/checks')
assert not r.exists();r.mkdir()
prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','PYTHONDONTWRITEBYTECODE=1','-e','MPLCONFIGDIR=/tmp/localization-inten-support-plots','-w',str(repo),'localization-zoo-dev','/opt/localization-venv/bin/python']
records=[]
commands=[('refresh_study_docs',prefix+['evaluation/scripts/refresh_study_docs.py']),('generate_full_variant_table',prefix+['evaluation/scripts/generate_full_variant_table.py']),('generate_paper_ratio_table',prefix+['evaluation/scripts/generate_paper_ratio_table.py']),('python',prefix+['-m','unittest','discover','-s','tests']),('showcase',prefix+['evaluation/scripts/validate_showcase.py']),('leaderboard',prefix+['evaluation/scripts/generate_leaderboard.py','--check']),('paper_ready',prefix+['evaluation/scripts/generate_paper_ready_evidence.py','--check']),('preservation',[sys.executable,'experiments/results/shared_near_range/verify_preservation.py','--output',str(r/'preservation.json')]),('native_identity',['git','diff','--exit-code','7505a2914328d53ac9b319d5f8042189b378d457','--','common','evaluation/src','evaluation/CMakeLists.txt','papers/*/src','papers/*/include','papers/*/test','papers/*/CMakeLists.txt','CMakeLists.txt','python/src','ros2']),('original_runs_identity',['git','diff','--exit-code','7505a2914328d53ac9b319d5f8042189b378d457','--','experiments/results/runs','experiments/results/mesh_inten_kitti_coverage']),('branch_whitespace',['git','diff','--check','7505a2914328d53ac9b319d5f8042189b378d457'])]
for name,command in commands:
 start=time.time()
 with (r/(name+'.log')).open('w') as out:result=subprocess.run(command,cwd=repo,stdout=out,stderr=subprocess.STDOUT)
 records.append({'label':name,'command':command,'cwd':str(repo),'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'started_unix':start,'finished_unix':time.time(),'exit_code':result.returncode})
 (r/'commands.json').write_text(json.dumps(records,indent=2)+'\n');print(name,result.returncode,flush=True)
 if result.returncode:
  print((r/(name+'.log')).read_text()[-4000:]);sys.exit(result.returncode)
shutil.copy2(__file__,r/'driver.py')
shutil.copy2(repo/'experiments/results/inten_support_diagnostics/render_docs.py',r/'documentation_driver.py')
(r/'exit.json').write_text(json.dumps({'exit_code':0,'scope':'Offline diagnostic tooling and documentation checks; original native sources, results and published numbers preserved.'},indent=2)+'\n')
