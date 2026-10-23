"""Verify consensus decisions against actual geometry profiles and native controls."""
from pathlib import Path
import importlib.util
import unittest


class TranslationConsensusStudyTest(unittest.TestCase):
    def test_data_derived_factorial_and_prior_negative_control(self):
        path=Path(__file__).resolve().parents[1]/'experiments/results/inten_translation_consensus/verify.py'
        spec=importlib.util.spec_from_file_location('translation_consensus_verify',path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        self.assertEqual(module.validate(history=False)['exit_code'],0)
