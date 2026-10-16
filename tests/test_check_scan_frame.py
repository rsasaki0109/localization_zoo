"""Sensor geometry and file-format contracts, independent of KITTI/NCLT files."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'evaluation/scripts'))
from check_scan_frame import Scan, analyze, load_points, load_scan, ordered_rows, transform_points


def spinning_scan(angles, *, ground=-1.7, points_per_ring=240, seed=3):
    rng = np.random.default_rng(seed)
    xyz, rings = [], []
    azimuth = np.linspace(.02, 2 * np.pi, points_per_ring, endpoint=False)
    for ring, angle in enumerate(angles):
        tangent = np.tan(np.deg2rad(angle))
        horizontal = (np.full(points_per_ring, ground / tangent) if angle < -2
                      else rng.uniform(6, 35, points_per_ring))
        xyz.append(np.column_stack([horizontal * np.cos(azimuth), horizontal * np.sin(azimuth), horizontal * tangent]))
        rings.extend([ring] * points_per_ring)
    points = np.vstack(xyz)
    return Scan(points, np.linspace(0, 1, len(points)), np.array(rings))


class SensorGeometryTest(unittest.TestCase):
    def test_uneven_beams_use_ring_major_order_and_do_not_require_periodicity(self):
        angles = np.r_[np.linspace(2, -8.5, 32), np.linspace(-9, -24.9, 32)]
        original = spinning_scan(angles)
        scan = Scan(original.points, original.intensity)
        self.assertEqual(ordered_rows(scan.points), 64)
        result = analyze([scan])
        self.assertEqual(result['rows'], 64)
        self.assertEqual(result['sensor_frame']['status'], 'assumed_identity')
        self.assertEqual(result['sensor_frame']['pose'], [0] * 6)
        self.assertEqual(result['fov']['nominal_up_deg'], 2)
        self.assertEqual(result['fov']['nominal_down_deg'], -24.9)
        self.assertIsNone(result['beam_spacing_deg'])
        self.assertEqual(result['method_flags']['dilo'][-4:], ['--dilo-paper-height', '192', '--dilo-paper-max-fill-gap', '4'])

    def test_wrap_jitter_does_not_add_a_laser(self):
        scan = spinning_scan(np.linspace(2, -24.9, 64))
        p = scan.points.copy()
        # Two extra boundary crossings cancel in net azimuth winding.
        k = np.flatnonzero(np.diff(np.arctan2(p[:, 1], p[:, 0])) < -np.pi)[0]
        p[k + 1, 1] = abs(p[k + 1, 1])
        p[k + 2, 1] = -abs(p[k + 2, 1])
        self.assertEqual(ordered_rows(p), 64)

    def test_inverted_translated_uniform_scans_recover_axial_frame(self):
        sensor = spinning_scan(np.linspace(-32 / 3, 92 / 3, 32), ground=-.8)
        body = sensor.points * [1, -1, -1] + [0, 0, -.957]
        body = np.round((body + 100) / .005) * .005 - 100
        scan = Scan(body, sensor.intensity * 255, sensor.rings)
        result = analyze([scan])
        frame = result['sensor_frame']
        self.assertEqual(frame['status'], 'axial_estimate')
        self.assertAlmostEqual(frame['pose'][2], -.957, delta=.02)
        self.assertEqual(frame['pose'][3], 180)
        self.assertEqual(result['rows'], 32)
        self.assertEqual(result['fov']['nominal_up_deg'], 31)
        self.assertEqual(result['fov']['nominal_down_deg'], -11)
        self.assertEqual(result['intensity']['range_hint'], '0..255')
        self.assertEqual(result['method_flags']['dilo'][-4:], ['--dilo-paper-height', '240', '--dilo-paper-max-fill-gap', '8'])

    def test_known_extrinsics_transform_all_axes_and_near_range_in_sensor_frame(self):
        pose = [10, -2, 1, 20, -10, 30]
        # Rotation preserves distance; stored origin must not define proximity.
        rng = np.random.default_rng(10)
        points = rng.normal(size=(1000, 3)) * .1 + [10, -2, 1]
        result = analyze([Scan(points)], input_sensor_pose=pose, rows=16)
        self.assertEqual(result['near_range']['fraction'], 1)
        self.assertEqual(result['sensor_frame']['status'], 'provided')
        np.testing.assert_allclose(np.linalg.norm(transform_points(points, pose), axis=1),
                                   np.linalg.norm(points - pose[:3], axis=1), atol=1e-12)
        np.testing.assert_allclose(transform_points(np.array([[1, 3, 3]]), [1, 2, 3, 0, 0, 90]),
                                   [[1, 0, 0]], atol=1e-12)

    def test_missing_ground_does_not_resolve_up_axis_from_periodicity_alone(self):
        rng = np.random.default_rng(4)
        points = rng.normal(size=(4000, 3)) * [10, 10, 8]
        result = analyze([Scan(points)], beam_spacing_deg=1.333)
        self.assertEqual(result['sensor_frame']['status'], 'unresolved')
        self.assertEqual(result['method_flags'], {})
        self.assertIsNone(result['near_range'])

    def test_shuffled_scan_does_not_claim_ordered_rings_or_a_sensor_model(self):
        rng = np.random.default_rng(9)
        p = spinning_scan(np.linspace(2, -24.9, 64)).points
        rng.shuffle(p)
        self.assertIsNone(ordered_rows(p))
        result = analyze([Scan(p)])
        self.assertEqual(result['sensor_model_candidate'], 'unknown')
        self.assertEqual(result['sensor_frame']['status'], 'unresolved')

    def test_nonfinite_xyz_filtered_and_missing_intensity_is_not_zero_intensity(self):
        p = spinning_scan(np.linspace(2, -24.9, 64)).points
        p = np.vstack([p, [np.nan, 0, 1], [0, 0, 0]])
        result = analyze([Scan(p)], input_sensor_pose=[0] * 6)
        self.assertEqual(result['invalid_points_removed'], 2)
        self.assertEqual(result['intensity']['status'], 'missing')
        self.assertIsNone(result['intensity']['min'])
        self.assertTrue(all('--input-min-range-m' not in flags for flags in result['method_flags'].values()))

    def test_invalid_parameters_and_insufficient_points_rejected(self):
        scan = spinning_scan(np.linspace(2, -24.9, 64))
        for kwargs in [{'rows': 0}, {'max_offset_m': -1}, {'beam_spacing_deg': 0},
                       {'input_sensor_pose': [0, 0, 0, np.inf, 0, 0]}]:
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                analyze([scan], **kwargs)
        with self.assertRaises(ValueError):
            analyze([Scan(np.zeros((3, 3)))])


class ScanFormatTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name)

    def test_kitti_xyzi_and_explicit_nclt_layout_preserve_intensity_and_rings(self):
        kitti = self.path / 'kitti.bin'
        np.array([[1, 2, 3, .5]], dtype='<f4').tofile(kitti)
        scan = load_scan(kitti)
        np.testing.assert_allclose(scan.points, [[1, 2, 3]])
        np.testing.assert_allclose(scan.intensity, [.5])
        nclt = self.path / 'nclt.bin'
        nclt.write_bytes(struct.pack('<HHHBB', 20000, 20200, 19800, 255, 7))
        scan = load_scan(nclt, 'nclt')
        np.testing.assert_allclose(scan.points, [[0, 1, -1]])
        self.assertEqual(scan.intensity.tolist(), [255])
        self.assertEqual(scan.rings.tolist(), [7])

    def test_ascii_pcd_count_fields_and_reordered_columns(self):
        p = self.path / 'cloud.pcd'
        p.write_text('FIELDS intensity descriptor z x y\nSIZE 4 4 4 4 4\nTYPE F F F F F\nCOUNT 1 2 1 1 1\nWIDTH 1\nHEIGHT 1\nPOINTS 1\nDATA ascii\n42 8 9 3 1 2\n')
        scan = load_scan(p)
        np.testing.assert_allclose(scan.points, [[1, 2, 3]])
        self.assertEqual(scan.intensity.tolist(), [42])
        np.testing.assert_allclose(load_points(p), scan.points)

    def test_binary_pcd_organized_rows_and_integer_intensity(self):
        p = self.path / 'cloud.pcd'
        header = b'FIELDS x y z intensity ring\nSIZE 4 4 4 1 1\nTYPE F F F U U\nCOUNT 1 1 1 1 1\nWIDTH 1\nHEIGHT 2\nPOINTS 2\nDATA binary\n'
        p.write_bytes(header + struct.pack('<fffBB', 1, 2, 3, 200, 0) + struct.pack('<fffBB', 4, 5, 6, 255, 1))
        scan = load_scan(p)
        self.assertEqual(scan.organized_rows, 2)
        self.assertEqual(scan.intensity.tolist(), [200, 255])
        np.testing.assert_allclose(scan.points, [[1, 2, 3], [4, 5, 6]])

    def test_truncated_records_and_compressed_pcd_fail_clearly(self):
        p = self.path / 'cloud.bin';p.write_bytes(b'bad')
        with self.assertRaisesRegex(ValueError, '16-byte'):
            load_scan(p)
        p = self.path / 'cloud.pcd'
        p.write_text('FIELDS x y z\nWIDTH 1\nHEIGHT 1\nPOINTS 1\nDATA binary_compressed\n')
        with self.assertRaisesRegex(ValueError, 'uncompressed'):
            load_scan(p)

    def test_cli_records_inputs_and_provided_pose_without_applying_overrides(self):
        p = self.path / 'cloud.bin'
        scan = spinning_scan(np.r_[np.linspace(2, -8.5, 32), np.linspace(-9, -24.9, 32)])
        np.column_stack([scan.points, scan.intensity]).astype('<f4').tofile(p)
        out = self.path / 'report.json'
        script = Path(__file__).resolve().parents[1] / 'evaluation/scripts/check_scan_frame.py'
        result = subprocess.run([sys.executable, str(script), str(p), '--json', '--json-output', str(out)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertEqual(report, json.loads(out.read_text()))
        self.assertEqual(len(report['inputs'][0]['sha256']), 64)
        self.assertTrue(report['command'])
        self.assertEqual(report['rows'], 64)


if __name__ == '__main__':
    unittest.main()
