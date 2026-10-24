import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import numpy as np

SCRIPTS = Path(__file__).resolve().parents[1] / 'evaluation/scripts'
sys.path.insert(0, str(SCRIPTS))
SPEC = importlib.util.spec_from_file_location('official_matcher', SCRIPTS/'run_official_kiss_matcher.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class OfficialMatcherTests(unittest.TestCase):
    def test_default_kitti_loader_preserves_xyz_and_pcd_fields_are_not_xyz_only(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            values = np.array([[1, 2, 3, .4], [-2, 0, 4, .8]], dtype='<f4')
            binary = root/'scan.bin'
            values.tofile(binary)
            np.testing.assert_array_equal(MODULE.load_input_cloud(binary), values[:, :3])
            pcd = root/'scan.pcd'
            # Intensity-first field order detects accidental PointXYZI reinterpretation.
            header = 'VERSION .7\nFIELDS intensity z y x\nSIZE 4 4 4 4\nTYPE F F F F\nCOUNT 1 1 1 1\nWIDTH 2\nHEIGHT 1\nPOINTS 2\nDATA binary\n'
            pcd.write_bytes(header.encode()+values[:, [3, 2, 1, 0]].tobytes())
            np.testing.assert_array_equal(MODULE.load_input_cloud(pcd, 'pcd'), values[:, :3])
            values[0, 0] = np.nan
            pcd.write_bytes(header.encode()+values[:, [3, 2, 1, 0]].tobytes())
            with self.assertRaisesRegex(ValueError, 'nonfinite'):
                MODULE.load_input_cloud(pcd, 'pcd')

    def test_augmented_source_maps_to_target_with_noncommuting_poses(self):
        from evaluate_external_kitti_odometry import rpy_matrix
        source_pose, target_pose = np.eye(4), np.eye(4)
        source_pose[:3, :3] = rpy_matrix(.2, -.1, .7)
        target_pose[:3, :3] = rpy_matrix(-.3, .4, -.2)
        source_pose[:3, 3], target_pose[:3, 3] = [4, 5, 2], [-1, 3, 6]
        point = np.array([2., -1., 3., 1.])
        augmented = MODULE.augmentation(90) @ point
        expected = MODULE.expected_transform([source_pose, target_pose], 0, 1, 90)
        np.testing.assert_allclose(expected @ augmented,
                                   np.linalg.solve(target_pose, source_pose @ point), atol=1e-12)
        rotation, translation = MODULE.errors(expected, expected)
        self.assertLess(rotation, 1e-5)
        self.assertLess(translation, 1e-12)

    def test_worker_command_cannot_receive_reference_or_gt_initializer(self):
        command = MODULE.child_command('/python', Path('/jobs/clouds.json'), Path('/out/pose.json'))
        self.assertEqual(command[2:], ['--worker-jobs', '/jobs/clouds.json',
                                       '--worker-output', '/out/pose.json'])
        self.assertNotIn('--reference-csv', command)

    def test_explicit_body_reference_conversion_is_scoring_only_and_requires_pin(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            reference, calibration, manifest = root/'body.csv', root/'extrinsic.json', root/'protocol.json'
            reference.write_text('lidar_pose.x,lidar_pose.y,lidar_pose.z,lidar_pose.roll,lidar_pose.pitch,lidar_pose.yaw\n0,0,0,0,0,0\n1,0,0,0,0,1.5707963267948966\n')
            calibration.write_text(json.dumps({'T_body_sensor': np.diag([1., -1., -1., 1.]).tolist()}))
            protocol = {'reference_csv_sha256': MODULE.sha(reference),
                'reference_sensor_to_body_sha256': MODULE.sha(calibration),
                'variants': [{'id': 'full', 'use_quatro': False, 'config': {}}],
                'cases': [{'id': 'pair', 'kind': 'pcd', 'source_frame': 0, 'target_frame': 1, 'source_yaw_deg': 0}],
                'input_format': 'pcd', 'input_keys': {'0': 'a.pcd', '1': 'b.pcd'},
                'inputs': {n: {'name': n, 'sha256': 'mock', 'bytes': 1} for n in ['a.pcd', 'b.pcd']},
                'package_version': '1.0.2', 'voxel_size_m': .3, 'timeout_seconds': 1,
                'success_rotation_deg': 2, 'success_translation_m': .5, 'claim_limit': 'test'}
            manifest.write_text(json.dumps(protocol))
            argv = ['runner', '--manifest', str(manifest), '--raw-root', str(root),
                    '--reference-csv', str(reference), '--output-dir', str(root/'out')]
            with mock.patch.object(sys, 'argv', argv), self.assertRaisesRegex(ValueError, 'calibration'):
                MODULE.main()

            def worker(command, **kwargs):
                job_path = Path(command[command.index('--worker-jobs')+1])
                job = job_path.read_text()
                self.assertNotIn(str(reference), job)
                self.assertNotIn(str(calibration), job)
                self.assertNotIn('--reference-sensor-to-body-json', command)
                output = Path(command[command.index('--worker-output')+1])
                output.write_text(json.dumps({'transform': [[0, -1, 0, 0], [1, 0, 0, -1], [0, 0, 1, 0], [0, 0, 0, 1]]}))
                return subprocess.CompletedProcess(command, 0)

            with mock.patch.object(sys, 'argv', argv+['--reference-sensor-to-body-json', str(calibration)]), \
                 mock.patch.object(MODULE.subprocess, 'run', side_effect=worker):
                MODULE.main()
            summary = json.loads((root/'out/summary.json').read_text())
            self.assertTrue(summary['cases'][0]['success'])
            self.assertLess(summary['cases'][0]['rotation_error_deg'], 1e-5)
            self.assertLess(summary['cases'][0]['translation_error_m'], 1e-12)

    def test_nonzero_and_timeout_workers_remain_in_denominator(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            reference = root/'reference.csv'
            reference.write_text('lidar_pose.x,lidar_pose.y,lidar_pose.z,lidar_pose.roll,lidar_pose.pitch,lidar_pose.yaw\n0,0,0,0,0,0\n')
            manifest = root/'protocol.json'
            manifest.write_text(json.dumps({'reference_csv_sha256': MODULE.sha(reference),
                'variants': [{'id': 'full', 'use_quatro': False, 'config': {}},
                             {'id': 'yaw', 'use_quatro': True, 'config': {}}],
                'cases': [{'id': 'smoke', 'kind': 'synthetic'}],
                'package_version': '1.0.2', 'voxel_size_m': .3, 'timeout_seconds': 1,
                'claim_limit': 'test'}))
            argv = ['runner', '--manifest', str(manifest), '--raw-root', str(root),
                    '--reference-csv', str(reference), '--output-dir', str(root/'out')]
            with mock.patch.object(sys, 'argv', argv), mock.patch.object(
                    MODULE.subprocess, 'run', side_effect=[subprocess.CompletedProcess([], 7),
                                                          subprocess.TimeoutExpired([], 1)]):
                MODULE.main()
            summary = json.loads((root/'out/summary.json').read_text())
            self.assertFalse(summary['all_commands_completed'])
            self.assertEqual(len(summary['cases']), 2)
            self.assertTrue(all(not row['success'] for row in summary['cases']))
            self.assertEqual(summary['cases'][0]['exit_code'], 7)
            self.assertTrue(summary['cases'][1]['timed_out'])
            for path in (root/'out').glob('*.job.json'):
                self.assertNotIn(str(reference), path.read_text())


if __name__ == '__main__':
    unittest.main()
