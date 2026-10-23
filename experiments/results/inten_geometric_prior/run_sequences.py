#!/usr/bin/env python3
"""Fetch bounded batches from one sequence, preserve native state, archive all outcomes."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import gzip
import json
from pathlib import Path
import struct
import subprocess
import sys
import time
from receipt import save,sha


def archived_frames(folder):
    buffer=b''
    for path in sorted(folder.glob('*.gz')):
        with gzip.open(path,'rb') as f:
            for chunk in iter(lambda:f.read(1024*1024),b''):
                buffer+=chunk
                lines=buffer.split(b'\n');buffer=lines.pop()
                for line in lines:
                    row=json.loads(line)
                    if row['type']=='frame':yield row
    if buffer.strip():raise ValueError('unterminated archived trace')


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--scratch',type=Path,required=True)
    p.add_argument('--sequence',action='append')
    p.add_argument('--commit-results',action='store_true')
    a=p.parse_args();study=Path(__file__).resolve().parent
    sys.path.insert(0,str(a.source_root/'evaluation/scripts'))
    import numpy as np
    from evaluate_external_kitti_odometry import load_kitti_poses,load_reference_csv
    from diagnose_kitti_steps import diagnose
    protocol=json.loads((study/'protocol.json').read_text())
    binary=json.loads((study/'binaries.json').read_text())
    source_commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
    sequences=a.sequence or protocol['sequence_order']
    for seq in sequences:
        assert seq in protocol['sequence_order']
        out=study/'sequences'/seq;out.mkdir(parents=True,exist_ok=False)
        original=a.source_root/'experiments/results/inten_facade_pca/sequences'/seq
        pins=json.loads((original/'inputs.json').read_text())['files']
        names=sorted(k for k in pins if k.endswith('.bin'))
        assert names==[f'{i:06d}.bin' for i in range(len(names))]
        trace_dir=original/'candidate/trace.jsonl.parts'
        reference=iter(archived_frames(trace_dir))
        rawroot=a.scratch/seq;scanroot=rawroot/'dataset/sequences'/seq/'velodyne'
        batches=[list(range(start,min(start+64,len(names)))) for start in range(0,len(names),64)]
        def fetch(indices):
            for attempt in range(20):
                missing=[i for i in indices if not (scanroot/names[i]).exists() or sha(scanroot/names[i])!=pins[names[i]]['sha256']]
                if not missing:return
                argv=[sys.executable,str(a.source_root/'evaluation/scripts/fetch_kitti_odometry_sequences.py'),
                      '--output-root',str(rawroot),'--sequence',seq,'--workers','4']
                for i in missing:argv+=['--frame',str(i)]
                r=subprocess.run(argv,capture_output=True,text=True)
                save(out/f'fetch_{indices[0]:06d}_{attempt}.json',{'argv':argv,'cwd':str(Path.cwd()),
                    'source_commit':source_commit,'exit_code':r.returncode,'stdout':r.stdout,'stderr':r.stderr})
                if r.returncode:
                    print(f'sequence {seq}: acquisition retry {attempt+1} after failure',flush=True)
                    time.sleep(min(30,2**min(attempt,5)))
            missing=[i for i in indices if not (scanroot/names[i]).exists() or sha(scanroot/names[i])!=pins[names[i]]['sha256']]
            if missing:raise RuntimeError(f'incomplete batch {seq}: {missing}')
        argv=['docker','exec','-i','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1',
              'localization-zoo-dev',binary['runner_container_path']]
        command={'argv':argv,'cwd':str(Path.cwd()),'native_source_commit':binary['source_commit'],
                 'controller_source_commit':source_commit,'binary_sha256':binary['runner_sha256'],
                 'input_manifest_sha256':sha(original/'inputs.json'),
                 'trace_shards':{f.name:sha(f) for f in sorted(trace_dir.glob('*.gz'))},'exit_code':None}
        save(out/'native_command.json',command)
        max_error=0.0; count=0; runtimes={'reference':0.0,'candidate':0.0}
        with (out/'native.stderr').open('wb') as err, gzip.open(out/'native.jsonl.gz','wb') as rawout, \
             (out/'inputs.jsonl').open('w') as inputs, (out/'reference.txt').open('w') as refposes, \
             (out/'candidate.txt').open('w') as candposes, ThreadPoolExecutor(max_workers=1) as pool:
            process=subprocess.Popen(argv,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=err)
            try:
                future=pool.submit(fetch,batches[0])
                for b,indices in enumerate(batches):
                    future.result()
                    if b+1<len(batches):future=pool.submit(fetch,batches[b+1])
                    for i in indices:
                        scan=scanroot/names[i];data=scan.read_bytes()
                        digest=__import__('hashlib').sha256(data).hexdigest()
                        assert digest==pins[names[i]]['sha256'] and len(data)==pins[names[i]]['bytes']
                        process.stdin.write(struct.pack('<II',i,len(data)//16)+data);process.stdin.flush()
                        line=process.stdout.readline()
                        if not line:raise RuntimeError('native output ended early')
                        rawout.write(line);row=json.loads(line)
                        assert row['frame']==i
                        old=next(reference);assert old['input_index']==i
                        error=float(np.max(np.abs(np.array(row['reference']['pose'])-np.array(old['pose']))))
                        max_error=max(max_error,error)
                        if error>1e-8:raise RuntimeError(f'baseline mismatch at {seq}:{i}: {error}')
                        inputs.write(json.dumps({'frame':i,'sha256':digest,'bytes':len(data),'baseline_max_abs_error':error})+'\n')
                        for name,stream in [('reference',refposes),('candidate',candposes)]:
                            pose=np.array(row[name]['pose']).reshape(4,4)
                            stream.write(' '.join(format(x,'.17g') for x in pose[:3].flat)+'\n')
                            runtimes[name]+=row[name+'_seconds']
                        count+=1;scan.unlink()
                    rawout.flush();inputs.flush();refposes.flush();candposes.flush()
                    print(f'sequence {seq}: frame {indices[-1]} complete',flush=True)
                process.stdin.close();code=process.wait()
                assert code==0 and count==len(names)
                assert next(reference,None) is None
            except BaseException:
                process.kill();process.wait();raise
            finally:
                command['exit_code']=process.returncode;command['frames_completed']=count
                command['maximum_baseline_matrix_error']=max_error
                save(out/'native_command.json',command)
        gtpath=a.source_root/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv'
        gt=load_reference_csv(gtpath)
        summary={'sequence':seq,'frames':count,'controls_valid':max_error<=1e-8,'native_exit_code':code,
                 'runtime_seconds':runtimes,'gt_sha256':sha(gtpath),'methods':{}}
        for name in runtimes:
            summary['methods'][name]=diagnose(load_kitti_poses(out/(name+'.txt')),gt)
        save(out/'summary.json',summary)
        save(out/'archive.json',{f.name:{'sha256':sha(f),'bytes':f.stat().st_size} for f in out.iterdir() if f.is_file()})
        # Raw scans were deleted only after hash/order/control verification;
        # all per-frame/native evidence is retained, then committed per sequence.
        if a.commit_results:
            subprocess.run(['git','add',str(out)],check=True)
            subprocess.run(['git','commit','-m',f'Archive fixed geometric-prior full KITTI {seq} trial'],check=True,stdout=subprocess.DEVNULL)
        print(f'sequence {seq}: evidence complete',flush=True)
    summaries=[json.loads((study/'sequences'/seq/'summary.json').read_text()) for seq in protocol['sequence_order']
               if (study/'sequences'/seq/'summary.json').exists()]
    if len(summaries)==len(protocol['sequence_order']):
        values={name:[s['methods'][name]['official_metrics_from_saved_poses']['kitti_rte_trans_pct'] for s in summaries]
                for name in ['reference','candidate']}
        means={name:float(np.exp(np.mean(np.log(v)))) for name,v in values.items()}
        failures=[s['sequence'] for s,r,c in zip(summaries,values['reference'],values['candidate']) if c>max(1.10*r,r+.05)]
        save(study/'summary.json',{'all_sequences_complete':True,'controls_valid':all(s['controls_valid'] for s in summaries),
            'geometric_mean_rte_pct':means,'regression_gate_failed_sequences':failures,
            'retention_gate_passed':means['candidate']<means['reference'] and not failures,
            'sequences':summaries})


if __name__=='__main__':
    main()
