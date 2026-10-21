"""Reconstruct archived uniform full results and the exact conditional controls."""
from pathlib import Path
import importlib.util,unittest
class RecursiveConsensusStudyTest(unittest.TestCase):
    def test_complete_native_evidence_controls_and_no_retuned_setting(self):
        path=Path(__file__).resolve().parents[1]/'experiments/results/inten_consensus_query_acceleration/verify.py'
        spec=importlib.util.spec_from_file_location('recursive_consensus_verify',path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        self.assertEqual(module.validate(history=False)['exit_code'],0)
