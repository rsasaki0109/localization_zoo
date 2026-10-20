"""Report every fixed full-sequence outcome from committed lossless evidence."""
from pathlib import Path
from collections import Counter
import argparse
import csv
import hashlib
import importlib.util
import json
import math
import subprocess
import sys
import tempfile

import numpy as np

E = Path(__file__).resolve().parent
W = E.parents[2]
sys.path.insert(0, str(W/'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_kitti_poses, normalize_first_pose


def module(name, path):
    spec=importlib.util.spec_from_file_location(name,path)
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value)
    return value


parent=module('row_evidence',W/'experiments/results/inten_natural_traces/diagnose.py')


def read(path):return json.loads(Path(path).read_text())
def write(path,data):Path(path).write_text(json.dumps(data,indent=2,allow_nan=False)+'\n')
def require(ok,message):
    if not ok:raise ValueError(message)
def matrix(values):return np.asarray(values,dtype=float).reshape(4,4)


def analyze_mode(folder, output, seq, mode):
    summary=read(folder/'summary.json');method=summary['methods'][0]
    validation=read(folder/'trace_validation.json');motion=read(folder/'motion_steps.json')
    require(validation['exit_code']==0 and validation['skipped_frames']==0,'incomplete trace')
    require(read(folder/'native_command.json')['exit_code']==0,'native failure')
    estimated_raw=np.asarray(load_kitti_poses(folder/'dogfooding_results/InTEn_LOAM.txt'))
    gtpath=Path(summary['artifacts']['evaluated_gt_path'])
    require(not gtpath.is_absolute(),'expected archive-relative evaluated GT artifact')
    truth_raw=np.asarray(load_kitti_poses(folder/gtpath))
    estimated=np.asarray(normalize_first_pose(list(estimated_raw)))
    truth=np.asarray(normalize_first_pose(list(truth_raw)))
    require(len(truth)==len(estimated)==method['frames']==validation['processed_frames'],'frame/GT coverage mismatch')
    rows=[];features=[];stops={s:Counter() for s in ('s2s','s2m')};systems=0
    max_saved=0.;previous_accepted=None;frame_count=0
    with (folder/'trace.jsonl').open() as stream:
        header=json.loads(next(stream))
        require(header['params']['accelerated'] is True,'acceleration differs from protocol')
        require(header['params'].get('s2s_row_support',False)==(mode=='candidate'),'row option differs from protocol')
        for line in stream:
            frame=json.loads(line)
            if frame['type']=='end':continue
            require(frame['type']=='frame' and frame['frame_index']==frame['input_index']==frame_count,'not a complete natural traversal')
            i=frame_count;frame_count+=1;features.append(frame['features'])
            saved_error=float(np.max(np.abs(matrix(frame['anchored_pose'])-estimated_raw[i])))
            max_saved=max(max_saved,saved_error)
            require(np.allclose(matrix(frame['anchored_pose']),estimated_raw[i],atol=1e-5,rtol=1e-8),'trace/saved pose precision mismatch')
            if frame['initialization']:
                require(i==0,'late initialization')
                previous_accepted=frame['accepted_relative'];continue
            require(frame['s2s']['input']==previous_accepted,'S2S prediction flow changed')
            previous_accepted=frame['accepted_relative']
            gt=np.linalg.inv(truth[i-1])@truth[i]
            final=np.linalg.inv(estimated[i-1])@estimated[i]
            s2s=matrix(frame['s2s']['output']);step=float(np.linalg.norm(gt[:3,3]));moving=step>=.1
            def ratio(T):return float(np.linalg.norm(T[:3,3])/step) if moving else None
            def error(T):return float(np.linalg.norm(T[:3,3]-gt[:3,3])/step) if moving else None
            a,b=ratio(s2s),ratio(final)
            row=dict(frame=i,moving=moving,gt_step_m=step,s2s_input_length_ratio=ratio(matrix(frame['s2s']['input'])),s2s_length_ratio=a,final_length_ratio=b,
                s2s_vector_error=error(s2s),final_vector_error=error(final),
                s2s_bad_length=abs(a-1)>.2 if moving else False,
                final_bad_length=abs(b-1)>.2 if moving else False,
                s2s_bad_vector=error(s2s)>.2 if moving else False,
                final_bad_vector=error(final)>.2 if moving else False,
                s2s_fallback=frame['s2s_fallback'],s2m_accepted=frame['s2m_accepted'])
            for stage in ('s2s','s2m'):
                solver=frame[stage];stops[stage][solver['stop']]+=1;systems+=len(solver['iterations'])
                row[stage+'_stop']=solver['stop'];row[stage+'_iterations']=len(solver['iterations'])
                for position in ('first','last'):
                    counts=solver['iterations'][0 if position=='first' else -1]['class_pairs'] if solver['iterations'] else [0]*4
                    for name,count in zip(('edge','ground','facade','intensity'),counts):row[f'{stage}_{position}_{name}_pairs']=count
            rows.append(row)
    require(frame_count==len(truth),'missing trace frame')
    require(sum(r['final_bad_length'] for r in rows)==motion['steps_length_off_over_threshold'],'saved-motion length label mismatch')
    require(sum(r['final_bad_vector'] for r in rows)==motion['steps_vector_off_over_threshold'],'saved-motion vector label mismatch')
    with (output/f'{seq}_{mode}_steps.csv').open('w') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(rows[0]),lineterminator='\n');writer.writeheader();writer.writerows(rows)
    data=dict(native_exit_code=0,frames=method['frames'],kitti_rte_trans_pct=method['kitti_rte_trans_pct'],
        kitti_rte_rot_deg_per_100m=method.get('kitti_rte_rot_deg_per_100m'),ate_m=method['ate_m'],
        moving_steps=sum(r['moving'] for r in rows),s2s_bad_length=sum(r['s2s_bad_length'] for r in rows),
        s2s_bad_vector=sum(r['s2s_bad_vector'] for r in rows),final_bad_length=sum(r['final_bad_length'] for r in rows),
        final_bad_vector=sum(r['final_bad_vector'] for r in rows),s2s_fallback=sum(r['s2s_fallback'] for r in rows),
        s2m_rejected=sum(not r['s2m_accepted'] for r in rows),solver_systems=systems,
        stops={s:dict(v) for s,v in stops.items()},max_trace_saved_pose_difference=max_saved,
        prediction_flow_checks=len(rows),bad_length_frame_ranges=motion['bad_length_step_frame_ranges'],
        bad_vector_frame_ranges=motion['bad_vector_step_frame_ranges'],
        path_length_ratio=motion['path_length_ratio'],motion_blocks=motion['blocks'],
        largest_translation_error_blocks=sorted((b for b in motion['blocks'] if b['block_relative_translation_error_pct'] is not None),key=lambda b:b['block_relative_translation_error_pct'],reverse=True)[:3],
        known_entry_observations=[r for r in rows if r['frame'] in {'01':(420,),'02':(1876,4460),'10':()}.get(seq,())],
        first_ground_pairs=parent.distribution(r['s2s_first_ground_pairs'] for r in rows),
        first_facade_pairs=parent.distribution(r['s2s_first_facade_pairs'] for r in rows),
        source_trace_sha256=validation['trace_sha256'])
    return data,features,truth_raw


def main():
    for source in (Path(__file__).resolve(),E/'postprocessing_protocol.json'):
        if subprocess.check_output(['git','show','HEAD:'+str(source.relative_to(W))],cwd=W)!=source.read_bytes():
            raise ValueError('Postprocessing source must be committed before execution: '+str(source))
    post_plan=read(E/'postprocessing_protocol.json')
    protected=[str((E/name).relative_to(W)) for name in ('sequences','protocol.json','binaries.json','collect.py','build.py')]
    require(not subprocess.check_output(['git','diff','--name-only',post_plan['raw_collection_commit'],'--',*protected],cwd=W,text=True).strip(),'Original collection evidence changed after its final raw commit')
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output-dir',type=Path,required=True);args=parser.parse_args()
    args.output_dir.mkdir(parents=True,exist_ok=False)
    protocol=read(E/'protocol.json');sequences={};comparison=[]
    for seq in sorted(protocol['manifests']):
        folder=E/'sequences'/seq
        require(read(folder/'cleanup.json')['hashes_verified_before_removal'],'cleanup not verified')
        with tempfile.TemporaryDirectory() as temporary:
            temporary=Path(temporary);parent.restore(folder,temporary)
            modes={};feature_sets={};gt={}
            for mode in ('baseline','candidate'):
                modes[mode],feature_sets[mode],gt[mode]=analyze_mode(temporary/mode,args.output_dir,seq,mode)
            require(feature_sets['baseline']==feature_sets['candidate'],'feature extraction changed between comparison arms')
            require(np.array_equal(gt['baseline'],gt['candidate']),'GT artifacts differ')
            compatibility=read(temporary/'legacy_compatibility.json') if seq=='07' else None
            if compatibility:require(compatibility['exit_code']==0,'legacy gate failed')
            pair_control=read(temporary/'default_pair_compatibility/result.json') if seq in ('01','02','10') else None
            if pair_control:require(pair_control['exit_code']==0,'default-off pair gate failed')
        baseline=modes['baseline']['kitti_rte_trans_pct'];candidate=modes['candidate']['kitti_rte_trans_pct']
        require(baseline>0 and candidate>0 and math.isfinite(baseline+candidate),'invalid official RTE')
        historical=read(W/'experiments/results'/f'inten_loam_kitti_seq_{seq}_full_paper_comp_matrix.json')
        historical_rte=next(v['kitti_rte_trans_pct'] for v in historical['variants'] if v['id']=='paper_odometry_plane_check')
        sequences[seq]=dict(modes=modes,candidate_over_baseline=candidate/baseline,
            historical_primary_rte_pct=historical_rte,fresh_reference_minus_historical_rte_pct=baseline-historical_rte,
            features_equal_every_frame=True,
            gt_artifacts_equal=True,legacy_full07_control=compatibility,default_pair_control=pair_control)
        comparison.append(dict(sequence=seq,frames=modes['baseline']['frames'],baseline_rte_pct=baseline,candidate_rte_pct=candidate,candidate_over_baseline=candidate/baseline,
            historical_primary_rte_pct=historical_rte,fresh_reference_minus_historical_rte_pct=baseline-historical_rte,
            baseline_ate_m=modes['baseline']['ate_m'],candidate_ate_m=modes['candidate']['ate_m'],
            baseline_rotation_deg_per_100m=modes['baseline']['kitti_rte_rot_deg_per_100m'],candidate_rotation_deg_per_100m=modes['candidate']['kitti_rte_rot_deg_per_100m'],
            baseline_bad_vector=modes['baseline']['final_bad_vector'],candidate_bad_vector=modes['candidate']['final_bad_vector'],
            baseline_first_ground_pairs_median=modes['baseline']['first_ground_pairs']['p50'],candidate_first_ground_pairs_median=modes['candidate']['first_ground_pairs']['p50'],
            baseline_first_facade_pairs_median=modes['baseline']['first_facade_pairs']['p50'],candidate_first_facade_pairs_median=modes['candidate']['first_facade_pairs']['p50'],
            baseline_bad_length=modes['baseline']['final_bad_length'],candidate_bad_length=modes['candidate']['final_bad_length'],
            baseline_s2s_bad_length=modes['baseline']['s2s_bad_length'],candidate_s2s_bad_length=modes['candidate']['s2s_bad_length']))
        print('analyzed',seq,flush=True)
    with (args.output_dir/'comparison.csv').open('w') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(comparison[0]),lineterminator='\n');writer.writeheader();writer.writerows(comparison)
    result=dict(exit_code=0,source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),
        sequence_count=len(sequences),all_sequences_present=len(sequences)==11,
        geometric_mean_candidate_over_baseline=float(np.exp(np.mean(np.log([s['candidate_over_baseline'] for s in sequences.values()])))),
        lower_rte_sequences=[s for s,d in sequences.items() if d['candidate_over_baseline']<1],
        higher_rte_sequences=[s for s,d in sequences.items() if d['candidate_over_baseline']>1],
        adverse_metrics={s:[k for k in ('kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m','ate_m','final_bad_length','final_bad_vector')
            if d['modes']['candidate'][k]>d['modes']['baseline'][k]] for s,d in sequences.items()},
        historical_reference_equal_all_sequences=all(d['fresh_reference_minus_historical_rte_pct']==0 for d in sequences.values()),sequences=sequences,
        limits='One fixed opt-in setting on every complete sequence, with acceleration held fixed in both arms. Public deterministic benchmark comparison, not a pure-paper score or independent held-out result. Stage counts condition on each realized trajectory; paired concurrency precludes standalone FPS claims. Local20% step labels are not official distance-segment RTE.')
    write(args.output_dir/'summary.json',result)
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    figure,axes=plt.subplots(1,2,figsize=(11,4.5),constrained_layout=True)
    labels=[r['sequence'] for r in comparison];x=np.arange(len(labels))
    axes[0].plot(x,[r['baseline_rte_pct'] for r in comparison],'o-',label='Fixed reference')
    axes[0].plot(x,[r['candidate_rte_pct'] for r in comparison],'o-',label='Missing-row completion')
    axes[0].set(xticks=x,xticklabels=labels,yscale='log',ylabel='Official KITTI RTE [%]',xlabel='Sequence');axes[0].legend(fontsize=8)
    ratios=[r['candidate_over_baseline'] for r in comparison]
    axes[1].bar(x,ratios,color=['#1864ab' if v<=1 else '#c92a2a' for v in ratios]);axes[1].axhline(1,color='0.5',linewidth=.8)
    axes[1].set(xticks=x,xticklabels=labels,ylabel='Candidate / fixed-reference RTE',xlabel='Sequence')
    figure.suptitle('Fixed full-sequence comparison; repository extension, default off')
    figure.savefig(args.output_dir/'official_rte.png',dpi=160);plt.close(figure)


if __name__=='__main__':main()
