import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from evaluation.scripts import analyze_inten_conditioning as analysis


REPO = Path(__file__).resolve().parents[1]
FIXTURE = REPO / 'experiments/results/inten_trace_export/evidence/fixture/paper_default/candidate_on'
SKIPPED = REPO / 'experiments/results/inten_trace_export/evidence/skipped_input/candidate_on'


def iteration(H, pose=None, g=None):
    return dict(H=np.asarray(H).ravel().tolist(), g=[0.] * 6 if g is None else list(g),
                linearization_pose=np.eye(4).ravel().tolist() if pose is None else pose.ravel().tolist(),
                delta=[0.] * 6, pairs=100, class_pairs=[25] * 4,
                weighted_squared_residual=10., step_attempted=True,
                step_finite=True, update_applied=True)


def plane_system(pose):
    # Independent scalar point/plane Jacobians, constructed from world geometry.
    points = np.array([[x, y, 2.] for x in (-2., 0., 2.) for y in (-1., 0., 1.)])
    normal = np.array([0., 0., 1.])
    R, t = pose[:3, :3], pose[:3, 3]
    n = R @ normal
    world = points @ R.T + t
    J = np.array([np.r_[n, np.cross(p, n)] for p in world])
    body_J = np.array([np.r_[normal, np.cross(p, normal)] for p in points])
    residuals = np.arange(len(points)) / 10
    return J.T @ J, J.T @ residuals, body_J.T @ body_J, body_J.T @ residuals


class InTenConditioningTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.output = self.root / 'out'

    def test_sensor_centered_plane_system_and_gradient_are_independent_of_world_origin(self):
        for angle, t in [(0., [0., 0., 0.]), (.7, [11., -7., 3.]), (-1.2, [-31., 50., -8.])]:
            c, s = np.cos(angle), np.sin(angle)
            pose = np.eye(4)
            pose[:3, :3] = [[c, 0, s], [0, 1, 0], [-s, 0, c]]
            pose[:3, 3] = t
            H, g, expected_H, expected_g = plane_system(pose)
            row = analysis.analyze_iteration(iteration(H, pose, g), 1., 1e-9)
            with self.subTest(angle=angle, t=t):
                self.assertEqual(row['status'], 'ok')
                np.testing.assert_allclose(np.array(row['scaled_H']).reshape(6, 6), expected_H, atol=1e-10)
                np.testing.assert_allclose(row['scaled_gradient'], expected_g, atol=1e-11)
                self.assertEqual(row['numerical_rank'], 3)
                self.assertIsNone(row['condition_number'])
                np.testing.assert_allclose(row['eigenvalues'], [0, 0, 0, 6, 9, 24], atol=1e-10)

    def test_explicit_length_changes_rotational_curvature_and_preserves_quadratic_form(self):
        H = np.diag([1., 2., 3., 16., 20., 24.])
        row = analysis.analyze_iteration(iteration(H), 2., 1e-9)
        np.testing.assert_allclose(row['eigenvalues'], [1, 2, 3, 4, 5, 6])
        self.assertEqual(row['condition_number'], 6)
        self.assertEqual(row['numerical_rank'], 6)
        q = np.arange(1., 7.)
        native = q / [1, 1, 1, 2, 2, 2]
        scaled_H = np.array(row['scaled_H']).reshape(6, 6)
        self.assertAlmostEqual(q @ scaled_H @ q, native @ H @ native)
        other = analysis.analyze_iteration(iteration(H), 1., 1e-9)
        self.assertNotEqual(other['condition_number'], row['condition_number'])

    def test_rank_cutoff_zero_and_indefinite_systems_do_not_get_finite_condition(self):
        for diagonal, status, rank in [([0.] * 6, 'zero_system', 0),
                                        ([1, 1, 1, 1, 1, 1e-12], 'ok', 5),
                                        ([1, 1, 1, 1, 1, -1e-12], 'ok', 5),
                                        ([1, 1, 1, 1, 1, -1], 'indefinite_H', None)]:
            row = analysis.analyze_iteration(iteration(np.diag(diagonal)), 1., 1e-9)
            with self.subTest(diagonal=diagonal):
                self.assertEqual(row['status'], status)
                self.assertEqual(row['numerical_rank'], rank)
                self.assertIsNone(row['condition_number'])
                self.assertEqual(row['eigenvalues'][0], min(diagonal))

    def test_nonfinite_failures_are_retained_without_zero_substitution(self):
        it = iteration(np.eye(6)); it['H'][0] = '+Infinity'; it['delta'][0] = 'NaN'
        it['step_finite'] = it['update_applied'] = False
        row = analysis.analyze_iteration(it, 1., 1e-9)
        self.assertEqual(row['status'], 'nonfinite_H')
        self.assertEqual(row['native_H'][0], '+Infinity')
        self.assertEqual(row['native_delta'][0], 'NaN')
        self.assertIsNone(row['eigenvalues'])
        it = iteration(np.eye(6)); it['g'][0] = '-Infinity'
        row = analysis.analyze_iteration(it, 1., 1e-9)
        self.assertEqual(row['status'], 'ok')
        self.assertEqual(row['native_gradient'][0], '-Infinity')
        self.assertIsNone(row['scaled_gradient'])
        self.assertEqual(row['numerical_rank'], 6)

    def test_asymmetric_nonrigid_and_overflow_systems_are_unavailable(self):
        H = np.eye(6); H[0, 1] = 1
        row = analysis.analyze_iteration(iteration(H), 1., 1e-9)
        self.assertEqual(row['status'], 'asymmetric_H')
        pose = np.eye(4); pose[0, 0] = 2
        row = analysis.analyze_iteration(iteration(np.eye(6), pose), 1., 1e-9)
        self.assertEqual(row['status'], 'invalid_linearization_pose')
        pose = np.eye(4); pose[0, 3] = 1e300
        row = analysis.analyze_iteration(iteration(np.eye(6), pose), 1., 1e-9)
        self.assertEqual(row['status'], 'numerical_failure')
        self.assertIsNone(row['condition_number'])

    def analyze(self, case=FIXTURE, **kwargs):
        return analysis.analyze_trace(case / 'trace.jsonl', case / 'summary.json',
                                      case / 'command.json', self.output, 1., **kwargs)

    def test_all_iterations_and_skips_preserve_original_indices_and_native_observations(self):
        report = self.analyze(SKIPPED)
        rows = [json.loads(line) for line in (self.output / 'iterations.jsonl').read_text().splitlines()]
        native = [json.loads(line) for line in (SKIPPED / 'trace.jsonl').read_text().splitlines()]
        expected = {(f['input_index'], f['frame_index'], stage, it['index']): it
                    for f in native if f['type'] == 'frame'
                    for stage in ('s2s', 's2m') for it in f[stage]['iterations']}
        self.assertEqual(len(rows), len(expected))
        self.assertTrue(all(row['input_index'] != row['frame_index'] for row in rows))
        for row in rows:
            it = expected[(row['input_index'], row['frame_index'], row['stage'], row['iteration_index'])]
            self.assertEqual(row['native_H'], it['H'])
            self.assertEqual(row['native_delta'], it['delta'])
            self.assertEqual(row['class_pairs'], it['class_pairs'])
        self.assertEqual(sum(s['iterations'] for s in report['stages'].values()), len(rows))
        self.assertEqual(report['validation']['skipped_frames'], 1)
        self.assertEqual(report['iterations_sha256'], analysis.sha256(self.output / 'iterations.jsonl'))

    def test_invalid_run_incomplete_trace_and_mutated_second_pass_publish_nothing(self):
        case = self.root / 'case'; case.mkdir()
        for name in ('trace.jsonl', 'summary.json', 'command.json'):
            (case / name).write_bytes((FIXTURE / name).read_bytes())
        receipt = json.loads((case / 'command.json').read_text()); receipt['exit_code'] = 1
        (case / 'command.json').write_text(json.dumps(receipt))
        with self.assertRaisesRegex(analysis.TraceError, 'not exit 0'):
            self.analyze(case)
        self.assertFalse(self.output.exists())
        (case / 'command.json').write_bytes((FIXTURE / 'command.json').read_bytes())
        (case / 'trace.jsonl').write_text('\n'.join((case / 'trace.jsonl').read_text().splitlines()[:-1]) + '\n')
        with self.assertRaisesRegex(analysis.TraceError, 'incomplete'):
            self.analyze(case)
        self.assertFalse(self.output.exists())
        (case / 'trace.jsonl').write_bytes((FIXTURE / 'trace.jsonl').read_bytes())
        original = analysis.validate_trace
        def mutate(*args):
            result = original(*args)
            (case / 'summary.json').write_text((case / 'summary.json').read_text() + '\n')
            return result
        with patch.object(analysis, 'validate_trace', side_effect=mutate):
            with self.assertRaisesRegex(analysis.TraceError, 'changed during analysis'):
                self.analyze(case)
        self.assertFalse(self.output.exists())

    def test_cli_requires_scale_and_never_overwrites_an_existing_directory(self):
        cmd = [sys.executable, str(REPO / 'evaluation/scripts/analyze_inten_conditioning.py'),
               '--trace', str(FIXTURE / 'trace.jsonl'), '--summary', str(FIXTURE / 'summary.json'),
               '--receipt', str(FIXTURE / 'command.json'), '--output-dir', str(self.output)]
        self.assertEqual(subprocess.run(cmd, capture_output=True).returncode, 2)
        self.assertFalse(self.output.exists())
        cmd += ['--rotation-length-m', '1']
        self.assertEqual(subprocess.run(cmd, capture_output=True).returncode, 0)
        before = {p.name: p.read_bytes() for p in self.output.iterdir()}
        self.assertEqual(subprocess.run(cmd, capture_output=True).returncode, 1)
        self.assertEqual({p.name: p.read_bytes() for p in self.output.iterdir()}, before)

    def test_invalid_scales_and_tolerances_are_rejected(self):
        for value in (0, -1, float('nan'), float('inf')):
            with self.subTest(scale=value), self.assertRaises(analysis.TraceError):
                analysis.analyze_trace(FIXTURE / 'trace.jsonl', FIXTURE / 'summary.json',
                                       FIXTURE / 'command.json', self.output, value)
        for value in (0, -1, 1, float('nan'), float('inf')):
            with self.subTest(rank_rtol=value), self.assertRaises(analysis.TraceError):
                self.analyze(rank_rtol=value)
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
