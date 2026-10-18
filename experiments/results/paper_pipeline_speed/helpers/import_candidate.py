from pathlib import Path
import argparse,hashlib,json,shutil
ap=argparse.ArgumentParser();ap.add_argument('--tag',required=True);ap.add_argument('--partial',action='store_true');a=ap.parse_args()
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/paper_pipeline_speed';tag=a.tag
meta=json.loads((r/(tag+'_binary.json')).read_text());c=e/'candidates'/tag;c.mkdir(parents=True,exist_ok=True)
def put(src,dst):
 dst.parent.mkdir(parents=True,exist_ok=True)
 if dst.exists():assert dst.read_bytes()==src.read_bytes(),f'Preserve conflicting evidence: {dst}'
 else:shutil.copy2(src,dst)
def tree(src,dst):
 for item in sorted(src.rglob('*')):
  if item.is_file():put(item,dst/item.relative_to(src))
put(r/(tag+'_binary.json'),c/'binary.json');tree(r/'checks'/tag,c/'checks')
for method in ['mesh_loam','imls_slam','inten_loam']:
 labels=[f'{tag}_{mode}_short_{method}' for mode in ['control','default','accelerated']]
 ready=all((r/'runs'/name/'command.json').exists() and json.loads((r/'runs'/name/'command.json').read_text())['exit_code']==0 and (r/'runs'/name/'poses.txt').exists() for name in labels)
 if not ready:
  assert a.partial,f'Incomplete screening: {method}'
  continue
 for name in labels:tree(r/'runs'/name,e/'runs'/name)
metrics=r/(tag+'_screening.json')
if metrics.exists():
 assert json.loads((r/(tag+'_screening_exit.json')).read_text())['exit_code']==0
 for name,dest in [(metrics,'screening.json'),(r/(tag+'_screening_exit.json'),'screening_exit.json')]:put(name,c/dest)
 phase='screening_all'
else:
 assert a.partial
 partial=json.loads((r/(tag+'_screening_partial.json')).read_text())
 phase='screening_'+'_'.join(partial)
 dst=c/(phase+'.json');body=(json.dumps(partial,indent=2)+'\n').encode()
 if dst.exists():assert dst.read_bytes()==body
 else:dst.write_bytes(body)
full=r/(tag+'_full_exit.json')
if full.exists() and json.loads(full.read_text())['exit_code']==0:
 for method in ['mesh_loam','imls_slam','inten_loam']:
  name=f'{tag}_accelerated_full_{method}';src=r/'runs'/name
  assert json.loads((src/'command.json').read_text())['exit_code']==0
  tree(src,e/'runs'/name)
 for source,dest in [(full,'full_exit.json'),(r/(tag+'_full_commands.json'),'full_commands.json')]:put(source,c/dest)
 phase='full'
# Preserve the transport failure, successful-step reuse and persistent launcher.
for name in [tag+'_candidate_exit.json',tag+'_screening_reused.json','resume_launcher.json']:
 src=r/name
 if src.exists():put(src,c/name)
helpers={}
for name in ['screen_candidate.py','resume_screen_candidate.py','run_full_candidate.py','import_candidate.py','compare_full.py','capture_baseline_flags.py']:
 src=r/name
 if not src.exists():continue
 sha=hashlib.sha256(src.read_bytes()).hexdigest();dst=e/'helpers'/name
 if dst.exists() and dst.read_bytes()!=src.read_bytes():dst=e/'helpers'/(src.stem+'_'+sha[:12]+src.suffix)
 put(src,dst);helpers[name]={'original_path':str(src),'sha256':sha,'archived_path':str(dst.relative_to(repo))}
(c/(phase+'_helper_sources.json')).write_text(json.dumps(helpers,indent=2)+'\n')
if (r/'baseline_target_flags.json').exists():put(r/'baseline_target_flags.json',e/'baseline_target_flags.json')
print('Imported candidate',tag,'phase',phase)
