"""Report only committed, cleaned sequence checkpoints; never count live runs."""
from support import *
import argparse
import csv
import io
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
assert not args.output_dir.exists(), 'Use a fresh directory to preserve previous checkpoints.'
revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
selection = json.loads((E / 'selection.json').read_text())['rows']
rows = []
controls = []
completed = []
sequence_exits = {}
for seq in PROTOCOL['order']:
    folder = E / 'sequences' / seq
    path = (folder / 'cleanup.json').relative_to(REPO).as_posix()
    exists = subprocess.run(['git', 'cat-file', '-e', revision + ':' + path],
                            cwd=REPO, capture_output=True)
    if exists.returncode:
        break

    def committed(name):
        path = (folder / name).relative_to(REPO).as_posix()
        raw = subprocess.check_output(['git', 'show', revision + ':' + path], cwd=REPO)
        assert raw == (folder / name).read_bytes(), 'Modified committed checkpoint: ' + path
        return json.loads(raw)

    cleanup = committed('cleanup.json')
    inputs = committed('inputs.json')
    audit = committed('audit/report.json')
    command = committed('audit/audit_command.json')
    assert cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists()
    assert inputs['matches_previous_official_inputs']
    assert command['implementation_commit'] == SOURCE
    assert command['exit_code'] == audit['exit_code'] and audit['exit_code'] in (0, 1)
    expected = {(r['method'], r['repo_best_aggregate'], r['repo_best_variant'])
                for r in selection if r['sequence'] == seq}
    actual = {(r['method'], r['aggregate_path'], r['variant_id']) for r in audit['rows']}
    assert actual == expected
    if (folder / 'frozen_comparison.json').exists():
        comparison = committed('frozen_comparison.json')
        assert comparison['exit_code'] == 0
        for check in comparison['checks']:
            assert check['saved_pose_bytes_equal'] and all(check['saved_pose_bytes_equal'].values())
            assert all(value <= 1e-9 for value in check['metric_abs_delta'].values())
        controls.extend(comparison['checks'])
    rows.extend(audit['rows'])
    completed.append(seq)
    sequence_exits[seq] = audit['exit_code']

assert completed, 'No committed completed checkpoint.'
drift = [r for r in rows if r['audit_status'] == 'drift']
assert all(r['audit_status'] in ('ok', 'drift') and r['process_exit_code'] == 0 for r in rows)
controlled = {(r['method'], r['sequence']) for r in controls}
assert all((r['method'], r['sequence']) in controlled for r in drift)
report = dict(command=[sys.executable, *sys.argv],
              cwd=str(REPO), evidence_revision=revision, implementation_commit=SOURCE,
              completed_sequences=completed, completed_cell_count=len(rows),
              planned_cell_count=len(selection), historical_matching_cell_count=len(rows) - len(drift),
              historical_drift_cell_count=len(drift), audit_exit_codes=sequence_exits,
              remaining_sequences=[s for s in PROTOCOL['order'] if s not in completed],
              whole_zoo_audit_complete=len(rows) == len(selection),
              historical_drift_cells=drift, frozen_comparisons=controls, rows=rows,
              exit_code=0, scope='Committed checkpoint verification only; native audit exits are retained separately. Not whole-zoo completion while remaining cells exist.')
write(args.output_dir / 'summary.json', report)
columns = ['method', 'sequence', 'variant_id', 'audit_status', 'stored_rpe_trans_pct',
           'rerun_rpe_trans_pct', 'stored_ate_m', 'rerun_ate_m', 'frames']
stream = io.StringIO()
writer = csv.DictWriter(stream, fieldnames=columns, extrasaction='ignore', lineterminator='\n')
writer.writeheader()
writer.writerows(rows)
(args.output_dir / 'comparison.csv').write_text(stream.getvalue())
print(json.dumps({key: report[key] for key in ('completed_cell_count',
      'historical_matching_cell_count', 'historical_drift_cell_count', 'whole_zoo_audit_complete')}))
