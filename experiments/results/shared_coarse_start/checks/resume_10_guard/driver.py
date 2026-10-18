"""Preserve and resume only a completed fetch failure; never resume native artifacts."""
from pathlib import Path
import argparse,hashlib,json,re,shutil,subprocess,time
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sequence',required=True,choices=[f'{i:02d}' for i in range(11)]);args=p.parse_args();seq=args.sequence
repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/shared_coarse_start';r=Path('/workspace/localization-env/shared-coarse-start')
def read(path):return json.loads(path.read_text())
def write(path,payload):path.write_text(json.dumps(payload,indent=2)+'\n')
def checked(command):return subprocess.run(command,cwd=repo,check=True)
assert subprocess.run(['git','diff','--cached','--quiet'],cwd=repo).returncode==0
active_bytes=(r/'active.json').read_bytes();a=json.loads(active_bytes);final=read(r/'run_exit.json');launcher=read(r/'latest_launcher.json')
assert final=={'exit_code':1,'error':'fetch failed after bounded retries; all failed receipts retained','sequence':seq}
assert a['status']=='failed' and a['sequence']==seq and not a['runners']
status=subprocess.run(['ps','-p',str(launcher['pid']),'-o','stat='],capture_output=True,text=True)
assert status.returncode==1 or status.stdout.strip().startswith('Z')
folder=e/'sequences'/seq
assert all(not(folder/n).exists() for n in ['inputs.json','audit','raw_control','comparison.json'])
assert not(e/'benchmarks/runs'/f'suma_kitti_seq_{seq}_full_shared_coarse_start_matrix').exists()
receipts=sorted((folder/'fetch_attempts').glob('*/fetch_command.json'));assert receipts
for path in receipts:
 receipt=read(path);assert receipt['exit_code']==1
 assert receipt['command'][1]=='evaluation/scripts/fetch_kitti_odometry_sequences.py'
 assert receipt['command'][receipt['command'].index('--sequence')+1]==seq
 assert 'EOFError: Range response ended' in (path.parent/'fetch.log').read_text()
number=1
while (e/'attempts'/f'fetch_{seq}_failure{("_retry"+str(number)) if number>1 else ""}.json').exists():number+=1
suffix='_retry'+str(number) if number>1 else ''
failure=e/'attempts'/f'fetch_{seq}_failure{suffix}.json';guard=e/'checks'/f'resume_{seq}{suffix}_guard';guard.mkdir(parents=True,exist_ok=False)
raw=r/'kitti/dataset/sequences'/seq/'velodyne';assert raw.is_dir() and not raw.is_symlink();bins=list(raw.glob('*.bin'))
write(failure,{'failed_status':final,'failed_active_state':a,'launcher':launcher,'recorded_at_unix':time.time(),'controller_confirmed_not_running':True,'controller_process_status':status.stdout.strip(),'retained_complete_bins':len(bins),'retained_complete_bin_bytes':sum(x.stat().st_size for x in bins),'scope':'Official Range EOF errors before native artifacts. Frozen source, binaries and native arguments are unchanged.'})
command=['/workspace/localization-env/host-venv/bin/python',str(e/'run_all.py'),'--resume-from',seq,'--check-resume-only'];started=time.time()
with (guard/'execution.log').open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
write(guard/'command.json',{'command':command,'cwd':str(repo),'started_unix':started,'finished_unix':time.time(),'actual_exit_code':result.returncode,'active_state_unchanged':active_bytes==(r/'active.json').read_bytes()})
assert result.returncode==0 and active_bytes==(r/'active.json').read_bytes()
source=read(e/'candidate.json')['source_commit'];command=['git','diff','--exit-code',source,'--','common','evaluation/src','papers/*/src','papers/*/include','papers/*/test','papers/*/CMakeLists.txt','CMakeLists.txt','ros2/localization_zoo_ros/CMakeLists.txt']
with (guard/'source_identity.log').open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
hashes={name:hashlib.sha256((r/(name+'_pcd_dogfooding')).read_bytes()).hexdigest() for name in ['candidate','baseline']}
for name,value in hashes.items():assert value==read(e/(name+'.json'))['sha256']
write(guard/'source_identity.json',{'command':command,'actual_exit_code':result.returncode,'binary_sha256':hashes});assert result.returncode==0
shutil.copyfile(Path(__file__),guard/'driver.py')
checked(['git','add','-f',str(failure),str(folder/'fetch_attempts'),str(guard)])
checked(['git','commit','-m',f'Preserve KITTI {seq} fetch failures and validate the read-only continuation{suffix}'])
assert final==read(r/'run_exit.json') and active_bytes==(r/'active.json').read_bytes()
archive=max([int(re.fullmatch(r'run_exit_attempt(\d+)\.json',x.name).group(1)) for x in r.glob('run_exit_attempt*.json')],default=0)+1
for src,dst in [('run_exit.json',f'run_exit_attempt{archive}.json'),('active.json',f'active_attempt{archive}.json'),('latest_launcher.json',f'launcher_attempt{archive}.json')]:
 assert not(r/dst).exists();shutil.copyfile(r/src,r/dst)
(r/'run_exit.json').unlink()
log=r/f'run_all_resume_{seq}{suffix}_driver.log';assert not log.exists()
command=['/workspace/localization-env/host-venv/bin/python',str(e/'run_all.py'),'--resume-from',seq]
head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip();started=time.time()
with log.open('w') as stream:process=subprocess.Popen(command,cwd=repo,stdin=subprocess.DEVNULL,stdout=stream,stderr=subprocess.STDOUT,start_new_session=True)
launch={'pid':process.pid,'command':command,'cwd':str(repo),'started_unix':started,'controller_checkout_commit':head,'implementation_commit':source,'reason':f'Continue official {seq} fetch after bounded Range EOF failures; previous committed checkpoints and absence of native artifacts validated read-only.','scope':'Same frozen native binaries and fixed method arguments. Reuse official CRC-checked complete files only; no native partial resume.'}
launch_path=e/'attempts'/f'resume_{seq}{suffix}_launch.json';assert not launch_path.exists()
for path in [launch_path,r/'latest_launcher.json',r/f'resume_{seq}{suffix}_launcher.json']:write(path,launch)
task=r/'task-state.json';s=read(task);s.update({'driver_pid':process.pid,'driver_log':str(log),'latest_launcher':str(r/'latest_launcher.json'),'latest_native_state':f'Official {seq} fetch continuation launched after committed failure/guard. Prior completed: {a["completed"]}. Same source/binaries/arguments. Health state authoritative.','latest_fetch_recovery':str(failure.relative_to(repo)),'recovery_helper':'R/recover_fetch.py; each executed copy is retained in the new guard directory. Validates and commits failure/guard before state archive and launch. Native artifacts are refused.'});write(task,s)
checked(['git','add',str(launch_path)]);checked(['git','commit','-m',f'Record guarded KITTI {seq} official fetch continuation{suffix}'])
print(json.dumps(launch))
