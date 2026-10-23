"""Schedule the unchanged complete group checks in fixed, memory-bounded pairs."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import hashlib,importlib.util,json,os,subprocess,sys,tempfile,time
E=Path(__file__).resolve().parent;W=E.parents[2]
s=importlib.util.spec_from_file_location('support_ordered_parallel_base',E/'verify_ordered.py')
base=importlib.util.module_from_spec(s);s.loader.exec_module(base)
checker=base.checker;inputs=base.inputs;sum3=base.sum3
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def bindings():
    protocol=json.loads((E/'verification_schedule_protocol.json').read_text())
    for name,digest in protocol['source_sha256'].items():assert sha(W/name)==digest,('Verification scheduler source changed',name)
    return protocol
def worker(group,output):
    bindings()
    result=checker.group_result(group)
    output.write_text(json.dumps(dict(group=group,result=result,producer_sha256=sha(Path(__file__)),
        native_archive_sha256=sha(E/'groups'/group/'archive.json')),indent=2,allow_nan=False)+'\n')
def validate_in(folder):
    protocol=bindings();folder.mkdir(parents=True,exist_ok=True)
    receipts={}
    def launch(group):
        output=folder/(group+'.json')
        command=[sys.executable,str(Path(__file__).resolve()),'--group',group,'--output',str(output)]
        env=dict(os.environ,OPENBLAS_NUM_THREADS='1',OMP_NUM_THREADS='1',PYTHONDONTWRITEBYTECODE='1')
        start=time.time()
        with (folder/(group+'.stdout.log')).open('xb') as out,(folder/(group+'.stderr.log')).open('xb') as err:
            run=subprocess.run(command,cwd=W,env=env,stdout=out,stderr=err)
        receipt=dict(command=command,cwd=str(W),exit_code=run.returncode,started_unix=start,finished_unix=time.time(),
            environment={'OPENBLAS_NUM_THREADS':'1','OMP_NUM_THREADS':'1'},
            output_sha256=sha(output) if output.exists() else None,producer_sha256=sha(Path(__file__)))
        (folder/(group+'_command.json')).write_text(json.dumps(receipt,indent=2)+'\n')
        return group,receipt
    for pair in protocol['fixed_pairs']:
        with ThreadPoolExecutor(max_workers=2) as pool:
            for group,receipt in pool.map(launch,pair):receipts[group]=receipt
        assert all(receipts[g]['exit_code']==0 for g in pair),('Full group verification failed',pair,str(folder))
    results={}
    for group in ('old','01','02','08'):
        receipt=receipts[group];output=folder/(group+'.json')
        assert receipt['producer_sha256']==sha(Path(__file__)) and sha(output)==receipt['output_sha256']
        payload=json.loads(output.read_text());assert payload['group']==group and payload['producer_sha256']==sha(Path(__file__))
        assert payload['native_archive_sha256']==sha(E/'groups'/group/'archive.json')
        results[group]=payload['result']
    original=checker.group_result
    def cached_group(group):return results[group]
    checker.group_result=cached_group
    try:
        # Every original global source/audit/build/input check and roll-up still
        # runs, after every original complete group checker has really run.
        return base.validate()
    finally:checker.group_result=original
def validate():
    with tempfile.TemporaryDirectory(prefix='inten_support_verification_') as folder:
        return validate_in(Path(folder))
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--group',choices=('old','01','02','08'))
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if a.group:worker(a.group,a.output)
    else:
        result=validate_in(a.output.parent/'workers')
        a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
        print(json.dumps({k:v for k,v in result.items() if k!='groups'}))
