from pathlib import Path
import hashlib,json,shutil
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/paper_pipeline_speed'
p=json.loads((r/'protocol.json').read_text())
for method in p['method_configs']:
 d=r/'runs'/f'baseline_profile_{method}'
 assert json.loads((d/'command.json').read_text())['exit_code']==0
 assert json.loads((d/'profile_command.json').read_text())['exit_code']==0
 assert json.loads((d/'summary.json').read_text())['evaluator_git_commit']==p['base_commit']
e.mkdir(parents=True,exist_ok=True)
for name in ['protocol.json','historical_aggregate_sha256.json','input_sha256.json','baseline_binary.json','environment.json','fetch_kitti07.json','fetch_kitti07.log','fetch_kitti07_failure_1.json','fetch_kitti07_failure_1.log','install_profiler.json','install_profiler.log','install_profiler_failure_1.json','install_profiler_failure_1.log','baseline_build.json','baseline_build.log']:
 dst=e/name
 assert not dst.exists(),dst
 shutil.copy2(r/name,dst)
helpers={}
for name in ['prepare.py','run_baseline.py','capture_environment.py','import_profiles.py']:
 dst=e/'helpers'/name;dst.parent.mkdir(exist_ok=True)
 shutil.copy2(r/name,dst)
 helpers[name]={'original_path':str(r/name),'sha256':hashlib.sha256((r/name).read_bytes()).hexdigest()}
for method in p['method_configs']:
 label=f'baseline_profile_{method}';dst=e/'runs'/label
 shutil.copytree(r/'runs'/label,dst)
 src=r/'baseline_worktree/dogfooding_results/paper_speed'/label/'dogfooding_results'
 pose={'mesh_loam':'Mesh_LOAM','imls_slam':'IMLS_SLAM','inten_loam':'InTEn_LOAM'}[method]
 shutil.copy2(src/(pose+'.txt'),dst/'poses.txt')
(e/'helper_sources.json').write_text(json.dumps(helpers,indent=2)+'\n')
cmd=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OMP_DYNAMIC=FALSE','-e','OPENBLAS_NUM_THREADS=1','localization-zoo-dev','/opt/localization-venv/bin/python',str(r/'capture_environment.py')]
(e/'environment_command.json').write_text(json.dumps({'command':cmd,'exit_code':0,'output':'environment.json','outer_cgroup':{'cpu_max':Path('/sys/fs/cgroup/cpu.max').read_text().strip(),'memory_max':Path('/sys/fs/cgroup/memory.max').read_text().strip()},'inner_cgroup_note':'Container has no local limit; the parent worker quota still applies.'},indent=2)+'\n')
print('Imported three completed CPU profiles, source/input hashes, environment and preparation receipts.')
