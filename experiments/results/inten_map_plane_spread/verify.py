"""Preserve every historical result and prove the one-setting full comparison."""
from pathlib import Path
from collections import Counter
import argparse,csv,hashlib,io,json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent;BASE='87fc51aa8fd4b6a1af7016ad03158787ec0b3ef7'
def read(p):return json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def before(n):return subprocess.check_output(['git','show',BASE+':'+n],cwd=W)
def git(*a):return subprocess.check_output(['git',*a],cwd=W,text=True).strip()
def require(ok,reason):
    if not ok:raise ValueError(reason)
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    for p in Path(__file__).resolve(),E/'postprocessing_protocol.json':require(subprocess.check_output(['git','show','HEAD:'+p.relative_to(W).as_posix()],cwd=W)==p.read_bytes(),'Uncommitted verification source')
    post=read(E/'postprocessing_protocol.json');P=read(E/'protocol.json');pins=read(E/'binaries.json');require(not (R/'collector.lock').exists(),'Collector active')
    protected=[str((E/n).relative_to(W)) for n in ('sequences','protocol.json','binaries.json','collect.py','build.py')];require(not git('diff','--name-only',post['raw_collection_commit'],'--',*protected),'Raw collection or source changed after numerical snapshot')
    historical=read(E/'historical_aggregate_sha256.json');require(len(historical)==743 and all(sha(W/n)==h for n,h in historical.items()),'Historical aggregate drift')
    new_names={str(Path('experiments/results')/(Path(m['path']).stem+'.json')) for m in P['manifests'].values()};require({str(p.relative_to(W)) for p in (W/'experiments/results').glob('*_matrix.json')}==set(historical)|new_names,'Unexpected aggregate set')
    new_runs={'experiments/results/runs/'+Path(m['path']).stem+'/' for m in P['manifests'].values()}
    for n in git('diff','--name-only',BASE,'--','experiments/results').splitlines():require(n.startswith(str(E.relative_to(W))+'/') or n in new_names or n=='experiments/results/index.json' or any(n.startswith(p) for p in new_runs),'Inherited result changed: '+n)
    publication={}
    for n in ('README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json'):require(before(n)==(W/n).read_bytes(),'Canonical publication changed: '+n);publication[n]=sha(W/n)
    n='docs/assets/paper/full_variant_results.csv';reader=csv.DictReader(io.StringIO(before(n).decode()));columns=reader.fieldnames;old=list(reader)
    with (W/n).open() as stream:
        reader=csv.DictReader(stream);require(reader.fieldnames==columns,'Variant columns changed');current=list(reader)
    keys=lambda rows:Counter(tuple(r[k] for k in columns) for r in rows)
    require(not (keys(old)-keys(current)),'Old variant rows changed');owned={read(W/m['path'])['problem']['id'] for m in P['manifests'].values()};extra=[r for r in current if r['problem_id'] in owned]
    require(len(current)==len(old)+22 and len(extra)==22 and all(r['is_current_default']=='false' and not r['fps'] for r in extra),'New variants promoted or timing ranked')
    sourcepaths=['common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**','CMakeLists.txt'];require(not git('diff','--name-only',P['native_source_commit'],'--',*sourcepaths),'Actual native source changed')
    binaries={}
    for item in pins['runner'],pins['spread_probe']:require(sha(item['path'])==item['sha256'],'Pinned binary changed');binaries[item['path']]=item['sha256']
    for n,h in pins['linked_static_libraries'].items():require(sha(n)==h,'Pinned library changed')
    report=read(E/'report/summary.json');require(report['exit_code']==0 and report['all_sequences_present'] and report['parent_reference_equal_all_sequences'],'Numerical report incomplete')
    configs=None;sequences={}
    for seq,meta in sorted(P['manifests'].items()):
        require(sha(W/meta['path'])==meta['sha256'],'Manifest changed');folder=E/'sequences'/seq;cleanup=read(folder/'cleanup.json');require(cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after'] and not Path(cleanup['owned_input_dir']).exists(),'Raw scans remain')
        inputs=read(folder/'inputs.json');previous=read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json');require(inputs['files']==previous['files'] and inputs['gt_csv_sha256']==previous['gt_csv_sha256'],'Official input hashes drift')
        configuration=read(folder/'effective_configuration.json')
        if configs is None:configs=configuration
        else:require(configs==configuration,'Configuration varies by sequence')
        for mode in ('baseline','candidate'):
            p=configuration[mode]['params'];require(p['s2m_plane_min_spread']==(.2 if mode=='candidate' else 0.) and p['accelerated'] and p['s2s_ground_pca'] and p['s2s_facade_pca'] and p['s2s_mapped_prior'] and p['s2s_translation_prior_ratio']==.03 and not p.get('s2s_row_support',False),'Wrong effective profile')
            s=read(folder/mode/'summary.json');r=read(folder/mode/'native_command.json');v=read(folder/mode/'trace_validation.json')
            require(r['exit_code']==0 and r['binary_sha256']==pins['runner']['sha256'] and r['native_source_commit']==P['native_source_commit'],'Wrong native producer/exit');require(s['methods'][0]['status']=='ok' and s['methods'][0]['frames']==inputs['scan_frames']==v['processed_frames'] and v['skipped_frames']==0,'Incomplete full run')
            require((W/'experiments/results/runs'/Path(meta['path']).stem/mode/'summary.json').read_bytes()==(folder/mode/'summary.json').read_bytes(),'Import changed native summary')
        a,b=[dict(configuration[m]['params']) for m in ('baseline','candidate')];a.pop('s2m_plane_min_spread');b.pop('s2m_plane_min_spread');require(a==b and configuration['baseline']['loader']==configuration['candidate']['loader'],'More than spread changes')
        compatibility=read(folder/'legacy_compatibility.json');require(compatibility['exit_code']==0 and compatibility['trace_body_exact_after_scan_root_substitution'],'Full baseline trace drift')
        old=W/'experiments/results/inten_facade_pca/sequences'/seq/'candidate/dogfooding_results'
        require(all((old/n).read_bytes()==(folder/'baseline/dogfooding_results'/n).read_bytes() for n in compatibility['saved_pose_gt_bytes_equal']),'Full baseline pose/GT drift')
        archive=read(folder/'archive.json');require(archive['byte_roundtrip_verified'],'Archive not roundtripped')
        for item in archive['files'].values():
            if item['storage']=='ordered_gzip_shards':
                for part in item['parts']:require(sha(folder/part['path'])==part['gzip_sha256'],'Archive part changed')
            else:require(sha(folder/item['path'])==item['stored_sha256'],'Archive file changed')
        aggregate=read(W/'experiments/results'/(Path(meta['path']).stem+'.json'));require(aggregate['status']=='ready' and all(v['fps'] is None and v['time_ms'] is None for v in aggregate['variants']),'Scored timing or incomplete aggregate')
        sequences[seq]=dict(frames=inputs['scan_frames'],native_exits=[0,0],reference_trace_pose_gt_exact=True,archive_roundtrip=True,owned_raw_absent=True)
    matched=read(E/'spread/report.json');require(matched['exit_code']==0 and matched['base_matches_immutable_native_controls_exact'] and matched['all_non_plane_initial_class_equations_exact'] and len(matched['cases'])==8,'Matched controls failed')
    boundary=read(E/'boundary/report.json');require(boundary['exit_code']==0 and boundary['trace_max_absolute_differences']['output']==0. and boundary['trace_max_absolute_differences']['continuous']==0.,'Current native-map reproduction failed')
    original=(W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text();expected='// Diagnostic-only producer; see spread_protocol.json.\ndouble diagnostic_map_plane_spread = 0.0;\n'+original.replace('params_.s2m_plane_min_spread','::diagnostic_map_plane_spread');require(expected==(E/'diagnostic_native.cpp').read_text(),'Diagnostic transform changed')
    for path in ('collection_execution/command.json','report_execution/command.json','import_execution/command.json','import_execution/result.json'):require(read(E/path)['exit_code']==0,'Failed workflow '+path)
    args.output.write_text(json.dumps(dict(exit_code=0,base_commit=BASE,historical_aggregate_count=len(historical),historical_aggregate_hashes_match=True,all_other_tracked_result_files_unchanged=True,new_aggregate_count=len(new_names),old_variant_rows_unchanged=True,new_variant_rows=len(extra),primary_publication_sha256=publication,binary_sha256=binaries,sequences=sequences,production_source_unchanged=True,reference_trace_numeric_bytes_after_scan_root_substitution_and_pose_gt_exact_all_sequences=True,matched_current_map_controls_exact=True,policy='Existing outside-paper option stays default off, no per-sequence adoption. Original native binary used in both full arms; diagnostic copy never contributes accuracy. All old scientific values preserved; raw paired timing unscored.'),indent=2)+'\n');print('Full-study preservation passed')
if __name__=='__main__':main()
