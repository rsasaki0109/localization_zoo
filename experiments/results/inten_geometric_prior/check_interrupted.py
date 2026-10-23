#!/usr/bin/env python3
"""Audit incomplete transport attempts without turning prefixes into accuracy claims."""
import argparse
import gzip
import json
from pathlib import Path
import numpy as np
from receipt import save, sha
from run_sequences import archived_frames


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args(); study=Path(__file__).resolve().parent
    sequences=json.loads((study/'protocol.json').read_text())['sequence_order']
    sources={}
    for sequence in sequences:
        original=a.source_root/'experiments/results/inten_facade_pca/sequences'/sequence
        digest=sha(original/'inputs.json')
        assert digest not in sources
        sources[digest]=(sequence,original)
    rows=[]
    for folder in sorted((study/'preflight_failures').iterdir()):
        if not folder.is_dir():continue
        command=json.loads((folder/'native_command.json').read_text())
        assert command['exit_code']!=0
        sequence,original=sources[command['input_manifest_sha256']]
        trace=original/'candidate/trace.jsonl.parts'
        assert command['trace_shards']=={f.name:sha(f) for f in sorted(trace.glob('*.gz'))}
        pins=json.loads((original/'inputs.json').read_text())['files']
        inputs=[json.loads(line) for line in (folder/'inputs.jsonl').read_text().splitlines()]
        with gzip.open(folder/'native.jsonl.gz','rt') as stream:
            native=[json.loads(line) for line in stream]
        poses={name:[list(map(float,line.split())) for line in (folder/(name+'.txt')).read_text().splitlines()]
               for name in ['reference','candidate']}
        count=command['frames_completed']
        assert len(native)==len(inputs)==len(poses['reference'])==len(poses['candidate'])==count
        maximum=0.0; checked=0
        for i,(row,raw,old) in enumerate(zip(native,inputs,archived_frames(original/'candidate/trace.jsonl.parts'))):
            assert row['frame']==raw['frame']==old['input_index']==i
            pin=pins[f'{i:06d}.bin']
            assert raw['sha256']==pin['sha256'] and raw['bytes']==pin['bytes']
            error=float(np.max(np.abs(np.array(row['reference']['pose'])-np.array(old['pose']))))
            assert error==raw['baseline_max_abs_error'] and error<=1e-8
            maximum=max(maximum,error)
            for name in poses:
                expected=np.array(row[name]['pose']).reshape(4,4)[:3].flatten()
                assert np.isfinite(expected).all() and np.array_equal(expected,np.array(poses[name][i]))
            checked+=1
        assert checked==count
        assert maximum==command['maximum_baseline_matrix_error']
        failed=[f for f in folder.glob('fetch_*.json') if json.loads(f.read_text())['exit_code']!=0]
        assert failed
        rows.append({'attempt':folder.name,'sequence':sequence,'frames_completed':count,
                     'estimator_prefix_available':count>0,
                     'maximum_baseline_matrix_error':maximum,'native_exit_code':command['exit_code'],
                     'failed_fetch_commands':len(failed),'hashes_order_and_pose_exports_valid':True,
                     'files':{f.name:sha(f) for f in sorted(folder.iterdir()) if f.is_file()}})
    assert rows
    save(a.output,{'all_prefix_controls_valid':True,'full_sequence_accuracy_claim':False,'attempts':rows})


if __name__=='__main__':main()
