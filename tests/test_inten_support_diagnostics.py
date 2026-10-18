"""Reject logs that could silently undercount native fallback evidence."""
import importlib.util
from pathlib import Path
import unittest

PATH = Path(__file__).resolve().parents[1] / 'experiments/results/inten_support_diagnostics/analyze.py'
SPEC = importlib.util.spec_from_file_location('inten_support_diagnostics', PATH)
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def progress(frame, s2s=100, s2m=120, classes='30/30/30/30'):
    return (f'[InTEn-LOAM paper] {frame}/21 g/f/e/r=100/50/20/10 '
            f's2s={s2s} s2m={s2m} (e/g/f/i {classes})\r')


class IntenSupportDiagnosticsTest(unittest.TestCase):
    def test_missing_duplicate_and_corrupt_records_are_rejected(self):
        good = progress(0) + progress(10) + progress(20)
        self.assertEqual(len(MODULE.parse_progress(good, 21, 10)), 3)
        for bad in (progress(0) + progress(20), good + progress(20),
                    progress(0) + progress(10, s2m=119) + progress(20),
                    progress(0) + progress(20) + progress(10)):
            with self.subTest(log=bad), self.assertRaises(ValueError):
                MODULE.parse_progress(bad, 21, 10)

    def test_cutoff_is_strict_and_empty_evidence_is_not_a_zero_minimum(self):
        rows = MODULE.parse_progress(progress(0) + progress(10, s2s=29, s2m=30, classes='10/10/10/0')
                                     + progress(20, s2s=30, s2m=29, classes='10/10/9/0'), 21, 10)[1:]
        summary = MODULE.support_summary(rows, 30)
        self.assertEqual(summary['s2s_below_cutoff'], 1)
        self.assertEqual(summary['s2m_below_cutoff'], 1)
        self.assertIsNone(MODULE.support_summary([], 30)['s2s_min'])
        self.assertIsNone(MODULE.support_summary([], 30)['s2m_min'])


if __name__ == '__main__':
    unittest.main()
