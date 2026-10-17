"""Aggregate committed whole-zoo replay checkpoints without changing old results."""
from support import *
import argparse
import csv
import io

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir', type=Path, required=True)
args = parser.parse_args()
selected = json.loads((E / 'selection.json').read_text())['rows']
rows = []
checks = []
revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
for seq in PROTOCOL['order']:
    folder = E / 'sequences' / seq
    for name in ('inputs.json', 'cleanup.json', 'audit/report.json', 'audit/audit_command.json'):
        relative = (folder / name).relative_to(REPO).as_posix()
        committed = subprocess.check_output(['git', 'show', revision + ':' + relative], cwd=REPO)
        assert committed == (folder / name).read_bytes(), 'Uncommitted execution evidence: ' + relative
    inputs = json.loads((folder / 'inputs.json').read_text())
    cleanup = json.loads((folder / 'cleanup.json').read_text())
    report = json.loads((folder / 'audit/report.json').read_text())
    command = json.loads((folder / 'audit/audit_command.json').read_text())
    assert report['exit_code'] == command['exit_code'] and report['exit_code'] in (0, 1)
    assert cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists()
    assert not (R / 'data/dogfooding_results' / f'kitti_seq_{seq}_full').is_symlink()
    assert inputs['matches_previous_official_inputs']
    if (folder / 'frozen_comparison.json').exists():
        relative = (folder / 'frozen_comparison.json').relative_to(REPO).as_posix()
        committed = subprocess.check_output(['git', 'show', revision + ':' + relative], cwd=REPO)
        comparison = json.loads(committed)
        assert comparison['exit_code'] == 0
        checks.extend(comparison['checks'])
    rows.extend(report['rows'])
expected = {(r['method'], r['sequence'], r['repo_best_aggregate'], r['repo_best_variant']) for r in selected}
actual = {(r['method'], r['sequence'], r['aggregate_path'], r['variant_id']) for r in rows}
assert actual == expected and len(rows) == len(selected)
drift = [r for r in rows if r['audit_status'] != 'ok']
controlled = {(r['method'], r['sequence']) for r in checks}
assert all((r['method'], r['sequence']) in controlled for r in drift)
summary = dict(implementation_commit=SOURCE, evidence_revision=revision,
               scope='Whole frozen best-RPE cell selection; historical per-cell arguments, no new setting selection.',
               completed_sequences=PROTOCOL['order'], selected_cell_count=len(selected),
               completed_cell_count=len(rows), missing=[], historical_metric_drift_cells=drift,
               frozen_comparisons=checks, rows=rows, exit_code=0,
               interpretation='Historical drift, if any, is retained and proven unchanged against the frozen base. No default regression or NCLT accuracy claim.')
write(args.output_dir / 'summary.json', summary)
columns = ['method', 'sequence', 'variant_id', 'audit_status', 'stored_rpe_trans_pct',
           'rerun_rpe_trans_pct', 'rerun_rpe_abs_delta', 'stored_ate_m', 'rerun_ate_m',
           'rerun_ate_abs_delta', 'frames', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m']
stream = io.StringIO()
writer = csv.DictWriter(stream, fieldnames=columns, extrasaction='ignore', lineterminator='\n')
writer.writeheader()
writer.writerows(rows)
(args.output_dir / 'comparison.csv').write_text(stream.getvalue())
print(json.dumps(dict(completed_cells=len(rows), historical_drift=len(drift), exit_code=0)))
