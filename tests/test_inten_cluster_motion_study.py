"""Keep a diagnostic mask tied to the actual frozen profiles and native controls."""
from pathlib import Path
import importlib.util
import unittest


class ClusterMotionStudyTest(unittest.TestCase):
    def test_archives_decisions_and_unchanged_native_control(self):
        source = Path(__file__).resolve().parents[1]/'experiments/results/inten_cluster_motion/verify.py'
        spec = importlib.util.spec_from_file_location('cluster_motion_verify', source)
        module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
        self.assertEqual(module.validate(history=False)['exit_code'], 0)
