"""Reconstruct the actual conditional rotation experiment, including negatives."""
from pathlib import Path
import importlib.util,unittest
class FixedRotationStudyTest(unittest.TestCase):
    def test_actual_native_controls_projected_equations_and_all_outcomes(self):
        path=Path(__file__).resolve().parents[1]/'experiments/results/inten_fixed_rotation/verify.py'
        spec=importlib.util.spec_from_file_location('fixed_rotation_verify',path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        self.assertEqual(module.validate(history=False)['exit_code'],0)
