"""Verify original results, matched full runs and opt-in/default boundaries."""
from pathlib import Path
from collections import Counter
import argparse
import csv
import hashlib
import io
import json
import subprocess

E=Path(__file__).resolve().parent
W=E.parents[2]
R=W.parent
BASE='e057e74619e870341a9b57ec0b13b0bd0ecd7f29'


def read(path):return json.loads(Path(path).read_text())
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def require(ok,message):
    if not ok:raise ValueError(message)
def before(name):return subprocess.check_output(['git','show',BASE+':'+name],cwd=W)


def main():
    for source in (Path(__file__).resolve(),E/'postprocessing_protocol.json'):
        if subprocess.check_output(['git','show','HEAD:'+str(source.relative_to(W))],cwd=W)!=source.read_bytes():
            raise ValueError('Postprocessing source must be committed before execution: '+str(source))
    post_plan=read(E/'postprocessing_protocol.json')
    protected=[str((E/name).relative_to(W)) for name in ('sequences','protocol.json','binaries.json','collect.py','build.py')]
    require(not subprocess.check_output(['git','diff','--name-only',post_plan['raw_collection_commit'],'--',*protected],cwd=W,text=True).strip(),'Original collection evidence changed after its final raw commit')
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    require(not (R/'collector.lock').exists(),'collection still active')
    protocol=read(E/'protocol.json');pins=read(E/'binaries.json')
    historical=read(E/'historical_aggregate_sha256.json')
    require(all(sha(W/name)==digest for name,digest in historical.items()),'historical aggregate changed')
    new_names={str(Path('experiments/results')/(Path(meta['path']).stem+'.json')) for meta in protocol['manifests'].values()}
    actual_names={str(p.relative_to(W)) for p in (W/'experiments/results').glob('*_matrix.json')}
    require(actual_names==set(historical)|new_names,'unexpected aggregate additions/removals')
    # Preserve every older tracked evidence file, including non-matrix results.
    # Only this new study, its imports and the shared aggregate index may change.
    new_runs={'experiments/results/runs/'+Path(m['path']).stem+'/' for m in protocol['manifests'].values()}
    changed=subprocess.check_output(['git','diff','--name-only',BASE,'--','experiments/results'],cwd=W,text=True).splitlines()
    for name in changed:
        allowed=(name.startswith(str(E.relative_to(W))+'/') or name in new_names or
                 name=='experiments/results/index.json' or any(name.startswith(prefix) for prefix in new_runs))
        require(allowed,'older scientific evidence changed: '+name)
    publication={}
    for name in ('README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json'):
        require(before(name)==(W/name).read_bytes(),'primary publication changed: '+name);publication[name]=sha(W/name)
    csvname='docs/assets/paper/full_variant_results.csv'
    old_reader=csv.DictReader(io.StringIO(before(csvname).decode()));old_rows=list(old_reader)
    with (W/csvname).open() as stream:
        reader=csv.DictReader(stream);new_rows=list(reader);require(reader.fieldnames==old_reader.fieldnames,'variant columns changed')
    def keys(rows):return Counter(tuple(row[k] for k in old_reader.fieldnames) for row in rows)
    old_counts,new_counts=keys(old_rows),keys(new_rows)
    require(not (old_counts-new_counts),'existing full-variant values changed')
    owned_ids={read(W/meta['path'])['problem']['id'] for meta in protocol['manifests'].values()}
    extra=[r for r in new_rows if r['problem_id'] in owned_ids]
    require(len(extra)==22 and len(new_rows)==len(old_rows)+22,'new variant coverage differs')
    require(all(r['is_current_default']=='false' and not r['fps'] for r in extra),'experiment promoted defaults or timing')
    source_paths=['common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**','CMakeLists.txt']
    require(not subprocess.check_output(['git','diff','--name-only',protocol['native_source_commit'],'--',*source_paths],cwd=W,text=True).strip(),'compiled native source changed after build')
    binaries={}
    for item in (pins['runner'],pins['probe'],protocol['parent_binary']):
        require(sha(item['path'])==item['sha256'],'pinned executable changed');binaries[item['path']]=item['sha256']
    for path,digest in pins['linked_static_libraries'].items():require(sha(path)==digest,'linked native dependency changed')
    for name in ('baseline','candidate'):
        item=read(W/'experiments/results/shared_near_range'/(name+'.json'))
        require(sha(item['binary'])==item['sha256'],'frozen shared-audit binary changed');binaries[item['binary']]=item['sha256']
    sequences={};configurations=None;pair_total=0
    for seq,meta in sorted(protocol['manifests'].items()):
        require(sha(W/meta['path'])==meta['sha256'],'frozen experiment manifest changed')
        manifest=read(W/meta['path']);folder=E/'sequences'/seq
        cleanup=read(folder/'cleanup.json');require(cleanup['hashes_verified_before_removal'] and not cleanup['input_dir_exists_after'] and not Path(cleanup['owned_input_dir']).exists(),'official scans remain')
        inputs=read(folder/'inputs.json');require(inputs['files']==read(W/'experiments/results/shared_near_range/sequences'/seq/'inputs.json')['files'],'official input hashes differ')
        current=read(folder/'effective_configuration.json')
        if configurations is None:configurations=current
        else:require(configurations==current,'effective settings differ across sequences')
        aggregate=read(W/'experiments/results'/(Path(meta['path']).stem+'.json'))
        archive=read(folder/'archive.json');require(archive['byte_roundtrip_verified'],'archive not validated')
        for mode in ('baseline','candidate'):
            source=folder/mode;summary=read(source/'summary.json');receipt=read(source/'native_command.json');validation=read(source/'trace_validation.json')
            require(receipt['exit_code']==validation['exit_code']==0 and validation['processed_frames']==inputs['scan_frames'] and validation['skipped_frames']==0,'native traversal incomplete')
            require(receipt['binary_sha256']==pins['runner']['sha256'],'wrong comparison binary')
            require(validation['trace_sha256']==archive['files'][mode+'/trace.jsonl']['sha256'],'trace archive/validator mismatch')
            imported=next(v for v in aggregate['variants'] if v['id']==mode)
            require(imported['kitti_rte_trans_pct']==summary['methods'][0]['kitti_rte_trans_pct'] and imported['frames']==summary['methods'][0]['frames'],'import altered accuracy')
            require(imported['decision']!='Adopt as current default' and imported['fps'] is None,'per-sequence adoption/timing claim')
        old_reference=W/'experiments/results/inten_mapped_prior_rigid/sequences'/seq/'candidate'
        require((folder/'baseline/dogfooding_results/InTEn_LOAM.txt').read_bytes()==(old_reference/'dogfooding_results/InTEn_LOAM.txt').read_bytes(),'full-sequence default-off saved pose drift: '+seq)
        original=read(old_reference/'summary.json')['methods'][0];fresh=read(folder/'baseline/summary.json')['methods'][0]
        require(all(original.get(k)==fresh.get(k) for k in ('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')),'full-sequence default-off metrics drift: '+seq)
        if seq in ('01','02','10'):
            control=read(folder/'default_pair_compatibility/result.json');require(control['exit_code']==0 and control['class_counts_exact'],'default-off compatibility failed');pair_total+=control['pairs']
        if seq=='07':require(read(folder/'legacy_compatibility.json')['exit_code']==0,'full07 legacy control failed')
        sequences[seq]=dict(frames=inputs['scan_frames'],native_exits=[0,0],owned_scans_absent=True,official_hashes_match=True)
    require(not subprocess.check_output(['git','diff','--name-only',protocol['prior_aborted_trial_commit'],'--',protocol['prior_aborted_trial']],cwd=W,text=True).strip(),'aborted initial trial changed')
    require(pair_total==360,'missing default-off pair controls')
    report=read(E/'report/summary.json');require(report['exit_code']==0 and report['all_sequences_present'],'full report incomplete')
    require(read(E/'collection_execution/command.json')['exit_code']==0 and read(E/'report_execution/command.json')['exit_code']==0,'collection/report failed')
    require(read(E/'import_execution/result.json')['exit_code']==0 and read(E/'import_execution/command.json')['exit_code']==0,'import failed')
    require(not subprocess.check_output(['git','diff','--name-only',BASE,'--','experiments/results/inten_natural_traces',
        'experiments/results/inten_seed_intervention','experiments/results/inten_plane_correspondences','experiments/results/shared_near_range','experiments/results/inten_row_support','experiments/results/inten_mapped_prior_rigid'],cwd=W,text=True).strip(),'parent evidence changed')
    result=dict(exit_code=0,base_commit=BASE,historical_aggregate_count=len(historical),historical_aggregate_hashes_match=True,all_other_tracked_result_files_unchanged=True,
        new_aggregate_count=len(new_names),reference_pose_bytes_equal_all_sequences=True,old_variant_rows_unchanged=True,new_variant_rows=len(extra),primary_publication_sha256=publication,
        binary_sha256=binaries,sequences=sequences,default_off_pair_controls=pair_total,
        policy='Native extension remains default off, no per-sequence winner adoption. Raw timing preserved but uniformly excluded from standalone aggregate FPS. All earlier scientific evidence and publication values are unchanged.')
    args.output.write_text(json.dumps(result,indent=2)+'\n');print('preservation passed',flush=True)


if __name__=='__main__':main()
