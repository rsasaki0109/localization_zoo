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
