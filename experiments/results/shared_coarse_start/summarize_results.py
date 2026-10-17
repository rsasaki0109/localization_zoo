"""Summarize only committed, completed fixed-setting experiment records.

Default execution requires all KITTI 00-10 sequences. --allow-partial creates an
explicitly incomplete interim report without a paper ratio or aggregate score.
"""
from pathlib import Path
from datetime import datetime,timezone
import argparse,csv,hashlib,json,math,subprocess,sys

E=Path(__file__).resolve().parent
REPO=E.parents[2]
EXPECTED=[f'{i:02d}' for i in range(11)]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--allow-partial',action='store_true')
    parser.add_argument('--output-dir',type=Path,required=True)
    args=parser.parse_args()
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
    sources={}

    def read(path,optional=False):
        relative=path.relative_to(REPO).as_posix()
        result=subprocess.run(['git','show',f'{revision}:{relative}'],cwd=REPO,capture_output=True)
        if result.returncode:
            if optional:return None
            raise ValueError(f'not committed at {revision}: {relative}')
        sources[relative]=hashlib.sha256(result.stdout).hexdigest()
        return json.loads(result.stdout)

    def local_summary(original):
        marker='/experiments/results/shared_coarse_start/'
        suffix=original.split(marker,1)[1]
        if '..' in Path(suffix).parts or Path(suffix).is_absolute():raise ValueError('invalid artifact path')
        return E/suffix

    protocol=read(E/'protocol.json')
    native=read(E/'candidate.json')
    rows=[];audit_rows=[];audit_official_checks=[];raw_preservation=[]
    for seq in EXPECTED:
        folder=E/'sequences'/seq
        comparison=read(folder/'comparison.json',optional=True)
        cleanup=read(folder/'cleanup.json',optional=True)
        if comparison is None or cleanup is None:continue
        assert cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after']
        assert comparison['source_commit']==native['source_commit']==protocol['implementation_commit']
        assert comparison['sequence']==seq and comparison['candidate_arguments']==protocol['suma_fixed_arguments']
        inputs=read(folder/'inputs.json')
        gt_relative=f'experiments/reference_data/kitti_seq_{seq}_full_gt.csv'
        gt_bytes=subprocess.check_output(['git','show',revision+':'+gt_relative],cwd=REPO)
        sources[gt_relative]=hashlib.sha256(gt_bytes).hexdigest()
        assert inputs['gt_csv_sha256']==sources[gt_relative], f'GT CSV changed: {seq}'
        previous_folder=REPO/'experiments/results/shared_degeneracy_remap/sequences'/seq
        previous_inputs=read(previous_folder/'inputs.json')
        assert previous_inputs['scan_frames']==inputs['scan_frames'] and previous_inputs['gt_csv_sha256']==inputs['gt_csv_sha256']
        bins=lambda value:{name:meta for name,meta in value['files'].items() if name.endswith('.bin')}
        assert bins(previous_inputs)==bins(inputs), f'official scan bytes differ from prior study: {seq}'
        previous=read(previous_folder/'comparison.json')['control']
        assert previous['status']=='ok'
        control,candidate=comparison['control'],comparison['candidate']
        assert control['frames']==candidate['frames']==inputs['scan_frames']==inputs['gt_frames']
        raw_keys=('frames','ate_m','rpe_trans_pct','rpe_rot_deg_per_m','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
        raw_deltas={key:abs(control[key]-previous[key]) for key in raw_keys}
        assert max(raw_deltas.values())<=1e-9, f'raw default control changed: {seq}: {raw_deltas}'
        raw_preservation.append({'sequence':seq,'reference_comparison':str((previous_folder/'comparison.json').relative_to(REPO)),
            'official_scan_bin_hashes_match':True,'absolute_metric_delta':raw_deltas})
        for data in (control,candidate):
            assert data['status']=='ok'
            for key in ('ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m'):
                assert isinstance(data[key],(int,float)) and math.isfinite(data[key]) and data[key]>=0
        assert 'opt-in initial coarse registrations=1 frames=1 distance_scale=5.000000 max_iterations=10 (not in the paper)' in candidate['note']
        for label,suffix in (('control',['--no-gt-seed','--suma-paper']),('candidate',protocol['suma_fixed_arguments'])):
            summary=local_summary(comparison[label+'_summary'])
            receipt=read(summary.with_name(summary.stem+'_native_command.json'))
            assert receipt['exit_code']==0 and receipt['binary_sha256']==native['sha256']
            command=receipt['command']
            assert len(command)==7+len(suffix) and command[3:6]==['--methods','suma','--summary-json']
            assert command[-len(suffix):]==suffix
            assert Path(command[1]).name==f'kitti_seq_{seq}_full'
            assert Path(command[2]).name==f'kitti_seq_{seq}_full_gt.csv'
            assert receipt['environment']=={'OMP_NUM_THREADS':'1','OPENBLAS_NUM_THREADS':'1'}
        candidate_diag=read(folder/'candidate_drift.json')
        control_diag=read(folder/'raw_control_drift.json')
        assert candidate_diag['parameters']==control_diag['parameters']=={
            'block_frames':100,'min_step_m':0.1,'relative_step_threshold':0.2}
        assert candidate_diag['frames']==control_diag['frames']==candidate['frames']
        assert candidate_diag['moving_step_count']==control_diag['moving_step_count']
        for data,diag in ((candidate,candidate_diag),(control,control_diag)):
            assert abs(data['kitti_rte_trans_pct']-diag['official_metrics_from_saved_poses']['kitti_rte_trans_pct'])<=0.001
        audit=read(folder/'audit/report.json')
        assert len(audit['rows'])==2 and {x['method'] for x in audit['rows']}=={'SuMa','IMLS-SLAM'}
        assert all(x['sequence']==seq and x['audit_status'] in ('ok','drift') for x in audit['rows'])
        for row in audit['rows']:
            aggregate=read(REPO/row['aggregate_path'])
            stored=next(v for v in aggregate['variants'] if v['id']==row['variant_id'])
            assert stored['args']==row['args']
            keys=('kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
            assert all(math.isfinite(row[k]) and math.isfinite(stored[k]) for k in keys)
            deltas={k:abs(row[k]-stored[k]) for k in keys}
            assert max(deltas.values())<=1e-9, f'official RTE drift: {seq} {row["method"]}: {deltas}'
            audit_official_checks.append({'sequence':seq,'method':row['method'],
                'aggregate_path':row['aggregate_path'],'variant_id':row['variant_id'],
                'stored':{k:stored[k] for k in keys},'rerun':{k:row[k] for k in keys},
                'absolute_delta':deltas})
        if seq=='04' or any(x['drifted'] for x in audit['rows']):
            frozen=read(folder/'frozen_comparison.json')
            assert all(all(value<=1e-9 for value in x['metric_abs_delta'].values()) for x in frozen)
        audit_rows.extend(audit['rows'])
        blocks=[b for b in candidate_diag['blocks'] if b['moving_step_count']>0
                and isinstance(b['block_relative_translation_error_pct'],(int,float))
                and math.isfinite(b['block_relative_translation_error_pct'])]
        worst=sorted(blocks,key=lambda x:x['block_relative_translation_error_pct'],reverse=True)[:3]
        def error_growth(diag):
            growing=[]
            for block in diag['blocks']:
                increase=block['position_error_end_m']-block['position_error_start_m']
                if math.isfinite(increase) and increase>0:
                    growing.append({**block,'position_error_increase_m':increase})
            return sorted(growing,key=lambda x:x['position_error_increase_m'],reverse=True)[:3]
        rows.append({'sequence':seq,'frames':candidate['frames'],
          'control_rte_pct':control['kitti_rte_trans_pct'],'candidate_rte_pct':candidate['kitti_rte_trans_pct'],
          'rte_difference_pp':candidate['kitti_rte_trans_pct']-control['kitti_rte_trans_pct'],
          'control_ate_m':control['ate_m'],'candidate_ate_m':candidate['ate_m'],
          'control_path_length_ratio':control_diag['path_length_ratio'],
          'candidate_path_length_ratio':candidate_diag['path_length_ratio'],
          'control_bad_length_steps':control_diag['steps_length_off_over_threshold'],
          'candidate_bad_length_steps':candidate_diag['steps_length_off_over_threshold'],
          'control_bad_vector_steps':control_diag['steps_vector_off_over_threshold'],
          'candidate_bad_vector_steps':candidate_diag['steps_vector_off_over_threshold'],
          'candidate_moving_steps':candidate_diag['moving_step_count'],
          'control_bad_length_ranges':control_diag['bad_length_step_frame_ranges'],
          'candidate_bad_length_ranges':candidate_diag['bad_length_step_frame_ranges'],
          'control_largest_error_growth_blocks':error_growth(control_diag),
          'candidate_largest_error_growth_blocks':error_growth(candidate_diag),
          'candidate_worst_blocks':worst,'comparison_path':str((folder/'comparison.json').relative_to(REPO))})
    missing=sorted(set(EXPECTED)-{r['sequence'] for r in rows})
    if not rows or (missing and not args.allow_partial):
        print(f'Incomplete committed coverage; missing sequences: {missing}',file=sys.stderr)
        return 2
    payload={'schema_version':1,'command':[sys.executable,*sys.argv],'cwd':str(Path.cwd()),
      'generated_at':datetime.now(timezone.utc).isoformat(),'input_commit':revision,
      'script_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
      'implementation_commit':native['source_commit'],'fixed_arguments':protocol['suma_fixed_arguments'],
      'complete':not missing,'missing_sequences':missing,'scope':'Non-paper fixed-setting transfer, no paper ratio or speed claim',
      'diagnostic_block_selection':{
          'candidate_worst_blocks':'Largest finite relative endpoint translation errors among blocks with moving steps.',
          'largest_error_growth_blocks':'Largest positive end-minus-start pointwise position-error increases, including stationary blocks; no alignment fit or rescaling.',
          'scope':'Blocks retain GT travel distance and frame bounds. Both rankings are diagnostic only, not official KITTI RTE.'},
      'rows':rows,'worse_sequences':[r['sequence'] for r in rows if r['rte_difference_pp']>1e-6],
      'audit_rows':audit_rows,'audit_official_metric_checks':audit_official_checks,
      'raw_control_preservation_checks':raw_preservation,
      'input_identity_reference':'Official scan .bin SHA256/bytes and GT CSV must match the prior committed full shared-remap study; per-fetch metadata is excluded.',
      'audit_preservation_summary':{
          'cells':len(audit_rows),'drifted_cells':sum(bool(x['drifted']) for x in audit_rows),
          'max_abs_rpe_trans_delta_pct':max(x['rerun_rpe_abs_delta'] for x in audit_rows),
          'max_abs_ate_delta_m':max(x['rerun_ate_abs_delta'] for x in audit_rows),
          'max_abs_frame_delta':max(abs(x['frames']-x['stored_frames']) for x in audit_rows),
          'max_abs_official_translation_rte_delta_pct':max(x['absolute_delta']['kitti_rte_trans_pct'] for x in audit_official_checks),
          'max_abs_official_rotation_rte_delta_deg_per_100m':max(x['absolute_delta']['kitti_rte_rot_deg_per_100m'] for x in audit_official_checks)},
      'committed_input_sha256':sources}
    args.output_dir.mkdir(parents=True,exist_ok=True)
    (args.output_dir/'summary.json').write_text(json.dumps(payload,indent=2,allow_nan=False)+'\n')
    fields=['sequence','frames','control_rte_pct','candidate_rte_pct','rte_difference_pp','control_ate_m',
            'candidate_ate_m','control_path_length_ratio','candidate_path_length_ratio','control_bad_length_steps','candidate_bad_length_steps']
    with (args.output_dir/'comparison.csv').open('w',newline='') as stream:
        writer=csv.DictWriter(stream,fieldnames=fields,extrasaction='ignore');writer.writeheader();writer.writerows(rows)
    table=['| Seq | Raw RTE [%] | Coarse RTE [%] | Raw ATE [m] | Coarse ATE [m] |',
           '|---|---:|---:|---:|---:|']
    table += [f"| {r['sequence']} | {r['control_rte_pct']:.6f} | {r['candidate_rte_pct']:.6f} | {r['control_ate_m']:.6f} | {r['candidate_ate_m']:.6f} |" for r in rows]
    (args.output_dir/'table.md').write_text('\n'.join(table)+'\n')
    diagnostics=['| Seq | Coarse path / GT | Raw bad length steps | Coarse bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |',
                 '|---|---:|---:|---:|---|---:|---:|']
    for row in rows:
        growing=row['candidate_largest_error_growth_blocks']
        block=growing[0] if growing else None
        interval=f"{block['first_pose_frame']}-{block['last_pose_frame']}" if block else '-'
        distance=f"{block['gt_distance_m']:.3f}" if block else '-'
        increase=f"{block['position_error_increase_m']:.3f}" if block else '-'
        moving=row['candidate_moving_steps']
        diagnostics.append(f"| {row['sequence']} | {row['candidate_path_length_ratio']:.6f} | {row['control_bad_length_steps']}/{moving} | {row['candidate_bad_length_steps']}/{moving} | {interval} | {distance} | {increase} |")
    diagnostics+=['','Bad length steps use GT motion at least 0.1 m and length error above 20%.',
      'Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.',
      'Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.',
      'These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.']
    (args.output_dir/'diagnostics.md').write_text('\n'.join(diagnostics)+'\n')
    print(json.dumps({'complete':not missing,'completed_sequences':[r['sequence'] for r in rows],
                      'missing_sequences':missing,'worse_sequences':payload['worse_sequences']}))
    return 0

if __name__=='__main__':
    raise SystemExit(main())
