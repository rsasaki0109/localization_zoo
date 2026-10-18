"""Exercise the offline checker on committed traces and explicit synthetic corruption."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys,time
E=Path(__file__).resolve().parent;REPO=E.parents[2]
P=json.loads((E/'protocol.json').read_text())


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir',required=True,type=Path)
    a=parser.parse_args();out=a.output_dir.resolve();assert not out.exists();out.mkdir(parents=True)
    source=P['tool_source_commit']
    subprocess.run(['git','diff','--exit-code',source,'--','evaluation/scripts/check_inten_trace.py','tests/test_inten_trace_validation.py'],cwd=REPO,check=True)
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    inputs={}
    for folder in P['inputs']:
        for name in ['trace.jsonl','summary.json','command.json']:
            path=REPO/folder/name;inputs[str(path.relative_to(REPO))]=sha(path)
    receipts=[];valid=[];invalid=[]
    def run(name,paths,expected,reason=None):
        folder=out/name;folder.mkdir();report=folder/'report.json'
        cmd=[sys.executable,str(REPO/'evaluation/scripts/check_inten_trace.py'),'--trace',str(paths[0]),
             '--summary',str(paths[1]),'--receipt',str(paths[2]),'--output',str(report)]
        start=time.time()
        with (folder/'stdout.log').open('w') as stdout,(folder/'stderr.log').open('w') as stderr:
            result=subprocess.run(cmd,cwd=REPO,stdout=stdout,stderr=stderr)
        receipt=dict(command=cmd,cwd=str(REPO),tool_source_commit=source,recorded_head=head,
                     exit_code=result.returncode,expected_exit_code=expected,started_unix=start,finished_unix=time.time())
        (folder/'command.json').write_text(json.dumps(receipt,indent=2)+'\n');receipts.append(receipt)
        assert result.returncode==expected,(name,result.returncode)
        if expected:
            error=(folder/'stderr.log').read_text()
            assert 'invalid InTEn trace:' in error and reason in error,(name,error)
            assert not report.exists(),name
            invalid.append(dict(case=name,exit_code=result.returncode,expected_error=reason,no_report_created=True))
        else:
            data=json.loads(report.read_text());assert data['exit_code']==0
            valid.append(dict(case=name,exit_code=0,report=data))
    for name,folder in zip(['default','plane_check','skipped_input'],P['inputs']):
        d=REPO/folder;run(name,[d/'trace.jsonl',d/'summary.json',d/'command.json'],0)
    base=REPO/P['inputs'][0]
    records=[json.loads(line) for line in (base/'trace.jsonl').read_text().splitlines()]
    summary=json.loads((base/'summary.json').read_text());receipt=json.loads((base/'command.json').read_text())
    mutations=[('missing_end','incomplete'),('after_end','record after end'),('footer_count','end counters'),
               ('literal_nan','nonstandard JSON numeric literal'),('mixed_receipt','argv mismatch'),
               ('native_failure','not exit 0'),('summary_count','processed frame count')]
    for name,reason in mutations:
        rec,summ,rcpt=copy.deepcopy(records),copy.deepcopy(summary),copy.deepcopy(receipt)
        if name=='missing_end':rec.pop()
        elif name=='after_end':rec.append(rec[1])
        elif name=='footer_count':rec[-1]['processed_frames']+=1
        elif name=='literal_nan':rec[2]['s2s']['iterations'][0]['H'][0]=float('nan')
        elif name=='mixed_receipt':rcpt=json.loads((REPO/P['inputs'][1]/'command.json').read_text())
        elif name=='native_failure':rcpt['exit_code']=1
        elif name=='summary_count':summ['methods'][0]['frames']-=1
        folder=out/'synthetic'/name;folder.mkdir(parents=True)
        paths=[folder/name for name in ['trace.jsonl','summary.json','receipt.json']]
        paths[0].write_text(''.join(json.dumps(v)+'\n' for v in rec))
        paths[1].write_text(json.dumps(summ)+'\n');paths[2].write_text(json.dumps(rcpt)+'\n')
        run(name,paths,1,reason)
    assert all(sha(REPO/path)==digest for path,digest in inputs.items())
    data=dict(exit_code=0,tool_source_commit=source,recorded_head=head,input_sha256=inputs,
              source_inputs_unchanged=True,valid=valid,invalid=invalid,
              cli_exits=[v['exit_code'] for v in receipts],
              scope='Offline checker only, using committed parent traces. Synthetic rejection cases are data mutations, not odometry failures. No native process or data fetch occurs.')
    (out/'summary.json').write_text(json.dumps(data,indent=2,allow_nan=False)+'\n')
    print('Committed trace corpus and synthetic rejection checks passed.')


if __name__=='__main__':main()
