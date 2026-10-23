#!/usr/bin/env python3
"""Report complete trajectories and every preregistered fixed-pair control."""
import argparse
import csv
import gzip
import json
from pathlib import Path
import sys
from receipt import save, sha


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--kiss',type=Path,required=True)
    p.add_argument('--probe',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    sys.path.insert(0,str(a.source_root/'evaluation/scripts'))
    import numpy as np
    from evaluate_external_kitti_odometry import load_kitti_poses, load_reference_csv, normalize_first_pose
    from diagnose_kitti_steps import diagnose
    base=a.source_root/'experiments/results'
    facade=base/'inten_facade_pca/sequences/01/candidate/dogfooding_results'
    historical=base/'shared_near_range/sequences/01/audit/replays/kiss_icp_kitti_seq_01_full_elevation_matrix__upstream_mechanics_elevation_native/dogfooding_results'
    paths={'inten_fixed':facade/'InTEn_LOAM.txt','kiss_same_input':a.kiss/'poses.txt',
           'kiss_historical_different_input':historical/'KISS_ICP.txt'}
    gtpath=a.source_root/'experiments/reference_data/kitti_seq_01_full_gt.csv'
    gt=load_reference_csv(gtpath)
    # Historical outputs are first-pose anchored; relative poses must have the same GT frame convention.
    for saved in [facade/'velodyne_evaluated_gt.txt',historical/'kitti_seq_01_full_evaluated_gt.txt']:
        assert np.max(np.abs(np.array(normalize_first_pose(load_kitti_poses(saved)))-np.array(normalize_first_pose(gt))))<1e-5
    g=np.array(gt); delta=np.linalg.inv(g[:-1])@g[1:]
    reports={}; steps=[]
    for name,path in paths.items():
        poses=load_kitti_poses(path)
        reports[name]=diagnose(poses,gt)
        local=np.linalg.inv(np.array(poses[:-1]))@np.array(poses[1:])
        for i,(est,ref) in enumerate(zip(local,delta),1):
            length=np.linalg.norm(ref[:3,3])
            steps.append({'method':name,'frame':i,'gt_length_m':float(length),
                'estimate_length_m':float(np.linalg.norm(est[:3,3])),
                'length_error_fraction':float(abs(np.linalg.norm(est[:3,3])/length-1)),
                'vector_error_fraction':float(np.linalg.norm(est[:3,3]-ref[:3,3])/length)})
    protocol=json.loads(Path(__file__).with_name('protocol.json').read_text())
    spec=protocol['diagnostic']; threshold=spec['step_bad_threshold']
    with gzip.open(a.probe/'raw.jsonl.gz','rt') as f:
        rows=[json.loads(line) for line in f if line.startswith('{')]
    paired=[]
    for row in rows:
        index,mode=row['id'].split('_',1); i=int(index)
        ref=delta[i-1]; length=np.linalg.norm(ref[:3,3])
        result={'frame':i,'mode':mode,'gt_length_m':float(length)}
        for key in ['seed','output']:
            pose=np.array(row[key]).reshape(4,4)
            result[key+'_length_ratio']=float(np.linalg.norm(pose[:3,3])/length)
            result[key+'_vector_error_fraction']=float(np.linalg.norm(pose[:3,3]-ref[:3,3])/length)
            result[key+'_length_bad']=bool(abs(result[key+'_length_ratio']-1)>threshold and length>=spec['min_gt_step_m'])
            result[key+'_vector_bad']=bool(result[key+'_vector_error_fraction']>threshold and length>=spec['min_gt_step_m'])
        paired.append(result)
    counts={}
    for mode in spec['seeds']:
        subset=[r for r in paired if r['mode']==mode]
        counts[mode]={'pairs':len(subset)}
        for group in ['length','vector']:
            for stage in ['seed','output']:
                counts[mode][stage+'_'+group+'_bad']=sum(r[stage+'_'+group+'_bad'] for r in subset)
            counts[mode]['good_seed_to_bad_output_'+group]=sum(not r['seed_'+group+'_bad'] and r['output_'+group+'_bad'] for r in subset)
    a.output.mkdir(parents=True,exist_ok=False)
    for name,rows in [('steps.csv',steps),('paired.csv',paired)]:
        with (a.output/name).open('w',newline='') as f:
            writer=csv.DictWriter(f,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    replay=json.loads((a.probe/'completion.json').read_text())
    save(a.output/'summary.json',{'scope':protocol['scope'],'paths':{k:str(v) for k,v in paths.items()},
        'input_sha256':{k:sha(v) for k,v in dict(paths,gt=gtpath).items()},
        'trajectories':reports,'paired_counts':counts,
        'controls_valid':replay['native_replay_all_equal'] and replay['loader_point_counts_all_equal'],
        'probe_completion_sha256':sha(a.probe/'completion.json')})


if __name__=='__main__':
    main()
