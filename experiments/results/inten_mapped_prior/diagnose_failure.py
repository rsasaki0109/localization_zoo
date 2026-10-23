"""Report geometric drift and missing completion from the original failed trace."""
from pathlib import Path
import argparse,csv,json,math,subprocess
import numpy as np

W=Path(__file__).resolve().parents[3]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--trace',type=Path,required=True);p.add_argument('--output-dir',type=Path,required=True)
a=p.parse_args()
if subprocess.check_output(['git','show','HEAD:'+Path(__file__).resolve().relative_to(W).as_posix()],cwd=W)!=Path(__file__).read_bytes():raise ValueError('Commit diagnostic source before execution')
a.output_dir.mkdir(parents=True,exist_ok=False)
rows=[];end=None;first_nonfinite=None
with a.trace.open() as stream:
 for line in stream:
  v=json.loads(line)
  if v['type']=='end':end=v;continue
  if v['type']!='frame':continue
  i=v['frame_index'];row={'frame':i}
  for name,values in [('pose',v['pose'])]+([] if v['initialization'] else [(stage+'_'+position,v[stage][position]) for stage in ('s2s','s2m') for position in ('input','output')]):
   matrix=np.asarray(values,dtype=float).reshape(4,4);finite=bool(np.all(np.isfinite(matrix)))
   if not finite and first_nonfinite is None:first_nonfinite={'frame':i,'field':name}
   with np.errstate(over='ignore',invalid='ignore'):
    orthogonality=float(np.linalg.norm(matrix[:3,:3].T@matrix[:3,:3]-np.eye(3)))
   row[name+'_finite']=finite;row[name+'_orthogonality_error']=orthogonality if math.isfinite(orthogonality) else None
  rows.append(row)
fields=['frame']+sorted({k for row in rows for k in row if k!='frame'})
with (a.output_dir/'rotation_drift.csv').open('w') as stream:
 writer=csv.DictWriter(stream,fieldnames=fields,lineterminator='\n');writer.writeheader();writer.writerows(rows)
result=dict(diagnostic_exit_code=0,trace_complete=end is not None,completed_frame_records=len(rows),last_completed_frame=rows[-1]['frame'],first_nonfinite_matrix=first_nonfinite,
    checkpoints=[row for row in rows if row['frame'] in (0,500,1000,1500,2000,rows[-1]['frame'])],
    convention='Frobenius norm of R.T R - I; raw full-matrix SE3 feedback can propagate non-rigid roundoff that left rigid updates do not remove. No accuracy score is estimated from this partial traversal.')
(a.output_dir/'summary.json').write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
print('failed-trace geometric diagnosis saved')
