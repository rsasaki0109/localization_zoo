from pathlib import Path
import json,subprocess,sys
E=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(E))
from support import *
paths=['docs/decisions.md','docs/experiments.md','docs/interfaces.md','docs/paper_assets.md','docs/paper_captions.md','docs/paper_comparison.md','docs/paper_roadmap.md','docs/paper_tracks.md','docs/reproduction_status.md','docs/variant_analysis.md','experiments/results/index.json']
checks=[]
for path in paths:
 before=subprocess.check_output(['git','show',PROTOCOL['base_commit']+':'+path],cwd=REPO,text=True).splitlines()
 after=(REPO/path).read_text().splitlines()
 assert len(before)==len(after)
 changes=[(a,b) for a,b in zip(before,after) if a!=b]
 assert all((a.startswith('_Generated at ') and b.startswith('_Generated at ')) or (a.startswith('> Generated: ') and b.startswith('> Generated: ')) or (a.strip().startswith('"generated_at":') and b.strip().startswith('"generated_at":')) for a,b in changes)
 checks.append(dict(path=path,changed_lines=len(changes),only_generated_timestamp=True))
controller_pins={}
for name in ['protocol.json','baseline.json','candidate.json','native_wrapper.py','support.py','run_all.py','selection.json','historical_aggregate_sha256.json']:
 path=E/name;before=subprocess.check_output(['git','show','7458c24d16096cc305889ed0864a5ef8bfb6bdf4:'+path.relative_to(REPO).as_posix()],cwd=REPO)
 assert before==path.read_bytes()
 controller_pins[name]=digest(path)
assert not (R/'kitti/dataset/sequences/04/velodyne').exists()
assert not (R/'data/dogfooding_results/kitti_seq_04_full').is_symlink()
write(E/'checks/checkpoint04/review.json',dict(generated_file_checks=checks,controller_files_unchanged_from='7458c24d16096cc305889ed0864a5ef8bfb6bdf4',controller_file_sha256=controller_pins,completed_04_owned_inputs_absent=True,exit_code=0))
print('Timestamp-only derived changes, frozen controller pins and completed input cleanup verified.')
