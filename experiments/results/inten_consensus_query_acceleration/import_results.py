"""Import complete native outputs without choosing per-sequence defaults."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import json
import shutil
import subprocess
import sys
import time

E=Path(__file__).resolve().parent
W=E.parents[2]
R=W.parent
PY=sys.executable
sys.path.insert(0,str(W/'evaluation/scripts'))
import run_experiment_matrix as matrix


def read(path):return json.loads(Path(path).read_text())
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def write(path,value):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2,allow_nan=False)+'\n')
def require(ok,message):
    if not ok:raise ValueError(message)


def main():
    for source in (Path(__file__).resolve(),E/'postprocessing_protocol.json'):
        if subprocess.check_output(['git','show','HEAD:'+str(source.relative_to(W))],cwd=W)!=source.read_bytes():
            raise ValueError('Postprocessing source must be committed before execution: '+str(source))
    post_plan=read(E/'postprocessing_protocol.json')
    protected=[str((E/name).relative_to(W)) for name in ('sequences','full_protocol.json','binaries_balanced.json','collect.py','build_balanced.py')]
    require(not subprocess.check_output(['git','diff','--name-only',post_plan['raw_collection_commit'],'--',*protected],cwd=W,text=True).strip(),'Original collection evidence changed after its final raw commit')
    require(not (R/'collector.lock').exists(),'collector owns the index')
    require(read(E/'collection_execution/command.json')['exit_code']==0,'collection incomplete')
    protocol=read(E/'full_protocol.json');pins=read(E/'binaries_balanced.json');host=read(E/'runtime.json')
    historical=read(E/'historical_aggregate_sha256.json')
    require(all(sha(W/name)==digest for name,digest in historical.items()),'historical aggregate changed before import')
    generated=datetime.now(timezone.utc).replace(microsecond=0).isoformat();created=[]
    for seq,meta in sorted(protocol['manifests'].items()):
        manifest_path=W/meta['path'];manifest=read(manifest_path);stem=manifest_path.stem
        aggregate=W/'experiments/results'/(stem+'.json')
        require(not aggregate.exists(),'refusing aggregate overwrite')
        variants=[]
        for variant in manifest['variants']:
            mode=variant['id'];source=E/'sequences'/seq/mode
            native=read(source/'summary.json');receipt=read(source/'native_command.json')
            require(receipt['exit_code']==0 and native['methods'][0]['status']=='ok','native run not valid')
            destination=W/'experiments/results/runs'/stem/mode
            destination.mkdir(parents=True,exist_ok=False)
            shutil.copyfile(source/'summary.json',destination/'summary.json')
            shutil.copyfile(source/'native_command.json',destination/'native_command.json')
            shutil.copytree(source/'dogfooding_results',destination/'dogfooding_results')
            (destination/'run.log').write_text('Actual native argv and exit: native_command.json\n'
                +'Lossless original stdout/stderr and complete trace: '+str(source.relative_to(W))+'/archive mapping in the sequence archive.json\n')
            runtime=dict(host,capture_scope='Shared Docker runtime captured before full paired runs, not an individual per-run timestamp.',
                native_environment=receipt['environment'],native_binary_sha256=pins['runner']['sha256'],
                measurement_context='Both fixed arms use the shared host with fixed baselineOMP1/candidateOMP4 and BLAS1. Raw timings retained; no standalone throughput comparison.')
            write(destination/'host.json',runtime)
            result=matrix.variant_result_from_summary(variant=variant,primary_method=manifest['stable_interface']['primary_method'],
                binary=Path(pins['runner']['path']),pcd_dir=W/manifest['problem']['dataset']['pcd_dir'],
                gt_csv=W/manifest['problem']['dataset']['gt_csv'],methods='inten_loam',dataset_extra_args=[],
                summary_path=destination/'summary.json',log_path=destination/'run.log')
            result.host=runtime
            raw_fps,raw_time=result.fps,result.time_ms
            # This experiment compares accuracy, not standalone throughput. Apply
            # the same unavailable marker to both arms, keeping raw native values.
            result.fps=None;result.time_ms=None
            result.decision='Keep as reference variant' if mode=='baseline' else 'Experimental; default off'
            result.decision_reason='One candidate setting is evaluated across all sequences. No per-sequence adoption or default selection; the native option remains off.'
            result.note+='; imported full paired experiment; timing is not a standalone speed measurement; actual argv in adjacent native_command.json'
            item=matrix.variant_result_to_dict(result)
            item['measurement_context']=dict(raw_fps=raw_fps,raw_time_ms=raw_time,
                standalone_timing_comparable=False,proxy_score_policy='Unscored, no FPS-based ranking or per-sequence default selection',
                original_summary_path=str((source/'summary.json').relative_to(W)),original_summary_sha256=sha(source/'summary.json'),
                native_command_path=str((destination/'native_command.json').relative_to(W)))
            require(sha(destination/'summary.json')==sha(source/'summary.json'),'import changed raw summary')
            variants.append(item)
        payload=dict(generated_at=generated,manifest_path=meta['path'],problem=manifest['problem'],stable_interface=manifest['stable_interface'],
            dataset=dict(pcd_dir=manifest['problem']['dataset']['pcd_dir'],gt_csv=manifest['problem']['dataset']['gt_csv'],extra_args=[]),
            variant_timeout_seconds=None,status='ready',blocker='',next_step='Review the uniform full-sequence experiment; repository extension stays default off.',variants=variants)
        write(aggregate,payload);created.append(str(aggregate.relative_to(W)))
    # The existing renderer reuses these complete aggregates, preserving the
    # explicit decision policy and never invoking a native binary or proxy rank.
    command=[PY,'evaluation/scripts/run_experiment_matrix.py','--reuse-existing','--reuse-aggregates','--merge-existing-index',
             '--output-dir','experiments/results','--docs-dir',str(R/'import_docs')]
    for seq,meta in sorted(protocol['manifests'].items()):command += ['--manifest',meta['path']]
    started=time.time();folder=E/'import_execution';folder.mkdir(parents=True,exist_ok=True)
    require(not (folder/'index_command.json').exists(),'index import already executed')
    with (folder/'index.log').open('xb') as out:
        p=subprocess.run(command,cwd=W,stdout=out,stderr=subprocess.STDOUT)
    write(folder/'index_command.json',dict(command=command,cwd=str(W),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),
        exit_code=p.returncode,started_unix=started,finished_unix=time.time()))
    require(p.returncode==0,'aggregate index reuse failed')
    require(all(sha(W/name)==digest for name,digest in historical.items()),'import changed an older aggregate')
    write(folder/'result.json',dict(exit_code=0,created_aggregates=created,historical_aggregates_unchanged=True,
        default_policy='All new variants remain non-default; no per-sequence winner is adopted.',
        timing_policy='Native summaries and raw times are byte-preserved. Aggregate timing fields are unavailable in every arm because unequal fixed worker counts on the shared host are not standalone speed measurements.'))


if __name__=='__main__':main()
