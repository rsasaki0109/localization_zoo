"""Derive full-sequence performance evidence; publish tables only after commit."""
from pathlib import Path
import argparse,hashlib,json,subprocess
import numpy as np
ap=argparse.ArgumentParser();ap.add_argument('--tag',required=True);ap.add_argument('--render',action='store_true');a=ap.parse_args()
repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/paper_pipeline_speed'
protocol=json.loads((e/'protocol.json').read_text());tag=a.tag
comparison=e/'comparisons'/(tag+'.json')
def committed(path):
 rel=str(path.relative_to(repo))
 recorded=subprocess.check_output(['git','show','HEAD:'+rel],cwd=repo)
 assert recorded==path.read_bytes(),f'{rel}: commit execution/derived evidence before rendering numeric docs'
if not a.render:
 assert not comparison.exists(),'Preserve prior comparisons; choose the actual new source tag'
 metadata=json.loads((e/'candidates'/tag/'binary.json').read_text())
 history=json.loads((e/'historical_aggregate_sha256.json').read_text())
 assert all(hashlib.sha256((repo/name).read_bytes()).hexdigest()==value for name,value in history.items())
 old_paper=subprocess.check_output(['git','show',protocol['base_commit']+':evaluation/data/paper_reported_numbers.json'],cwd=repo)
 assert old_paper==(repo/'evaluation/data/paper_reported_numbers.json').read_bytes()
 input_hashes=json.loads((e/'input_sha256.json').read_text())
 assert list(sorted(input_hashes))==[f'{i:06}.bin' for i in range(protocol['accuracy_window_frames'])]
 methods={}
 for method,cfg in protocol['method_configs'].items():
  runs={}
  for mode,label,source,flags in [('baseline',tag+'_control_full_'+method,protocol['base_commit'],cfg['args']),('accelerated',tag+'_accelerated_full_'+method,metadata['source_commit'],cfg['args']+['--paper-pipeline-accelerated'])]:
   directory=e/'runs'/label
   for name in ['summary.json','command.json','poses.txt']:committed(directory/name)
   receipt=json.loads((directory/'command.json').read_text());summary=json.loads((directory/'summary.json').read_text())
   assert receipt['exit_code']==0 and not receipt['profiler_enabled']
   assert receipt['source_commit']==source==summary['evaluator_git_commit']
   binary_meta=json.loads((e/('baseline_binary.json' if mode=='baseline' else 'candidates/'+tag+'/binary.json')).read_text())
   assert receipt['binary_sha256']==binary_meta['binary_sha256']
   assert len(summary['methods'])==1 and summary['num_frames']==summary['total_pcd_frames']==1101
   row=summary['methods'][0];assert row['status']=='ok' and row['frames']==1101
   assert summary['association_mode']=='exact_frame_id'
   assert summary['gt_sha256']==hashlib.sha256((repo/protocol['method_configs'][method]['source_manifest']).parent.joinpath('reference_data/kitti_seq_07_full_gt.csv').read_bytes()).hexdigest()
   cmd=receipt['command'];assert cmd[cmd.index('--summary-json')+2:]==flags
   assert 'OMP_NUM_THREADS=4' in cmd and 'OMP_DYNAMIC=FALSE' in cmd and 'OPENBLAS_NUM_THREADS=1' in cmd
   assert not any(x.startswith(('CPUPROFILE=','LD_PRELOAD=')) for x in cmd)
   assert row['time_ms']>0 and row['kitti_rte_trans_pct'] is not None
   poses=np.loadtxt(directory/'poses.txt');assert poses.shape==(1101,12) and np.isfinite(poses).all()
   runs[mode]={'row':row,'poses':poses,'receipt':str((directory/'command.json').relative_to(repo))}
  before,after=runs['baseline'],runs['accelerated'];b,c=before['row'],after['row']
  prior=e/'runs'/('baseline_full_'+method);committed(prior/'summary.json');committed(prior/'poses.txt')
  prior_row=json.loads((prior/'summary.json').read_text())['methods'][0];prior_poses=np.loadtxt(prior/'poses.txt')
  assert np.array_equal(prior_poses,before['poses']),'Frozen binary full trajectory changed; diagnose before speed claims'
  old=json.loads((repo/'experiments/results'/Path(cfg['source_manifest']).name).read_text())
  selected=next(v for v in old['variants'] if v['id']==cfg['source_variant'])
  trans=(before['poses']-after['poses'])[:,[3,7,11]]
  methods[method]={'earlier_baseline_time_ms':prior_row['time_ms'],'earlier_baseline_to_paired_time_ratio':prior_row['time_ms']/b['time_ms'],'paired_control_saved_poses_match_earlier_baseline':True,'variant':cfg['source_variant'],'args':cfg['args'],'frames':1101,'baseline':b,'accelerated':c,'speedup':b['time_ms']/c['time_ms'],'speed_goal_3x':b['time_ms']/c['time_ms']>=3,'rte_equal_to_three_decimals':f"{b['kitti_rte_trans_pct']:.3f}"==f"{c['kitti_rte_trans_pct']:.3f}",'rte_delta_percentage_points':c['kitti_rte_trans_pct']-b['kitti_rte_trans_pct'],'pose_matrix_max_abs_delta':float(np.max(np.abs(before['poses']-after['poses']))),'max_translation_delta_m':float(np.max(np.linalg.norm(trans,axis=1))),'final_translation_delta_m':float(np.linalg.norm(trans[-1])),'historical_committed_rte_pct':selected['kitti_rte_trans_pct'],'receipts':[before['receipt'],after['receipt']]}
 payload={'schema_version':1,'baseline_source_commit':protocol['base_commit'],'accelerated_source_commit':metadata['source_commit'],'threads':4,'development_sequence':'KITTI07, including the first120 profiling/screening frames; full07 is not heldout','saved_pose_significant_digits':9,'profiler_used_for_speed_comparison':False,'primary_speed_reference':'Frozen-source full control immediately before each accelerated method; earlier baseline retained separately due host timing variation','historical_aggregates_unchanged':len(history),'paper_reference_data_unchanged':True,'methods':methods,'all_speed_goals_reached':all(v['speed_goal_3x'] for v in methods.values()),'all_rte_equal_to_three_decimals':all(v['rte_equal_to_three_decimals'] for v in methods.values())}
 comparison.parent.mkdir(exist_ok=True);comparison.write_text(json.dumps(payload,indent=2)+'\n')
 print(json.dumps(payload,indent=2))
else:
 committed(comparison);s=json.loads(comparison.read_text())
 for method,row in s['methods'].items():
  path=repo/'papers'/method/'README.md';text=path.read_text()
  header='## Optional execution acceleration';start=text.index(header)
  marker='[Execution protocol and CPU profiles]';cut=text.index(marker,start)
  before=row['baseline'];after=row['accelerated']
  lines=['[Execution protocol and CPU profiles](../../experiments/results/paper_pipeline_speed/README.md)',f"and [full comparison](../../experiments/results/paper_pipeline_speed/comparisons/{tag}.json).",'',
   'Full KITTI 07, identical selected registration arguments, four OpenMP workers,',
   'one method at a time on the same recorded host; the profiler is disabled:', '',
   '| Execution | Time [s] | FPS | Official RTE [%] | ATE [m] |',
   '|---|---:|---:|---:|---:|',
   f"| Paired frozen control | {before['time_ms']/1000:.2f} | {before['fps']:.3f} | {before['kitti_rte_trans_pct']:.6f} | {before['ate_m']:.6f} |",
   f"| Opt-in acceleration | {after['time_ms']/1000:.2f} | {after['fps']:.3f} | {after['kitti_rte_trans_pct']:.6f} | {after['ate_m']:.6f} |",'',
   f"Measured speedup: **{row['speedup']:.2f}x**. Maximum translation difference from the",f"paired frozen control: {row['max_translation_delta_m']:.9f} m.",
   'Pose differences use the saved KITTI matrices (nine significant digits).']
  if row['rte_equal_to_three_decimals']:lines+=['Official RTE agrees to the third decimal.']
  else:lines+=['Official RTE changes at the third decimal; the difference requires the diagnosis below.']
  if not row['speed_goal_3x']:lines+=['','**Negative result:** this attempt does not meet the roadmap speed target.','The option remains off by default; the roadmap item is still open.']
  lines+=['','KITTI 07 guided implementation and is not held out. This comparison uses a',
          'frozen-source full control run immediately before acceleration on the same host',
          'and thread count. Historical',
          'Table 6 results retain their original evidence and are not replaced.','']
  path.write_text(text[:cut]+'\n'.join(lines))
 print('Rendered method tables from committed comparison evidence.')
