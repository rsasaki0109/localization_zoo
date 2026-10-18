from pathlib import Path
import argparse,json,shutil
ap=argparse.ArgumentParser();ap.add_argument('--tag',required=True);ap.add_argument('--method',choices=['mesh_loam','imls_slam','inten_loam'],required=True);a=ap.parse_args()
r=Path('/workspace/localization-env/paper-speed');e=Path('/workspace/localization_zoo/experiments/results/paper_pipeline_speed')
for mode in ['control','accelerated']:
 label=f'{a.tag}_{mode}_full_{a.method}';src=r/'runs'/label
 receipt=json.loads((src/'command.json').read_text());summary=json.loads((src/'summary.json').read_text());row=summary['methods'][0]
 assert receipt['exit_code']==0 and not receipt['profiler_enabled'] and row['frames']==1101 and row['status']=='ok' and (src/'poses.txt').exists()
 dst=e/'runs'/label
 for item in sorted(src.rglob('*')):
  if not item.is_file():continue
  target=dst/item.relative_to(src);target.parent.mkdir(parents=True,exist_ok=True)
  if target.exists():assert target.read_bytes()==item.read_bytes()
  else:shutil.copy2(item,target)
# Keep the exact executed helper without replacing older bodies.
shutil.copy2(Path(__file__),e/'helpers'/Path(__file__).name)
print('Imported completed full pair',a.tag,a.method)
