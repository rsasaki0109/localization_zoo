from pathlib import Path
import json,sys
E=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(E))
from support import *
verify_binaries()
folder=E/'sequences/01';audit=json.loads((folder/'audit/report.json').read_text());command=json.loads((folder/'audit/audit_command.json').read_text());inputs=json.loads((folder/'inputs.json').read_text())
selected=[r for r in json.loads((E/'selection.json').read_text())['rows'] if r['sequence']=='01']
assert audit['exit_code']==command['exit_code']==1 and command['implementation_commit']==SOURCE
assert len(audit['rows'])==len(selected)
assert {(r['method'],r['aggregate_path'],r['variant_id']) for r in audit['rows']}=={(r['method'],r['repo_best_aggregate'],r['repo_best_variant']) for r in selected}
expected=json.loads((E/'candidate.json').read_text());checked=[]
for row in audit['rows']:
 assert row['audit_status'] in ('ok','drift')
 assert row['process_exit_code']==0 and row['frames']==inputs['scan_frames']==inputs['gt_frames']
 summary=Path(row['summary_path']);native=json.loads(summary.with_name(summary.stem+'_native_command.json').read_text())
 assert native['exit_code']==0 and native['binary_sha256']==expected['sha256']
 assert native['command']==[expected['binary'],*row['command'][1:]]
 assert native['environment']=={'OMP_NUM_THREADS':'1','OPENBLAS_NUM_THREADS':'1'}
 checked.append(dict(method=row['method'],status=row['audit_status'],summary_sha256=digest(summary),native_receipt_sha256=digest(summary.with_name(summary.stem+'_native_command.json'))))
assert inputs['matches_previous_official_inputs']
assert not (folder/'cleanup.json').exists()
assert not (folder/'frozen_comparison.json').exists()
assert not any(p.suffix in ('.bin','.pcd') for p in (folder/'audit').rglob('*') if p.is_file())
closed_hashes={p.relative_to(REPO).as_posix():digest(p) for p in sorted((folder/'audit').rglob('*')) if p.is_file()}
write(E/'checks/01_primary/verification.json',dict(closed_audit_file_sha256=closed_hashes,command=[sys.executable,*sys.argv],implementation_commit=SOURCE,completed_primary_cells=len(checked),historical_matching_cells=sum(r['status']=='ok' for r in checked),historical_drift_cells=sum(r['status']=='drift' for r in checked),historical_audit_exit_code=1,primary_native_process_exits_all_zero=True,cleanup_complete=False,frozen_comparison_complete=False,checked_summaries=checked,exit_code=0,scope='Completed primary runs only; frozen comparison and owned input cleanup are in progress, so01 is not a completed sequence checkpoint.'))
print('Completed primary evidence verified; frozen comparison and cleanup remain pending.')
