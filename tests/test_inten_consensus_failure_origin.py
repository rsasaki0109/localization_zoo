"""Ensure the new diagnostic distinguishes seed damage from refinement damage."""
from pathlib import Path
import importlib.util
import unittest
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
E = ROOT/'experiments/results/inten_consensus_failure_origin'


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result)
    return result


class FailureOriginTest(unittest.TestCase):
    def test_reverse_motion_and_zero_motion_are_distinct(self):
        m = module('origin_metrics', E/'analyze.py')
        gt = np.eye(4); gt[0, 3] = 2.
        reverse = np.eye(4); reverse[0, 3] = -2.
        values = m.stage_metrics(reverse, gt)
        self.assertFalse(values['bad_length'])
        self.assertTrue(values['bad_vector'])
        self.assertEqual(values['forward_ratio'], -1.)
        stopped = m.stage_metrics(np.eye(4), gt)
        self.assertTrue(stopped['bad_length'])
        self.assertTrue(stopped['bad_vector'])
        self.assertIsNone(m.stage_metrics(np.eye(4), np.eye(4))['length_ratio'])

    def test_stage_crossing_does_not_blame_already_bad_joint(self):
        m = module('origin_crossings', E/'analyze.py')
        self.assertEqual(m.crossing(False, True, True), 'joint_good_seed_bad_accepted_bad')
        self.assertEqual(m.crossing(False, False, True), 'joint_good_seed_good_accepted_bad')
        self.assertEqual(m.crossing(True, True, True), 'joint_already_bad')
        self.assertEqual(m.crossing(True, True, False), 'joint_bad_to_accepted_good')

    def test_measured_kernel_roundoff_accepted_but_real_drift_rejected(self):
        import json, math
        m = module('origin_portable_ratios', E/'analyze.py')
        for kernel in ('Haswell', 'Sandybridge'):
            probe = json.loads((E/'checks/portability'/(kernel+'.json')).read_text())
            self.assertTrue(probe['strict_failures'])
            for r in probe['strict_failures']:
                self.assertFalse(math.isclose(r['committed'], r['reconstructed'], rel_tol=1e-12, abs_tol=1e-12))
                self.assertTrue(m.ratio_agrees(r['committed'], r['reconstructed']))
        self.assertFalse(m.ratio_agrees(1., 1.000001))

    def test_all_frozen_full_traces_reconstruct_report(self):
        m = module('origin_verification', E/'verify.py')
        self.assertEqual(m.validate(history=False)['exit_code'], 0)
