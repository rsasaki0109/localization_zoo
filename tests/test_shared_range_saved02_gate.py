"""Exercise publication gates with a committed synthetic saved-path study.

No estimator, scanner data or running study output is used by these fixtures.
"""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import types
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
STUDY = ROOT / 'experiments/results/shared_near_range'


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value) + '\n')


def native_output(summary):
    return summary.parent / (summary.stem + '_native') / 'dogfooding_results'


class Saved02PublicationGateTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.repo = Path(self.temporary.name)
        self.study = self.repo / 'experiments/results/shared_near_range'
        self.script = self.study / 'diagnose_saved02.py'
        self.script.parent.mkdir(parents=True)
        self.script.write_bytes((STUDY / self.script.name).read_bytes())
        (self.study / 'support.py').write_text('# This fixture installs an isolated support namespace.\n')
        wrapper = self.study / 'native_wrapper.py'
        wrapper.write_text('# Synthetic output-isolation wrapper, distinct from both native executables.\n')
        self.wrapper_sha256 = hashlib.sha256(wrapper.read_bytes()).hexdigest()
        write_json(self.study / 'protocol.json', {'implementation_commit': 'synthetic-frozen-source'})
        protocol = json.loads((STUDY / 'diagnostics/02_protocol.json').read_text())
        write_json(self.study / 'diagnostics/02_protocol.json', protocol)
        for name in ('diagnose_kitti_steps.py', 'evaluate_external_kitti_odometry.py'):
            target = self.repo / 'evaluation/scripts' / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((ROOT / 'evaluation/scripts' / name).read_bytes())
        support = types.SimpleNamespace(E=self.study, REPO=self.repo,
                                        R=self.repo / 'runtime', SOURCE='synthetic-frozen-source',
                                        native_output=native_output, write=write_json)
        specification = importlib.util.spec_from_file_location('_saved02_fixture', self.script)
        self.module = importlib.util.module_from_spec(specification)
        previous_support = sys.modules.get('support')
        sys.modules['support'] = support
        try:
            specification.loader.exec_module(self.module)
        finally:
            if previous_support is None:
                sys.modules.pop('support', None)
            else:
                sys.modules['support'] = previous_support
        self.addCleanup(lambda: sys.path.remove(str(self.repo / 'evaluation/scripts')))
        self.paths = {}
        rows, frozen_rows, checks = [], [], []
        # A straight, moving path long enough for official KITTI segments.
        poses = ''.join('1 0 0 %d 0 1 0 0 0 0 1 0\n' % (frame * 5) for frame in range(201))
        metrics = {key: 0.0 for key in ('ate_m', 'rpe_trans_pct', 'kitti_rte_trans_pct',
                                       'kitti_rte_rot_deg_per_100m')}
        for method, (stem, pose_name) in self.module.METHODS.items():
            summaries = []
            for mode, digest in (('audit', 'candidate'), ('frozen_baseline', 'baseline')):
                summary = self.study / 'sequences/02' / mode / 'replays' / (stem + '.json')
                write_json(summary, {'methods': [dict(name=method, **metrics)]})
                write_json(summary.with_name(stem + '_native_command.json'),
                           dict(exit_code=0, binary_sha256=digest, finished_unix=1,
                                environment={'OMP_NUM_THREADS': '1', 'OPENBLAS_NUM_THREADS': '1'}))
                output = native_output(summary)
                output.mkdir(parents=True)
                for name in (pose_name, 'gt.txt', 'kitti_seq_02_full_evaluated_gt.txt'):
                    (output / name).write_text(poses)
                summaries.append(summary)
            self.paths[method] = summaries[0]
            aggregate = Path('experiments/results') / (stem + '_matrix.json')
            write_json(self.repo / aggregate,
                       {'variants': [{'id': 'fixed', 'args': [],
                                      'summary_path': 'missing_history/' + stem + '/summary.json'}]})
            rows.append(dict(method=method, audit_status='drift', process_exit_code=0,
                             summary_path=str(summaries[0]), frames=201,
                             aggregate_path=str(aggregate), variant_id='fixed', args=[]))
            frozen_rows.append(dict(method=method, process_exit_code=0, summary_path=str(summaries[1])))
            checks.append(dict(method=method, sequence='02', metric_abs_delta=metrics,
                               saved_pose_bytes_equal={pose_name: True, 'gt.txt': True}))
        for name in ('candidate', 'baseline'):
            write_json(self.study / (name + '.json'), {'sha256': name, 'source_commit': 'synthetic-frozen-source'})
        for mode, digest, report_rows in (('audit', 'candidate', rows),
                                          ('frozen_baseline', 'baseline', frozen_rows)):
            write_json(self.study / 'sequences/02' / mode / 'report.json',
                       dict(exit_code=1, binary_sha256=self.wrapper_sha256, rows=report_rows))
            write_json(self.study / 'sequences/02' / mode / 'audit_command.json', {'exit_code': 1})
        write_json(self.study / 'sequences/02/frozen_comparison.json', dict(exit_code=0, checks=checks))
        write_json(self.study / 'sequences/02/cleanup.json',
                   dict(sequence='02', hashes_verified_before_removal=True,
                        input_dir_exists_after=False, owned_input_dir=str(self.repo / 'removed_scans')))
        self.git('init', '-q')
        self.commit()
        self.output = self.repo / 'diagnostic_output'

    def git(self, *arguments):
        return subprocess.run(['git', '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.invalid',
                               *arguments], cwd=self.repo, check=True, capture_output=True, text=True)

    def commit(self):
        self.git('add', '.')
        self.git('commit', '-qm', 'Synthetic committed saved-path fixture')

    def invoke(self, *extra):
        with patch.object(sys, 'argv', [str(self.script), '--output-dir', str(self.output), *extra]), \
                contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            self.module.main()

    def prepare_relocated03(self, sequence='03'):
        """Relocate a closed cohort;04 retains its additional native controls."""
        source = self.study / 'sequences/02'
        target = self.study / 'sequences' / sequence
        shutil.copytree(source, target)
        if sequence == '07':
            # Add a distinct VLOM receipt/path so omitting its control cannot
            # accidentally pass by reusing A-LOAM's complete cohort.
            for mode in ('audit', 'frozen_baseline'):
                path = target / mode / 'report.json'
                report = json.loads(path.read_text())
                row = dict(next(row for row in report['rows'] if row['method'] == 'A-LOAM'))
                original = target / mode / 'replays/aloam.json'
                summary = original.with_name('vlom.json')
                contents = json.loads(original.read_text())
                contents['methods'][0]['name'] = 'VLOM'
                write_json(summary, contents)
                shutil.copyfile(original.with_name('aloam_native_command.json'),
                                summary.with_name('vlom_native_command.json'))
                shutil.copytree(native_output(original), native_output(summary))
                (native_output(summary) / 'A_LOAM.txt').rename(native_output(summary) / 'VLOM.txt')
                row.update(method='VLOM', summary_path=str(
                    source / mode / 'replays/vlom.json'))
                if mode == 'audit':
                    row['aggregate_path'] = 'experiments/results/vlom_matrix.json'
                    write_json(self.repo / row['aggregate_path'], {'variants': [
                        dict(id='fixed', args=[], summary_path='missing_history/vlom/summary.json')]})
                report['rows'].append(row)
                write_json(path, report)
            path = target / 'frozen_comparison.json'
            comparison = json.loads(path.read_text())
            row = dict(comparison['checks'][0], method='VLOM',
                       saved_pose_bytes_equal={'VLOM.txt': True, 'gt.txt': True})
            comparison['checks'].append(row)
            write_json(path, comparison)
        protocol = json.loads((STUDY / f'diagnostics/{sequence}_protocol.json').read_text())
        write_json(self.study / f'diagnostics/{sequence}_protocol.json', protocol)
        cohort = (set(self.module.METHODS07) if sequence == '07' else
                  set(self.module.METHODS) if sequence == '05' else
                  {'A-LOAM', 'F-LOAM', 'LeGO-LOAM', 'Mesh-LOAM'} if sequence == '08' else
                  {'CT-ICP', 'Mesh-LOAM'} if sequence in ('04', '06') else {'Mesh-LOAM'})
        recorded_root = Path('/archived/saved-study-checkout')
        for mode in ('audit', 'frozen_baseline'):
            path = target / mode / 'report.json'
            report = json.loads(path.read_text())
            report['rows'] = [row for row in report['rows'] if row['method'] in cohort]
            for row in report['rows']:
                relative = Path(row['summary_path']).relative_to(self.repo)
                relative = Path(str(relative).replace('sequences/02/', f'sequences/{sequence}/'))
                row.update(sequence=sequence, cwd=str(recorded_root),
                           summary_path=str(recorded_root / relative))
            if sequence == '04' and mode == 'frozen_baseline':
                for method in ('DiLO', 'ELO'):
                    summary = target / mode / 'replays' / (method.lower() + '.json')
                    write_json(summary, {'methods': [{'name': method}]})
                    write_json(summary.with_name(summary.stem + '_native_command.json'),
                               dict(exit_code=0, binary_sha256='baseline', finished_unix=1))
                    report['rows'].append(dict(method=method, sequence=sequence,
                                               process_exit_code=0, cwd=str(recorded_root),
                                               summary_path=str(recorded_root / summary.relative_to(self.repo))))
            write_json(path, report)
        for path in target.glob('*/replays/*_native/dogfooding_results/kitti_seq_02_full_evaluated_gt.txt'):
            path.rename(path.with_name(f'kitti_seq_{sequence}_full_evaluated_gt.txt'))
        path = target / 'frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'] = [row for row in comparison['checks'] if row['method'] in cohort]
        for row in comparison['checks']:
            row['sequence'] = sequence
        if sequence == '04':
            for method in ('DiLO', 'ELO'):
                comparison['checks'].append(dict(method=method, sequence=sequence,
                                                metric_abs_delta={'ate_m': 0},
                                                saved_pose_bytes_equal={method + '.txt': True}))
        write_json(path, comparison)
        path = target / 'cleanup.json'
        cleanup = json.loads(path.read_text())
        cleanup['sequence'] = sequence
        write_json(path, cleanup)
        self.commit()

    def test_complete_committed_paths_produce_zero_error_and_keep_all_methods(self):
        self.invoke()
        result = json.loads((self.output / 'summary.json').read_text())
        self.assertEqual(result['exit_code'], 0)
        self.assertEqual({row['method'] for row in result['rows']}, set(self.module.METHODS))
        self.assertTrue(all(result['completion_gate'].values()))
        for row in result['rows']:
            self.assertTrue(all(value == 0 for value in row['current_saved_pose_metric_abs_delta'].values()))
            self.assertEqual(row['largest_current_error_growth_block']['position_error_end_m'], 0)

    def test_uncommitted_cleanup_is_rejected_without_creating_output(self):
        relative = 'experiments/results/shared_near_range/sequences/02/cleanup.json'
        self.git('rm', '--cached', relative)
        self.git('commit', '-qm', 'Leave cleanup uncommitted')
        with self.assertRaises(SystemExit) as failure:
            self.invoke()
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_available_historical_path_has_known_gap_without_alignment(self):
        method = 'A-LOAM'
        current = native_output(self.paths[method])
        historical = self.repo / 'missing_history/aloam'
        historical.mkdir(parents=True)
        (historical / 'evaluated_gt_kitti.txt').write_bytes((current / 'gt.txt').read_bytes())
        path = historical / 'trajectory_kitti.txt'
        path.write_text(''.join('1 0 0 %.1f 0 1 0 0 0 0 1 0\n' % (frame * 4.5)
                               for frame in range(201)))
        metrics = self.module.diagnose(self.module.load_kitti_poses(path),
                                      self.module.load_kitti_poses(current / 'gt.txt'))
        expected = metrics['official_metrics_from_saved_poses']
        write_json(historical / 'summary.json', {'methods': [dict(name=method, **expected)]})
        report_path = self.study / 'sequences/02/audit/report.json'
        report = json.loads(report_path.read_text())
        row = next(row for row in report['rows'] if row['method'] == method)
        row.update(stored_rpe_trans_pct=expected['rpe_trans_pct'], stored_ate_m=expected['ate_m'])
        write_json(report_path, report)
        self.commit()
        self.invoke()
        result = json.loads((self.output / 'summary.json').read_text())
        paired = next(row for row in result['rows'] if row['method'] == method)
        self.assertTrue(paired['paired_historical_comparison_available'])
        self.assertTrue(paired['same_evaluated_gt_bytes'])
        self.assertEqual(paired['final_path_gap_m'], 100)
        self.assertEqual(paired['max_path_gap_frame'], 200)
        self.assertTrue((self.output / 'aloam/paired_trace.png').is_file())

    def test_modified_last_method_pose_is_rejected_before_any_output(self):
        path = native_output(self.paths['Mesh-LOAM']) / 'Mesh_LOAM.txt'
        path.write_text(path.read_text().replace('1 0 0 5 ', '1 0 0 6 ', 1))
        with self.assertRaises(SystemExit) as failure:
            self.invoke()
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_failed_frozen_comparison_is_rejected_without_creating_output(self):
        path = self.study / 'sequences/02/frozen_comparison.json'
        report = json.loads(path.read_text())
        report['exit_code'] = 2
        write_json(path, report)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke()
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_native_hash_cannot_substitute_for_audit_wrapper_hash(self):
        path = self.study / 'sequences/02/audit/report.json'
        report = json.loads(path.read_text())
        report['binary_sha256'] = 'candidate'
        write_json(path, report)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke()
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_wrapper_hash_cannot_substitute_for_native_receipt_hash(self):
        summary = self.paths['Mesh-LOAM']
        path = summary.with_name(summary.stem + '_native_command.json')
        receipt = json.loads(path.read_text())
        receipt['binary_sha256'] = self.wrapper_sha256
        write_json(path, receipt)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke()
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_closed03_uses_committed_artifacts_in_relocated_checkout(self):
        self.prepare_relocated03()
        self.invoke('--sequence', '03')
        result = json.loads((self.output / 'summary.json').read_text())
        self.assertEqual(result['sequence'], '03')
        self.assertEqual([row['method'] for row in result['rows']], ['Mesh-LOAM'])
        self.assertEqual(result['rows'][0]['largest_current_error_growth_block']['position_error_end_m'], 0)
        self.assertTrue(all(result['completion_gate'].values()))

    def test03_rejects_a_protocol_for_another_sequence_before_output(self):
        self.prepare_relocated03()
        path = self.study / 'diagnostics/03_protocol.json'
        protocol = json.loads(path.read_text())
        protocol['sequence'] = '02'
        write_json(path, protocol)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '03')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test03_rejects_frozen_comparison_from_another_sequence(self):
        self.prepare_relocated03()
        path = self.study / 'sequences/03/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'][0]['sequence'] = '02'
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '03')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test03_rejects_summary_outside_recorded_checkout(self):
        self.prepare_relocated03()
        path = self.study / 'sequences/03/audit/report.json'
        report = json.loads(path.read_text())
        report['rows'][0]['summary_path'] = '/unrelated/mesh_loam.json'
        write_json(path, report)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '03')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_unclosed09_is_not_an_allowed_diagnostic_sequence(self):
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '09')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_closed08_keeps_all_four_differences_without_inheriting_ct(self):
        self.prepare_relocated03('08')
        self.invoke('--sequence', '08')
        result = json.loads((self.output / 'summary.json').read_text())
        expected = {'A-LOAM', 'F-LOAM', 'LeGO-LOAM', 'Mesh-LOAM'}
        self.assertEqual({row['method'] for row in result['rows']}, expected)
        self.assertEqual(set(result['frozen_control_methods']), expected)
        self.assertTrue(all(result['completion_gate'].values()))

    def test08_rejects_an_unexpected_ct_control_before_output(self):
        self.prepare_relocated03('08')
        path = self.study / 'sequences/08/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'].append(dict(method='CT-ICP', sequence='08',
                                         metric_abs_delta={'ate_m': 0},
                                         saved_pose_bytes_equal={'CT_ICP.txt': True}))
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '08')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_closed07_retains_vlom_alongside_all_five_previous_methods(self):
        self.prepare_relocated03('07')
        self.invoke('--sequence', '07')
        result = json.loads((self.output / 'summary.json').read_text())
        expected = {'A-LOAM', 'CT-ICP', 'F-LOAM', 'LeGO-LOAM', 'Mesh-LOAM', 'VLOM'}
        self.assertEqual({row['method'] for row in result['rows']}, expected)
        self.assertEqual(set(result['frozen_control_methods']), expected)
        self.assertTrue(all(result['completion_gate'].values()))
        self.assertTrue((self.output / 'vlom/current_steps.json').is_file())

    def test07_rejects_missing_vlom_control_before_output(self):
        self.prepare_relocated03('07')
        path = self.study / 'sequences/07/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'] = [row for row in comparison['checks'] if row['method'] != 'VLOM']
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '07')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_closed05_retains_all_historical_differences_in_relocated_checkout(self):
        self.prepare_relocated03('05')
        self.invoke('--sequence', '05')
        result = json.loads((self.output / 'summary.json').read_text())
        self.assertEqual(result['sequence'], '05')
        self.assertEqual({row['method'] for row in result['rows']}, set(self.module.METHODS))
        self.assertEqual(set(result['frozen_control_methods']), set(self.module.METHODS))
        self.assertTrue(all(result['completion_gate'].values()))

    def test05_rejects_a_missing_frozen_control_before_output(self):
        self.prepare_relocated03('05')
        path = self.study / 'sequences/05/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'] = [row for row in comparison['checks'] if row['method'] != 'F-LOAM']
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '05')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test05_rejects_a_retained_owned_alias_even_when_dangling(self):
        self.prepare_relocated03('05')
        alias = self.repo / 'runtime/data/dogfooding_results/kitti_seq_05_full'
        alias.parent.mkdir(parents=True)
        alias.symlink_to(self.repo / 'removed_scans')
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '05')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test_closed04_retains_extra_controls_and_diagnoses_only_historical_differences(self):
        self.prepare_relocated03('04')
        self.invoke('--sequence', '04')
        result = json.loads((self.output / 'summary.json').read_text())
        self.assertEqual(result['sequence'], '04')
        self.assertEqual({row['method'] for row in result['rows']}, {'CT-ICP', 'Mesh-LOAM'})
        self.assertEqual(set(result['frozen_control_methods']), {'CT-ICP', 'DiLO', 'ELO', 'Mesh-LOAM'})
        self.assertTrue(all(result['completion_gate'].values()))

    def test_closed06_keeps_its_complete_cohort_without_inheriting_extra04_controls(self):
        self.prepare_relocated03('06')
        self.invoke('--sequence', '06')
        result = json.loads((self.output / 'summary.json').read_text())
        self.assertEqual(result['sequence'], '06')
        self.assertEqual({row['method'] for row in result['rows']}, {'CT-ICP', 'Mesh-LOAM'})
        self.assertEqual(set(result['frozen_control_methods']), {'CT-ICP', 'Mesh-LOAM'})
        self.assertTrue(all(result['completion_gate'].values()))

    def test06_rejects_an_unexpected_control_before_output(self):
        self.prepare_relocated03('06')
        path = self.study / 'sequences/06/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'].append(dict(method='ELO', sequence='06',
                                         metric_abs_delta={'ate_m': 0},
                                         saved_pose_bytes_equal={'ELO.txt': True}))
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '06')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test04_rejects_a_bad_receipt_on_an_additional_control_before_output(self):
        self.prepare_relocated03('04')
        path = self.study / 'sequences/04/frozen_baseline/replays/dilo_native_command.json'
        receipt = json.loads(path.read_text())
        receipt['binary_sha256'] = self.wrapper_sha256
        write_json(path, receipt)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '04')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test04_rejects_a_difference_on_an_additional_control_before_output(self):
        self.prepare_relocated03('04')
        path = self.study / 'sequences/04/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        row = next(row for row in comparison['checks'] if row['method'] == 'ELO')
        row['metric_abs_delta']['ate_m'] = 1
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '04')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())

    def test04_rejects_a_missing_additional_control_before_output(self):
        self.prepare_relocated03('04')
        path = self.study / 'sequences/04/frozen_comparison.json'
        comparison = json.loads(path.read_text())
        comparison['checks'] = [row for row in comparison['checks'] if row['method'] != 'DiLO']
        write_json(path, comparison)
        self.commit()
        with self.assertRaises(SystemExit) as failure:
            self.invoke('--sequence', '04')
        self.assertEqual(failure.exception.code, 2)
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
