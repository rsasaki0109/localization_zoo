import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

import numpy as np

SCRIPTS = Path(__file__).resolve().parents[1]/'evaluation/scripts'
sys.path.insert(0, str(SCRIPTS))
from reference_frames import load_sensor_to_body, sensor_frame_poses


class ReferenceFrameTests(unittest.TestCase):
    def test_reference_poses_use_right_multiplication_and_relative_conjugation(self):
        body0, body1, extrinsic = np.eye(4), np.eye(4), np.eye(4)
        body1[:2, :2] = [[0, -1], [1, 0]]
        body1[:3, 3] = [2, 3, 4]
        extrinsic[:3, :3] = np.diag([1., -1., -1.])
        extrinsic[:3, 3] = [.1, .2, .3]
        sensor = sensor_frame_poses([body0, body1], extrinsic)
        relative = np.linalg.solve(sensor[1], sensor[0])
        point = np.array([1., 2., 3., 1.])
        np.testing.assert_allclose(sensor[1] @ relative @ point, body0 @ extrinsic @ point)
        np.testing.assert_allclose(relative, np.linalg.inv(extrinsic) @ np.linalg.inv(body1) @ body0 @ extrinsic)

    def test_scan_stamp_marker_is_not_a_homogeneous_transform(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'extrinsic.json'
            matrix = np.eye(4)
            matrix[3, 3] = 0
            path.write_text(json.dumps({'T_body_sensor': matrix.tolist()}))
            with self.assertRaisesRegex(ValueError, 'homogeneous'):
                load_sensor_to_body(path)

    def test_mcd_legacy_bytes_and_explicit_sensor_conversion(self):
        spec = importlib.util.spec_from_file_location('mcd_converter', SCRIPTS/'mcd_gt_to_csv.py')
        converter = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(converter)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, output = root/'body.csv', root/'out.csv'
            source.write_text('num,t,x,y,z,qx,qy,qz,qw\n0,1,1,2,3,0,0,0,1\n')
            argv = ['converter', '--input', str(source), '--output', str(output)]
            with mock.patch.object(sys, 'argv', argv):
                self.assertEqual(converter.main(), 0)
            expected = ('timestamp,lidar_pose.x,lidar_pose.y,lidar_pose.z,lidar_pose.roll,lidar_pose.pitch,lidar_pose.yaw\r\n'
                        '1.000000000,1.0000000000,2.0000000000,3.0000000000,0.0000000000,0.0000000000,0.0000000000\r\n')
            self.assertEqual(output.read_bytes(), expected.encode())
            extrinsic = np.eye(4)
            extrinsic[:2, :2] = [[0, -1], [1, 0]]
            extrinsic[:3, 3] = [.1, .2, .3]
            path = root/'extrinsic.json'
            path.write_text(json.dumps({'T_body_sensor': extrinsic.tolist()}))
            with mock.patch.object(sys, 'argv', argv+['--sensor-to-body-json', str(path)]):
                self.assertEqual(converter.main(), 0)
            values = np.loadtxt(output, delimiter=',', skiprows=1)
            np.testing.assert_allclose(values[1:4], [1.1, 2.2, 3.3], atol=1e-10)
            np.testing.assert_allclose(values[4:], [0, 0, np.pi/2], atol=1e-10)


if __name__ == '__main__':
    unittest.main()
