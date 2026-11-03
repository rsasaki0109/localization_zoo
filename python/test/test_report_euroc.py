import json
from pathlib import Path
import tempfile
import unittest

import test_evaluate_euroc
from evaluate_euroc import evaluate
from report_euroc import render


class ReportTests(unittest.TestCase):
    def test_prefix_preserves_reference_and_missing_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fixture = test_evaluate_euroc.EvaluationTests()
            trajectory, rows = fixture.fixture(root)
            fixture.write(trajectory, rows[:1])
            result = evaluate(root, trajectory, include_samples=True)
            self.assertEqual(result['samples'][1]['status'], 'missing')
            self.assertIsNone(result['samples'][1]['estimate_xyz'])
            self.assertIsNotNone(result['samples'][1]['reference_xyz'])
            self.assertIsNone(result['samples'][1]['translation_error_m'])
            out = root / 'report'
            render(result, out)
            self.assertEqual(json.loads((out / 'metrics.json').read_text())['tracking_fraction'], 0.5)
            self.assertIn('<svg', (out / 'trajectory-errors.svg').read_text())
            self.assertIn('<svg', (out / 'coverage.svg').read_text())
            self.assertIn('scale fixed at 1', (out / 'index.html').read_text())
            with self.assertRaises(FileExistsError):
                render(result, out)

    def test_all_lost_report_has_null_errors(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fixture = test_evaluate_euroc.EvaluationTests()
            trajectory, rows = fixture.fixture(root)
            for row in rows:
                row.update(status='lost', pose=None)
            fixture.write(trajectory, rows)
            result = evaluate(root, trajectory, include_samples=True)
            out = root / 'report'
            render(result, out)
            self.assertIsNone(json.loads((out / 'metrics.json').read_text())['translation_rmse_m'])
            self.assertTrue(all(s['translation_error_m'] is None for s in result['samples']))


if __name__ == '__main__':
    unittest.main()
