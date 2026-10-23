"""Freeze postprocessing, then commit every numeric result before prose."""
from pathlib import Path
import hashlib,importlib.util,json,os,shutil,subprocess,time
R=Path(__file__).resolve().parent;W=R/'worktree';E=W/'experiments/results/inten_map_plane_spread';PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1',MPLCONFIGDIR=str(R/'matplotlib'),XDG_CACHE_HOME=str(R/'cache'))
def read(p):return json.loads(p.read_text())
def write(p,v):p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def git(*a):return subprocess.check_output(['git',*a],cwd=W,text=True).strip()
def commit(message,paths):
    subprocess.run(['git','add','-f',*map(str,paths)],cwd=W,check=True);subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True);subprocess.run(['git','commit','-m',message],cwd=W,check=True)
def module():
    s=importlib.util.spec_from_file_location('archive',W/'experiments/results/inten_natural_traces/run_sequences.py');a=importlib.util.module_from_spec(s);s.loader.exec_module(a);return a
archival=module()
def execute(label,argv):
    stage=R/(label+'_execution');stage.mkdir();started=time.time();source=git('rev-parse','HEAD')
    with (stage/'stdout.log').open('xb') as out,(stage/'stderr.log').open('xb') as err:p=subprocess.run(argv,cwd=W,env=ENV,stdout=out,stderr=err)
    write(stage/'command.json',dict(command=argv,cwd=str(W),source_commit=source,started_unix=started,finished_unix=time.time(),exit_code=p.returncode,environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')}))
    dest=E/(label+'_execution')
    if dest.exists():
        if label!='import':raise RuntimeError('Unexpected execution evidence')
        for item in dest.iterdir():
            target=stage/item.name
            if not item.is_file() or target.exists():raise RuntimeError('Execution receipt collision')
            shutil.copyfile(item,target);assert sha(item)==sha(target)
        shutil.rmtree(dest)
    archival.archive(stage,dest);print(label,p.returncode,flush=True)
    if p.returncode:raise RuntimeError(label+' failed: retain all evidence')
def main():
    if (R/'collector.lock').exists() or read(R/'collector_command.json')['exit_code']!=0 or read(R/'collection_exit.json')['exit_code']!=0 or len(read(R/'active.json')['completed'])!=11 or git('status','--porcelain'):raise RuntimeError('Clean successful full collection required')
    raw=git('rev-parse','HEAD');stage=R/'collection_record';stage.mkdir()
    for a,b in [('collector_command.json','command.json'),('collector.stdout','stdout.log'),('collector.stderr','stderr.log'),('collection_exit.json','exit.json')]:shutil.copyfile(R/a,stage/b)
    archival.archive(stage,E/'collection_execution');archival.archive(R/'backups',E/'backup_execution')
    for label,names in [('setup_driver',['setup_command.json','setup.stdout','setup.stderr']),('spread_driver',['spread_driver.json','spread.stdout','spread.stderr']),('start_driver',['start_driver.json','start.stdout','start.stderr']),('immutable_storage',['historical_storage_dedup.json','historical_storage_dedup_command.json','dedup_immutable_storage.py','historical_storage_dedup_extra.json','historical_storage_dedup_extra_command.json','dedup_immutable_storage_extra.py','dedup_extra.stdout','dedup_extra.stderr'])]:
        f=R/(label+'_record');f.mkdir()
        for name in names:shutil.copyfile(R/name,f/name)
        archival.archive(f,E/'checks'/label)
    for a,b in [('report_draft.py','report.py'),('import_draft.py','import_results.py'),('verify_draft.py','verify.py'),('write_docs_draft.py','write_docs.py'),('readme_draft.md','readme_template.md'),('finalize_docs.py','finalize_docs.py')]:shutil.copyfile(R/a,E/b)
    post=dict(schema_version=1,raw_collection_commit=raw,preservation_base_commit='87fc51aa8fd4b6a1af7016ad03158787ec0b3ef7',native_source_commit=read(E/'protocol.json')['native_source_commit'],collection_protocol_commit=git('log','-1','--format=%H','--',str((E/'protocol.json').relative_to(W))),source_sha256={n:sha(E/n) for n in ['report.py','import_results.py','verify.py','write_docs.py','readme_template.md','finalize_docs.py']},environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')},report='All11 full native outcomes, every local bad label/newfailure and fixed-block error growth. Verify both profiles match inherited settings and only existing S2M spread0 vs0.2 differs; same immutable real native binary. Reconstruct S2S adaptive prior and every finite total damped update with relative backward error<=1e-10, retain conditioning-sensitive forward differences without treating them as drift. Full-run trajectory/seed/map feedback affects later S2S too. Matched actual-map terminal controls remain separate and never enter accuracy.',preservation='All743 preexisting aggregate hashes, all other inherited tracked evidence, all historical variant rows and canonical publications remain unchanged; add11 unscored experimental aggregates/22nondefault FPSblank rows. No winner adoption or native source changes. Both freshfullbaseline trace numeric bytes after declared scan-root-only substitution and exact poses/GT/metrics must match preceding159candidate on everysequence.',timing='Raw timings retained; concurrent paired processes shareCPUs, so aggregateFPS/time unavailable and benchmarkscore0.',calibration=read(E/'protocol.json')['selection'],order='Sources/protocol committed before numerical derivation. Commit complete report/import numbers before prose. Regenerate three required derivatives; if paperJSON unchanged its two conditional generators are notneeded. Run actualexitchecks and require complete fullNative/ROS2 CI at exact final head beforeReady. No merge.')
    write(E/'postprocessing_protocol.json',post);commit('Freeze full plane-spread reporting protocol and archive completed execution',[E])
    execute('report',[PY,str(E/'report.py'),'--output-dir',str(E/'report')]);execute('import',[PY,str(E/'import_results.py')])
    paths=[E,W/'experiments/results/index.json']
    for m in read(E/'protocol.json')['manifests'].values():
        stem=Path(m['path']).stem;paths.extend([W/'experiments/results'/(stem+'.json'),W/'experiments/results/runs'/stem])
    commit('Record every full S2M plane-spread outcome before numerical interpretation',paths);print('Numerical evidence committed',git('rev-parse','HEAD'),flush=True)
if __name__=='__main__':main()
