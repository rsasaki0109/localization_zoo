"""Complete backup archival after a retained wrapper-only storage failure."""
from pathlib import Path
import hashlib
import importlib.util
import json
import stat
import subprocess

E=Path(__file__).resolve().parent
W=E.parents[2]
R=W.parent
def read(p):return json.loads(p.read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def require(ok,message):
    if not ok:raise RuntimeError(message)
def committed(p):
    require(subprocess.check_output(['git','show','HEAD:'+str(p.relative_to(W))],cwd=W)==p.read_bytes(),'Recovery source must be committed: '+str(p))

def main():
    for p in (Path(__file__).resolve(),E/'collection_recovery_plan.json'):
        committed(p)
    require(not (R/'collector.lock').exists(),'Collector still owns the index')
    plan=read(E/'collection_recovery_plan.json')
    require(read(E/'collection_wrapper_failure/command.json')['exit_code']==1,'Actual wrapper failure not retained')
    require(read(E/'collection_execution/command.json')['exit_code']==0,'Underlying collector failed')
    status=read(R/'collection_exit.json')
    require(status['exit_code']==0 and len(status['completed'])==11,'Complete native collection required')
    require(sha(E/'run_collection.py')==plan['wrapper_source_sha256'],'Frozen wrapper source changed')
    archive_source=W/'experiments/results/inten_natural_traces/run_sequences.py'
    require(sha(archive_source)==plan['archive_source_sha256'],'Frozen archive source changed')
    for seq in status['completed']:
        folder=E/'sequences'/seq
        cleanup=read(folder/'cleanup.json')
        require(cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists(),'Owned scans not already cleaned: '+seq)
        for mode in ('baseline','candidate'):
            require(read(folder/mode/'native_command.json')['exit_code']==0,'Native failure: '+seq+'/'+mode)
    def preserved():
        for name,pin in plan['old_backup_files'].items():
            path=W/name
            require(sha(path)==pin['sha256'] and path.stat().st_size==pin['bytes'] and stat.S_IMODE(path.stat().st_mode)==pin['mode'],'Old backup archive changed: '+name)
    preserved()
    spec=importlib.util.spec_from_file_location('backup_archive',archive_source)
    archive=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(archive)
    target=E/'full_backup_execution'
    archive.archive(R/'backups',target)
    require(read(target/'archive.json')['byte_roundtrip_verified'],'Complete backup archive did not roundtrip')
    preserved()
    result=dict(exit_code=0,native_collection_exit_code=0,retained_original_wrapper_exit_code=1,
        complete_backup_archive=str(target.relative_to(W)),old_backup_files_unchanged=True,
        native_rerun=False,retuning=False,
        policy='The collector and every fixed full native arm already completed0. Preserve its original wrapper storage failure and all old backup bytes; archive all backup receipts to a fresh target without rerunning acquisition, native processing, validation or cleanup.')
    (E/'collection_recovery_result.json').write_text(json.dumps(result,indent=2)+'\n')
    print('Complete backup receipts archived; every old backup byte and all native evidence preserved')

if __name__=='__main__':main()
