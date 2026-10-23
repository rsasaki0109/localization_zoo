import os,signal,json,subprocess,shutil,time,hashlib
from pathlib import Path
r=Path('experiments/results/inten_geometric_prior');out=r/'paused_attempts/merge_requested_00'
assert not out.exists()
expected={371089:'receipt.py',371179:'run_sequences.py',371181:'docker exec',371206:'/dev/inten-geometric-prior-20261023/build/stream_runner'}
rows={}
for pid,needle in expected.items():
 p=Path('/proc')/str(pid);cmd=(p/'cmdline').read_bytes().replace(b'\0',b' ').decode();assert needle in cmd
 rows[str(pid)]={'cmdline':cmd,'start_ticks':(p/'stat').read_text().rsplit(')',1)[1].split()[19]}
# Stop the spawning controller before discovering and stopping acquisition children.
os.kill(371179,signal.SIGSTOP)
processes={int(a):int(b) for a,b in (line.split() for line in subprocess.check_output(['ps','-eo','pid=,ppid='],text=True).splitlines())}
pids={371179}
while True:
 more={pid for pid,parent in processes.items() if parent in pids}-pids
 if not more:break
 pids.update(more)
for pid in sorted(pids|{371089}):
 try:os.kill(pid,signal.SIGSTOP)
 except ProcessLookupError:pass
subprocess.run(['docker','exec','localization-zoo-dev','kill','-STOP','20709'],check=True)
for pid in expected:
 status=(Path('/proc')/str(pid)/'status').read_text();state=next(l for l in status.splitlines() if l.startswith('State:'))
 assert 'T' in state;rows[str(pid)]['state_after']=state
shutil.copytree(r/'sequences/00',out)
(out/'native.jsonl.gz').rename(out/'native.jsonl.gz.partial')
for suffix in ('json','stdout','stderr'):
 shutil.copy2(r/f'commands/full_trial_extended_acquisition_wait.{suffix}',out/f'controller.{suffix}')
count=len((out/'inputs.jsonl').read_text().splitlines())
meta={'reason':'User requested closing this development increment and merging; trial remains incomplete.','paused_unix':time.time(),'mechanism':'SIGSTOP; native map/history remain only in current process memory. No successful termination is claimed.','controller_exit_code':None,'native_exit_code':None,'complete_input_records_saved':count,'processes':rows,'stopped_host_pids':sorted(pids|{371089,371206}),'resume_requirement':'Verify PIDs/start ticks and binary first, then SIGCONT native container PID 20709 and the recorded host processes. If any state is lost, preserve this attempt and restart sequence 00 from frame zero with the same settings. Never append a fresh estimator to this prefix.','files':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()}}
(out/'pause.json').write_text(json.dumps(meta,indent=2)+'\n')
print(json.dumps(meta,indent=2))

