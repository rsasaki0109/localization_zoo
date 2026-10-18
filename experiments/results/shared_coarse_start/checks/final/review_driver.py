from pathlib import Path
import json,re,subprocess,sys,time
repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/shared_coarse_start';out=e/'checks/final';r=Path('/workspace/localization-env/shared-coarse-start');base='14cfe450b55cb495190140e1f1fcb2c4119c43c7'
head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
def committed(path,revision=head):return subprocess.check_output(['git','show',revision+':'+path],cwd=repo)
def block(text):return text.split('<!-- SHARED_REMAP_RESULTS:START -->',1)[1].split('<!-- SHARED_REMAP_RESULTS:END -->',1)[0]
assert block(committed('papers/suma/README.md',base).decode())==block((repo/'papers/suma/README.md').read_text())
generated=['docs/'+name+'.md' for name in ['decisions','experiments','interfaces','paper_assets','paper_captions','paper_comparison','paper_roadmap','paper_tracks','reproduction_status','variant_analysis']]+['experiments/results/index.json']
pattern=r'\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?\+00:00'
for name in generated:
 old=committed(name).decode().splitlines();new=(repo/name).read_text().splitlines();assert len(old)==len(new)
 changes=[(a,b) for a,b in zip(old,new) if a!=b];assert len(changes)==1
 assert all(re.sub(pattern,'<timestamp>',a)==re.sub(pattern,'<timestamp>',b) for a,b in changes)
for seq in [f'{i:02d}' for i in range(11)]:
 assert not(r/'kitti/dataset/sequences'/seq/'velodyne').exists()
 alias=r/'data/dogfooding_results'/f'kitti_seq_{seq}_full';assert not alias.exists() and not alias.is_symlink()
s=json.loads(committed('experiments/results/shared_coarse_start/report/summary.json'));assert s['complete'] and len(s['rows'])==11 and s['audit_preservation_summary']['cells']==22
assert all(value==0 for key,value in s['audit_preservation_summary'].items() if key.startswith('max_abs'))
commands=[]
for label,command in [('preservation_review',[sys.executable,str(e/'verify_preservation.py'),'--output',str(out/'preservation_review.json')]),('whitespace_review',['git','diff','--check'])]:
 started=time.time()
 with (out/(label+'.log')).open('w') as log:result=subprocess.run(command,cwd=repo,stdout=log,stderr=subprocess.STDOUT)
 commands.append({'label':label,'command':command,'cwd':str(repo),'started_unix':started,'finished_unix':time.time(),'actual_exit_code':result.returncode});assert result.returncode==0
(out/'review.json').write_text(json.dumps({'input_commit':head,'old_suma_remap_block_bytes_equal':True,'derived_changes_are_timestamps_only':generated,'all_owned_scan_directories_and_aliases_absent':True,'full_report_complete':True,'audit_cells':22,'commands':commands,'scope':'Final documentation formatting and preservation review; native source unchanged.'},indent=2)+'\n')
print(json.dumps({'review':'passed','audit_cells':22,'owned_input_directories_absent':11}))
