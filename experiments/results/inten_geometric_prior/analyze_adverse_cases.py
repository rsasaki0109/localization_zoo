#!/usr/bin/env python3
"""Read predeclared stage diagnostics; report sequence completion separately."""
import argparse,gzip,json,sys
from pathlib import Path
import numpy as np
from receipt import save,sha
p=argparse.ArgumentParser(__doc__)
p.add_argument('--source-root',type=Path,required=True)
p.add_argument('--attempt',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();study=Path(__file__).resolve().parent
sys.path.insert(0,str(a.source_root/'evaluation/scripts'))
from evaluate_external_kitti_odometry import load_reference_csv
command=json.loads((a.attempt/'native_command.json').read_text())
with gzip.open(a.attempt/'native.jsonl.gz','rt') as f:rows=[json.loads(line) for line in f]
assert len(rows)==command['frames_completed'] and len(rows)>420
assert [row['frame'] for row in rows]==list(range(len(rows)))
assert command['maximum_baseline_matrix_error']<=1e-8
truth=a.source_root/'experiments/reference_data/kitti_seq_01_full_gt.csv'
gt=load_reference_csv(truth);cases=[]
def ratio(delta,length):return float(np.linalg.norm(np.asarray(delta).reshape(4,4)[:3,3])/length)
for frame in [202,215,420]:
    delta=np.linalg.inv(gt[frame-1])@gt[frame];length=float(np.linalg.norm(delta[:3,3]))
    assert length>=.1
    case={'frame':frame,'gt_step_m':length}
    for name in ['reference','candidate']:
        row=rows[frame][name]
        final=np.linalg.inv(np.asarray(rows[frame-1][name]['pose']).reshape(4,4))@np.asarray(row['pose']).reshape(4,4)
        case[name]={'s2s_input_ratio':ratio(row['s2s_input'],length),
                    's2s_output_ratio':ratio(row['s2s_output'],length),
                    'final_step_ratio':ratio(final,length),
                    'final_length_error_over_20_percent':abs(ratio(final,length)-1)>.2,
                    's2s_fallback':row['s2s_fallback'],'s2m_accepted':row['s2m_accepted'],
                    'geometric_prior_used':row['geometric_prior_used']}
    cases.append(case)
complete=command['exit_code']==0 and len(rows)==len(gt)
scope=('Predeclared 01 adverse cases in a complete continuous trial. Stage diagnostics only; full-sequence metrics are reported separately. No all-sequence or retention-gate claim.' if complete else
       'Predeclared 01 adverse cases in an incomplete continuous trial. No full-sequence RTE, ATE, all-sequence or retention-gate claim.')
save(a.output,{'scope':scope,'sequence_complete':complete,
    'selection':'Frames 202, 215 and 420 named in protocol.json before the candidate was run. No post-result retuning.',
    'attempt':str(a.attempt),'frames_available':len(rows),'native_exit_code':command['exit_code'],
    'definition':'Ratio is norm of local translation / norm of GT local translation; destination-frame index.',
    'protocol_sha256':sha(study/'protocol.json'),'native_sha256':sha(a.attempt/'native.jsonl.gz'),
    'gt_sha256':sha(truth),'cases':cases})
