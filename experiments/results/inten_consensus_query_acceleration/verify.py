"""Preserve historical science and reconstruct the uniform recursive comparison."""
from pathlib import Path
from collections import Counter
import argparse,csv,hashlib,importlib.util,io,json,math,subprocess
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
BASE='13ab2f8f0871707060f4923ff3362317280f8af4'
def read(p):return json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=W,text=True).strip()
def before(n):return subprocess.check_output(['git','show',BASE+':'+n],cwd=W)
def require(ok,reason):
    if not ok:raise ValueError(reason)
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def cached_controls():
    restore=module('recursive_archive_reader',E.parent/'inten_cluster_motion/verify.py').restored
    folder=E/'checks/cached_parity_balanced'
    rows=[list(map(json.loads,restore(folder,'native_'+str(t)+'.stdout.jsonl').splitlines())) for t in (1,4)]
    require(rows[0]==rows[1],'Worker-count parity drift')
    profiles=list(map(json.loads,restore(E.parent/'inten_cluster_motion/controls','native.stdout.jsonl').splitlines()))
    old={(p['frame'],p['cluster']):p['costs_m2'] for p in profiles if p['type']=='profile'}
    actual={(p['frame'],p['cluster']):p['costs_m2'] for p in rows[0] if p['type']=='profile'}
    require(old==actual,'Native full-point profile cost drift')
    previous={(p['frame'],p['mode']):p for p in map(json.loads,restore(E.parent/'inten_fixed_rotation/controls','native.stdout.jsonl').splitlines())}
    check=module('recursive_decision_checker',E/'check_consensus_trace.py').check_frame
    calls=0
    for row in rows[0]:
        if row['type']!='terminal':continue
        i,m=row['frame'],row['mode'];f=row['trace'];check(f,m==1)
        def equal(a,b):
            a,b=dict(a),dict(b);last=a.pop('returned_pairs');first=b.pop('returned_pairs')
            require(last==a['iterations'][-1]['pairs'] and first==sum(b['iterations'][0]['class_pairs']),'Forced/normal count convention drift')
            require(a==b,'Native solver trace drift')
        equal(f['s2s'],previous[(i,0)]['solver'])
        selected=0
        if m and f['s2s_consensus']['usable']:
            selected=5;equal(f['s2s_consensus']['refinement'],previous[(i,5)]['solver'])
        require(f['accepted_relative']==previous[(i,selected)]['output'],'Conditional estimated pose drift');calls+=1
    require(calls==16,'Incomplete cached endpoint controls')
    return dict(profiles=len(actual),cached_native_calls_per_worker_count=calls,worker_counts_bitwise_equal=True,all_geometry_and_conditional_native_controls_exact=True)

def validate(history=True):
    post=read(E/'postprocessing_protocol.json');P=read(E/'full_protocol.json');pins=read(E/'binaries_balanced.json')
    for name,digest in post['source_sha256'].items():require(sha(E/name)==digest,'Postprocessing source changed: '+name)
    for name,digest in P['source_sha256'].items():
        if history or name.startswith(str(E.relative_to(W))+'/'):require(sha(W/name)==digest,'Frozen producer source changed: '+name)
    require(all(sha(W/name)==digest for name,digest in P['preserved_original_sha256'].items()),'Original negative/interruption evidence changed')
    historical=read(E/'historical_aggregate_sha256.json')
    require(all(sha(W/n)==h for n,h in historical.items()),'Historical aggregate drift')
    require(read(E/'collection_execution/command.json')['exit_code']==0,'Collection not complete')
    outer=read(E/'checks/outer_execution_completion.json')
    require(outer['actual_os_exit_code'] is None and not outer['outer_tool_result_available'],'Lost external-session exit must not be fabricated')
    require(outer['collector_subprocess_exit_code']==0 and outer['collector_receipt_sha256']==sha(W/outer['collector_receipt_path']),'Durable actual collector exit receipt changed')
    for label in ('configure','build','ctest','runner'):
        require(read(E/'checks/candidate_build_balanced'/(label+'_command.json'))['exit_code']==0,'Corrected native build failed: '+label)
    require(read(E.parent/'inten_recursive_consensus/checks/candidate_build/ctest_command.json')['exit_code']==8,'Initial expectation failure lost')
    require(read(E/'checks/cached_parity_balanced/result.json')['exit_code']==0 and read(E/'checks/cached_parity_balanced/decision_verification.json')['exit_code']==0,'Cached controls failed')
    cached=cached_controls()
    for path in ('report_execution/command.json','import_execution/command.json','import_execution/result.json'):
        require(read(E/path)['exit_code']==0,'Postprocessing failure: '+path)
    report=read(E/'report/summary.json')
    require(report['exit_code']==0 and report['all_sequences_present'] and report['parent_reference_equal_all_sequences'],'Incomplete numerical report')
    require(set(report['sequences'])==set(P['manifests'])=={f'{i:02d}' for i in range(11)},'Full sequence coverage differs')
    configs=None;ratios=[];over_twice=[];sequences={}
    for seq,meta in sorted(P['manifests'].items()):
        require(sha(W/meta['path'])==meta['sha256'],'Manifest changed')
        folder=E/'sequences'/seq;cleanup=read(folder/'cleanup.json')
        require(cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after'],'Cleanup evidence incomplete')
        if history:require(not Path(cleanup['owned_input_dir']).exists(),'Owned raw scans remain')
        inputs=read(folder/'inputs.json');original=read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json')
        require(inputs['files']==original['files'] and inputs['gt_csv_sha256']==original['gt_csv_sha256'],'Official input hashes drift')
        cfg=read(folder/'effective_configuration.json')
        if configs is None:configs=cfg
        else:require(configs==cfg,'Effective setting differs across sequences')
        a,b=[dict(cfg[m]['params']) for m in ('baseline','candidate')]
        a.pop('s2s_geometry_consensus',None);b.pop('s2s_geometry_consensus',None)
        require(a==b and cfg['baseline']['loader']==cfg['candidate']['loader'],'Multiple setting changes')
        for mode in ('baseline','candidate'):
            p=cfg[mode]['params'];require(p.get('s2s_geometry_consensus',False)==(mode=='candidate') and not p.get('s2s_facade_cauchy',False) and not p.get('s2s_predicted_dor_facade',False) and not p.get('s2s_row_support',False),'Wrong fixed flags')
            require(p['huber']==.1 and p['s2m_plane_min_spread']==0. and p['s2s_translation_prior_ratio']==.03 and all(p[k] for k in ('accelerated','s2s_mapped_prior','s2s_ground_pca','s2s_facade_pca')),'Inherited reference differs')
            s=read(folder/mode/'summary.json');receipt=read(folder/mode/'native_command.json');v=read(folder/mode/'trace_validation.json');c=read(folder/mode/'consensus_validation.json')
            require(receipt['exit_code']==0 and receipt['native_source_commit']==P['native_source_commit'] and receipt['binary_sha256']==pins['runner']['sha256'],'Native producer/exit drift')
            require(receipt['environment']=={'OMP_NUM_THREADS':str(P['native_arm_workers'][mode]),'OPENBLAS_NUM_THREADS':'1'},'Workers differ across sequences')
            require(s['methods'][0]['status']=='ok' and s['methods'][0]['frames']==inputs['scan_frames']==v['processed_frames']==c['frames'] and v['skipped_frames']==0 and v['nonfinite_values']==0,'Incomplete/nonfinite native traversal')
            require(c['exit_code']==0 and c['enabled']==(mode=='candidate') and c['all_votes_masks_and_fallbacks_reconstructed'] and c['all_projected_rotations_bitwise_original'] and c['all_projected_updates_additive_and_backward_checked'],'Consensus proof failed')
            imported=W/'experiments/results/runs'/Path(meta['path']).stem/mode
            require((imported/'summary.json').read_bytes()==(folder/mode/'summary.json').read_bytes(),'Imported summary drift')
            for artifact in (folder/mode/'dogfooding_results').glob('*.txt'):
                require((imported/'dogfooding_results'/artifact.name).read_bytes()==artifact.read_bytes(),'Imported poses/GT drift')
            metric=report['sequences'][seq]['modes'][mode]
            for key in ('frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m'):
                require(metric[key]==s['methods'][0][key],'Report/native metric drift: '+key)
            with (E/'report'/f'{seq}_{mode}_steps.csv').open() as stream:steps=list(csv.DictReader(stream))
            require(len(steps)==inputs['scan_frames']-1,'Step coverage differs')
            for label in ('length','vector'):
                require(sum(r['final_bad_'+label]=='True' for r in steps)==metric['final_bad_'+label],'Bad-step count drift')
        for mode in ('baseline','candidate'):
            for label in ('validate','consensus_validate','steps'):
                require(read(folder/mode/'analysis'/(label+'_command.json'))['exit_code']==0,'Actual full validation command failed')
        require(read(folder/'equivalence_analysis/implementation_equivalence_command.json')['exit_code']==0,'Actual implementation-equivalence check failed')
        equivalent=read(folder/'implementation_equivalence.json');require(equivalent['exit_code']==0,'Implementation equivalence missing')
        if seq=='01':
            for mode in ('baseline','candidate'):
                original_folder=E.parent/'inten_recursive_consensus/sequences/01'/mode
                require(read(original_folder/'summary.json')['methods'][0]['kitti_rte_trans_pct']==read(folder/mode/'summary.json')['methods'][0]['kitti_rte_trans_pct'],'Original full01 negative changed')
                for pth in (original_folder/'dogfooding_results').glob('*.txt'):
                    require(pth.read_bytes()==(folder/mode/'dogfooding_results'/pth.name).read_bytes(),'Original01 pose/GT changed')
                require(equivalent['checks'][mode]['numerical_trace_body_exact'],'Full01 trace equivalence missing')
        if seq=='07':require(equivalent['checks']['candidate_prefix']['numerical_trace_body_exact'] and equivalent['checks']['candidate_prefix']['complete_original_frames_compared']>0,'Interrupted07 complete-prefix equivalence missing')
        control=read(folder/'legacy_compatibility.json')
        require(control['exit_code']==0 and control['trace_body_exact_after_scan_root_substitution'],'Historical baseline trace mismatch')
        parent=W/'experiments/results/inten_facade_pca/sequences'/seq/'candidate'
        for name in control['saved_pose_gt_bytes_equal']:
            require((parent/'dogfooding_results'/name).read_bytes()==(folder/'baseline/dogfooding_results'/name).read_bytes(),'Baseline artifact drift')
        old=read(parent/'summary.json')['methods'][0];baseline=read(folder/'baseline/summary.json')['methods'][0];candidate=read(folder/'candidate/summary.json')['methods'][0]
        require(all(old[k]==baseline[k] for k in ('frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')),'Baseline metric drift')
        data=report['sequences'][seq];ratio=candidate['kitti_rte_trans_pct']/baseline['kitti_rte_trans_pct'];ratios.append(ratio)
        require(data['candidate_over_baseline']==ratio,'Candidate/reference ratio drift')
        paper=post['paper_values']['kitti_'+seq]
        require(data['paper_rte_pct']==paper and data['candidate_over_paper']==candidate['kitti_rte_trans_pct']/paper,'Paper ratio drift')
        if data['candidate_over_paper']>2:over_twice.append(seq)
        manifest=read(folder/'archive.json');require(manifest['byte_roundtrip_verified'],'Archive not roundtripped')
        for item in manifest['files'].values():
            if item['storage']=='ordered_gzip_shards':
                for part in item['parts']:require(sha(folder/part['path'])==part['gzip_sha256'],'Archived trace shard changed')
            else:require(sha(folder/item['path'])==item['stored_sha256'],'Archived output changed')
        aggregate=read(W/'experiments/results'/(Path(meta['path']).stem+'.json'))
        require(aggregate['status']=='ready' and all(v['fps'] is None and v['time_ms'] is None and v['benchmark_score']==0 and not v['measurement_context']['standalone_timing_comparable'] for v in aggregate['variants']),'New result scored or incomplete')
        sequences[seq]=dict(frames=inputs['scan_frames'],actual_native_exits=[0,0],all_geometry_decisions_and_projected_steps_checked=True,reference_trace_pose_gt_exact=True,archive_roundtrip=True)
    require(over_twice==report['over_twice_paper_sequences'],'Paper exception set drift')
    require(math.isclose(math.exp(sum(math.log(v) for v in ratios)/len(ratios)),report['geometric_mean_candidate_over_baseline'],rel_tol=1e-12),'Geometric mean drift')
    if history:
        require(not (R/'collector.lock').exists(),'Collector active')
        protected=[str((E/n).relative_to(W)) for n in ('sequences','full_protocol.json','binaries_balanced.json','collect.py','build_balanced.py')]
        require(not git('diff','--name-only',post['raw_collection_commit'],'--',*protected),'Collection changed after numerical snapshot')
        sourcepaths=['common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**','CMakeLists.txt']
        require(not git('diff','--name-only',P['native_source_commit'],'--',*sourcepaths),'Native source changed after producer')
        for item in (pins['runner'],):require(sha(item['path'])==item['sha256'],'Pinned binary changed')
        for name,h in pins['linked_static_libraries'].items():require(sha(name)==h,'Pinned library changed')
        for name in ('README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json'):
            require(before(name)==(W/name).read_bytes(),'Primary publication changed: '+name)
        require((W/'papers/inten_loam/README.md').read_bytes().startswith(before('papers/inten_loam/README.md')),'Historical README changed')
        name='docs/assets/paper/full_variant_results.csv';reader=csv.DictReader(io.StringIO(before(name).decode()));columns=reader.fieldnames;old=list(reader)
        with (W/name).open() as stream:reader=csv.DictReader(stream);require(reader.fieldnames==columns,'Variant columns changed');current=list(reader)
        counts=lambda rows:Counter(tuple(r[k] for k in columns) for r in rows)
        require(not (counts(old)-counts(current)),'Historical variant row changed')
        owned={read(W/m['path'])['problem']['id'] for m in P['manifests'].values()}
        extra=[r for r in current if r['problem_id'] in owned]
        require(len(extra)==22 and len(current)==len(old)+22 and all(r['is_current_default']=='false' and not r['fps'] for r in extra),'New row promoted or ranked by timing')
        new_names={str(Path('experiments/results')/(Path(m['path']).stem+'.json')) for m in P['manifests'].values()}
        new_runs={'experiments/results/runs/'+Path(m['path']).stem+'/' for m in P['manifests'].values()}
        for name in git('diff','--name-only',BASE,'--','experiments/results').splitlines():
            require(name.startswith(str(E.relative_to(W))+'/') or name in P['preserved_original_sha256'] or name in new_names or name=='experiments/results/index.json' or any(name.startswith(p) for p in new_runs),'Historical result file changed: '+name)
    return dict(exit_code=0,historical_aggregate_count=len(historical),historical_aggregates_unchanged=True,full_sequence_count=len(sequences),sequences=sequences,cached=cached,history_and_live_producer_checked=history,policy='All outcomes retained; outside-paper option remains default off, no per-sequence adoption or standalone FPS claim. Local rotation constraint does not fix global mapped rotation.')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    a.output.write_text(json.dumps(validate(),indent=2)+'\n');print('Uniform full comparison, archived controls and historical preservation passed')
