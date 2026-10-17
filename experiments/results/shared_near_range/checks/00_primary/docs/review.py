from pathlib import Path
import json,subprocess,sys
E=Path(__file__).resolve().parents[3];sys.path.insert(0,str(E))
from support import *
paths=['docs/decisions.md','docs/experiments.md','docs/interfaces.md','docs/paper_assets.md','docs/paper_captions.md','docs/paper_comparison.md','docs/paper_roadmap.md','docs/paper_tracks.md','docs/reproduction_status.md','docs/variant_analysis.md','experiments/results/index.json']
checks=[]
for path in paths:
 before=subprocess.check_output(['git','show',PROTOCOL['base_commit']+':'+path],cwd=REPO,text=True).splitlines();after=(REPO/path).read_text().splitlines();assert len(before)==len(after)
 changed=[(a,b) for a,b in zip(before,after) if a!=b]
 assert all((a.startswith('_Generated at ') and b.startswith('_Generated at ')) or (a.startswith('> Generated: ') and b.startswith('> Generated: ')) or (a.strip().startswith('"generated_at":') and b.strip().startswith('"generated_at":')) for a,b in changed)
 checks.append(dict(path=path,changed_lines=len(changed),timestamp_only=True))
pins={}
for name in ['protocol.json','baseline.json','candidate.json','native_wrapper.py','support.py','run_all.py','selection.json','historical_aggregate_sha256.json']:
 path=E/name;before=subprocess.check_output(['git','show','d843a8325ff7140460cde1daf62b0997b644d07e:'+path.relative_to(REPO).as_posix()],cwd=REPO);assert before==path.read_bytes();pins[name]=digest(path)
audit=json.loads((E/'sequences/00/audit/report.json').read_text());methods={'A-LOAM':'aloam','F-LOAM':'floam','LeGO-LOAM':'lego_loam','Mesh-LOAM':'mesh_loam','VLOM':'vlom'}
for method,module in methods.items():
 row=next(r for r in audit['rows'] if r['method']==method);text=(REPO/'papers'/module/'README.md').read_text().split('<!-- SHARED_NEAR_RANGE_00_PRIMARY -->')[1].split('<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->')[0]
 for key in ['stored_rpe_trans_pct','rerun_rpe_trans_pct','stored_ate_m','rerun_ate_m']:assert f'{row[key]:.6f}' in text
 assert row['variant_id'] in text and 'still running' in text
raw_log='experiments/results/shared_near_range/sequences/00/audit/replays/aloam_kitti_seq_00_full_transfer_matrix__kitti_default.log';fixture_log='experiments/results/shared_near_range/checks/fixture_parity/disabled/baseline/baseline.log'
code='common/include/common/range_filter.h';csv='experiments/results/shared_near_range/checkpoints/04/comparison.csv'
command=['git','check-attr','whitespace','--',raw_log,fixture_log,code,csv];attrs=subprocess.check_output(command,cwd=REPO,text=True)
assert raw_log+': whitespace: unset' in attrs and fixture_log+': whitespace: unset' in attrs
assert code+': whitespace: unspecified' in attrs and csv+': whitespace: unspecified' in attrs
assert not (R/'kitti/dataset/sequences/04/velodyne').exists()
assert (R/'kitti/dataset/sequences/00/velodyne').is_dir()
assert not (R/'kitti/dataset/sequences/01/velodyne').exists()
write(E/'checks/00_primary/docs/review.json',dict(generated_files=checks,unchanged_controller_pins=pins,method_numeric_blocks_match_committed_primary=True,attribute_command=command,attribute_output=attrs,cleanup_04_complete=True,input_00_retained_for_frozen_comparisons=True,input_01_absent=True,exit_code=0,scope='Partial00 documentation review; no frozen00 comparison or cleaned checkpoint completion claim.'))
print('Numeric documentation, controller pins, timestamp-only generators and raw-log attribute scope verified.')
