"""Stop only the verified owned slow07 candidate after exact acceleration controls."""
from pathlib import Path
import hashlib,json,os,signal,subprocess,time
E=Path(__file__).resolve().parent;W=E.parents[2]
OLD=Path('/workspace/localization-env/inten-recursive-consensus')
def read(p):return json.loads(p.read_text())

def main():
    require=read(E/'checks/cached_parity/result.json')
    assert require['exit_code']==0 and require['bitwise_worker_count_parity']
    assert read(E/'checks/cached_parity/decision_verification.json')['exit_code']==0
    state=read(OLD/'active.json');assert state['status']=='native' and state['sequence']=='07' and state['completed']==['01']
    original=read(OLD/'worktree/experiments/results/inten_recursive_consensus/binaries.json')['runner']
    assert hashlib.sha256(Path(original['path']).read_bytes()).hexdigest()==original['sha256']
    expected=str(OLD/'full_staging/07/candidate/summary.json');raw=str(OLD/'kitti/dataset/sequences/07/velodyne')
    pids=subprocess.check_output(['ps','-C','pcd_dogfooding','-o','pid='],text=True).split()
    found=[]
    for value in pids:
        pid=int(value)
        try:args=[p.decode() for p in (Path('/proc')/value/'cmdline').read_bytes().split(b'\0') if p]
        except FileNotFoundError:continue
        if args and args[0]==original['path'] and expected in args and raw in args:found.append(pid)
    assert len(found)==1,found
    request=dict(observed_unix=time.time(),command=['python',str(Path(__file__).resolve())],cwd=str(W),
       acceleration_source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),
       reason='Engineering exact-query acceleration after native tests and bitwise full cached profile/solver controls; no accuracy setting, threshold or rule changes. Preserve slow full01 negative and require its full numerical equality in the fresh uniform producer.',
       owned_candidate_pid=found[0],matched_summary=expected,matched_raw=raw,signal='SIGINT',
       preserve_all_partial_outputs_and_actual_nonzero_exit=True,no_raw_deletion=True,
       original_full01_commit='bd3724fd5aa82513c1784d249a7cf6133df5b95b')
    path=OLD/'acceleration_pause_request.json'
    with path.open('x') as out:out.write(json.dumps(request,indent=2)+'\n')
    os.kill(found[0],signal.SIGINT)
    print('SIGINT sent only to verified owned slow07 candidate; wait for actual collector failure and archive all evidence')

if __name__=='__main__':main()
