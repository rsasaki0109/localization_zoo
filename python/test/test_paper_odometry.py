#!/usr/bin/env python3
"""Check the actual Python boundary against native paper pipelines on real scans."""
import argparse
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'python'))
from example_paper_odometry import METHODS, load_scans, process
from synthetic_paper_scan import moving_room_scans


class PaperBindings(unittest.TestCase):
    reports = {}

    @classmethod
    def setUpClass(cls):
        cls.scans = load_scans(ROOT / 'evaluation/fixtures/mcd_kth_smoke')

    @classmethod
    def tearDownClass(cls):
        if ARGS.report:
            ARGS.report.write_text(json.dumps({'methods': cls.reports}, indent=2) + '\n')

    def test_native_parity_and_independent_pose_copies(self):
        with tempfile.TemporaryDirectory() as directory:
            paths = []
            for i, (_, scan) in enumerate(self.scans):
                path = Path(directory) / f'{i}.f64'
                scan.astype('<f8').tofile(path)
                paths.append(str(path))
            for method, name in METHODS.items():
                with self.subTest(method=method):
                    completed = subprocess.run([str(ARGS.reference), method, *paths],
                                               check=True, capture_output=True, text=True)
                    reference = np.loadtxt(io.StringIO(completed.stdout), delimiter=',', ndmin=2)
                    odometry = getattr(lz, name)()
                    rows = []
                    for i, (_, scan) in enumerate(self.scans):
                        if i == 0:
                            scan = np.asfortranarray(scan)
                        elif i == 1:
                            storage = np.zeros((len(scan), 6))
                            storage[:, ::2] = scan
                            scan = storage[:, ::2]
                        pose, state, support = process(odometry, method, scan)
                        self.assertTrue(np.isfinite(pose).all())
                        np.testing.assert_array_equal(odometry.pose, pose)
                        rows.append([*pose.reshape(-1), state, support])
                    # No CLI profiles, GT seeds, or Python accumulation: the same
                    # native defaults process exactly the same float64 inputs.
                    np.testing.assert_allclose(rows, reference, rtol=0, atol=1e-12)
                    actual = np.asarray(rows)
                    self.reports[method] = {
                        'frames': len(rows),
                        'max_abs_pose_delta': float(np.max(np.abs(actual[:, :16] - reference[:, :16]))),
                        'diagnostics_equal': bool(np.array_equal(actual[:, 16:], reference[:, 16:])),
                        'python_poses': actual[:, :16].tolist(),
                        'native_poses': reference[:, :16].tolist(),
                    }
                    self.assertGreater(reference[-1, 16], 0)
                    owned = odometry.pose
                    owned[0, 3] += 100
                    self.assertNotEqual(owned[0, 3], odometry.pose[0, 3])

    def test_dilo_moving_output_matches_native_with_the_same_defaults(self):
        scans = moving_room_scans()
        with tempfile.TemporaryDirectory() as directory:
            paths = []
            for i, (_, scan) in enumerate(scans):
                path = Path(directory) / f'{i}.f64'
                scan.astype('<f8').tofile(path)
                paths.append(str(path))
            completed = subprocess.run([str(ARGS.reference), 'dilo_paper', *paths],
                                       check=True, capture_output=True, text=True)
            reference = np.loadtxt(io.StringIO(completed.stdout), delimiter=',')
            odometry = lz.DiloPaperOdometry()
            actual = np.array([[*pose.reshape(-1), state, support]
                               for _, scan in scans
                               for pose, state, support in [process(odometry, 'dilo_paper', scan)]])
            self.assertGreater(np.linalg.norm(reference[-1, :16].reshape(4, 4) - np.eye(4)), 1e-6)
            np.testing.assert_allclose(actual, reference, rtol=0, atol=1e-12)
            self.reports['dilo_synthetic'] = {
                'frames': len(scans), 'nonidentity_native_output': True,
                'max_abs_pose_delta': float(np.max(np.abs(actual[:, :16] - reference[:, :16]))),
                'diagnostics_equal': bool(np.array_equal(actual[:, 16:], reference[:, 16:])),
                'python_poses': actual[:, :16].tolist(),
                'native_poses': reference[:, :16].tolist(),
            }

    def test_invalid_arrays_do_not_advance_stream(self):
        for method, name in METHODS.items():
            with self.subTest(method=method):
                odometry = getattr(lz, name)()
                for invalid in [np.empty((0, 3)), np.full((2, 3), np.nan), np.full((2, 3), np.inf)]:
                    with self.assertRaises(ValueError):
                        odometry.process(invalid)
                with self.assertRaises((TypeError, ValueError)):
                    odometry.process(np.zeros((2, 4)))
                pose, _, _ = process(odometry, method, self.scans[0][1])
                np.testing.assert_array_equal(pose, np.eye(4))

    def test_existing_opt_in_defaults_and_enum_roundtrip(self):
        self.assertFalse(lz.SumaPaperParams().normalize_bilinear)
        self.assertEqual(lz.ImlsPaperParams().coarse_frames, 0)
        self.assertEqual(lz.DiloPaperParams().degenerate_ratio, 0)
        params = lz.Litamin2OdometryParams()
        params.map_mode = lz.Litamin2MapMode.LATEST
        self.assertEqual(params.map_mode, lz.Litamin2MapMode.LATEST)
        params.lambda_ = 0.001
        self.assertEqual(params.lambda_, 0.001)
        self.assertNotIsInstance(lz.Litamin2Odometry(params), lz.Litamin2)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--module-dir', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--report', type=Path)
    ARGS, remaining = parser.parse_known_args()
    sys.path.insert(0, str(ARGS.module_dir.resolve()))
    import localization_zoo as lz
    unittest.main(argv=[sys.argv[0], *remaining])
