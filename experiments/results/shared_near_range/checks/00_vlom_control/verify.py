"""Verify the closed VLOM control without reading the active Mesh-LOAM output."""
import json
from pathlib import Path
import subprocess
import sys

E = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(E))
from support import REPO, SOURCE, digest, native_output, verify_binaries, write

sys.path.insert(0, str(REPO / 'evaluation/scripts'))
from rescore_kitti_rte import primary_result

METHODS = ('VLOM',)
FIELDS = ('rpe_trans_pct', 'ate_m', 'frames', 'kitti_rte_trans_pct',
          'kitti_rte_rot_deg_per_100m')


def main():
    assert not (Path(__file__).parent / 'summary.json').exists(), 'Use a fresh evidence directory.'
    verify_binaries()
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()

    def committed(path):
        raw = subprocess.check_output(['git', 'show', revision + ':' + path.relative_to(REPO).as_posix()], cwd=REPO)
        assert raw == path.read_bytes(), path
        return json.loads(raw)

    primary = committed(E / 'sequences/00/audit/report.json')
    pins = {name: committed(E / (name + '.json')) for name in ('baseline', 'candidate')}
    files = set()
    checks = []
    for method in METHODS:
        row = next(row for row in primary['rows'] if row['method'] == method)
        assert row['audit_status'] == 'drift' and row['process_exit_code'] == 0
        candidate_path = Path(row['summary_path'])
        frozen_path = E / 'sequences/00/frozen_baseline/replays' / candidate_path.name
        receipt_path = frozen_path.with_name(frozen_path.stem + '_native_command.json')
        receipt = json.loads(receipt_path.read_text())
        assert receipt['exit_code'] == 0 and receipt['finished_unix'] >= receipt['started_unix']
        assert receipt['binary_sha256'] == pins['baseline']['sha256']
        assert receipt['command'][0] == pins['baseline']['binary']
        assert receipt['environment'] == {'OMP_NUM_THREADS': '1', 'OPENBLAS_NUM_THREADS': '1'}
        new_receipt = committed(candidate_path.with_name(candidate_path.stem + '_native_command.json'))
        assert new_receipt['exit_code'] == 0 and new_receipt['binary_sha256'] == pins['candidate']['sha256']
        expected_command = list(new_receipt['command'])
        expected_command[0] = pins['baseline']['binary']
        expected_command[expected_command.index('--summary-json') + 1] = str(frozen_path)
        assert receipt['command'] == expected_command, method
        old_summary = json.loads(frozen_path.read_text())
        new_summary = committed(candidate_path)
        assert old_summary['gt_sha256'] == new_summary['gt_sha256']
        assert old_summary['pcd_dir'] == new_summary['pcd_dir']
        old = primary_result(old_summary, method)
        new = primary_result(new_summary, method)
        assert old['status'] == new['status'] == 'ok'
        deltas = {key: abs(old[key] - new[key]) for key in FIELDS}
        assert all(value == 0 for value in deltas.values()), method
        before, after = native_output(frozen_path), native_output(candidate_path)
        poses = sorted(p.name for p in before.glob('*.txt') if not p.name.endswith('_evaluated_gt.txt'))
        assert poses
        equal = {name: (before / name).read_bytes() == (after / name).read_bytes() for name in poses}
        assert all(equal.values()), method
        closed = [frozen_path, receipt_path, frozen_path.with_suffix('.log')]
        closed += [p for p in frozen_path.with_name(frozen_path.stem + '_native').rglob('*') if p.is_file()]
        files.update(closed)
        checks.append(dict(method=method, sequence='00', native_process_exit_code=receipt['exit_code'],
            historical_audit_status=row['audit_status'], stored_rpe_trans_pct=row['stored_rpe_trans_pct'],
            stored_ate_m=row['stored_ate_m'], replayed_metrics={key: old[key] for key in FIELDS},
            metric_abs_delta=deltas, saved_pose_bytes_equal=equal,
            baseline_summary_head_metadata=old_summary.get('evaluator_git_commit')))
    write(Path(__file__).parent / 'summary.json', dict(command=[sys.executable, *sys.argv],
        cwd=str(REPO), evidence_revision=revision, implementation_commit=SOURCE,
        baseline_source_commit=pins['baseline']['source_commit'], checks=checks,
        evidence_files={p.relative_to(REPO).as_posix(): digest(p) for p in sorted(files)},
        pending_frozen_methods=['Mesh-LOAM'], sequence_checkpoint_complete=False,
        owned_input_cleanup_complete=False, parent_frozen_audit_exit_available=False, exit_code=0,
        provenance_note='Summary evaluator_git_commit records working-tree HEAD at write time; frozen source and executable SHA256 pins establish native provenance.',
        scope='Closed VLOM frozen-control subset only. Historical primary audit retains exit 1. No complete00, whole-zoo or fresh NCLT claim.'))
    print(json.dumps(dict(verified_closed_methods=list(METHODS), exit_code=0)))


if __name__ == '__main__':
    main()
