import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from evaluation.scripts.check_inten_trace import TraceError, validate_trace

REPO = Path(__file__).resolve().parents[1]
E = REPO / 'experiments/results/inten_trace_export/evidence'
CASES = [E / 'fixture/paper_default/candidate_on',
         E / 'fixture/paper_odometry_plane_check/candidate_on',
         E / 'skipped_input/candidate_on']
SCRIPT = REPO / 'evaluation/scripts/check_inten_trace.py'


class InTenTraceValidationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.records = [json.loads(line) for line in (CASES[0] / 'trace.jsonl').read_text().splitlines()]
        self.summary = json.loads((CASES[0] / 'summary.json').read_text())
        self.receipt = json.loads((CASES[0] / 'command.json').read_text())

    def write_case(self, records=None, summary=None, receipt=None):
        paths = [self.root / name for name in ['trace.jsonl', 'summary.json', 'receipt.json']]
        records = self.records if records is None else records
        paths[0].write_text(''.join(json.dumps(x) + '\n' for x in records))
        paths[1].write_text(json.dumps(self.summary if summary is None else summary))
        paths[2].write_text(json.dumps(self.receipt if receipt is None else receipt))
        return paths

    def check(self, records=None, summary=None, receipt=None):
        return validate_trace(*self.write_case(records, summary, receipt))

    def test_committed_normal_and_skipped_traces(self):
        for folder, processed, skipped in zip(CASES, [3, 3, 2], [0, 0, 1]):
            with self.subTest(folder=folder):
                report = validate_trace(folder / 'trace.jsonl', folder / 'summary.json', folder / 'command.json')
                self.assertEqual((report['input_frames'], report['processed_frames'], report['skipped_frames']), (3, processed, skipped))
                self.assertEqual(report['records'], 5)
                self.assertEqual(report['stages']['s2s']['calls'], processed - 1)
                self.assertEqual(report['stages']['s2m']['calls'], processed - 1)
                self.assertEqual(report['stages']['s2s']['stops']['not_run'], 1)
                self.assertEqual(report['nonfinite_values'], 0)

    def test_incomplete_and_post_end_records(self):
        for records, message in [(self.records[:-1], 'incomplete'), (self.records + [self.records[1]], 'after end')]:
            with self.subTest(message=message), self.assertRaisesRegex(TraceError, message):
                self.check(records)
        bad = copy.deepcopy(self.records); bad[-1]['processed_frames'] += 1
        with self.assertRaisesRegex(TraceError, 'end counters'):
            self.check(bad)

    def test_input_and_estimator_indices_are_distinct(self):
        bad = [json.loads(line) for line in (CASES[2] / 'trace.jsonl').read_text().splitlines()]
        summary = json.loads((CASES[2] / 'summary.json').read_text())
        receipt = json.loads((CASES[2] / 'command.json').read_text())
        bad[3]['frame_index'] = bad[3]['input_index']
        with self.assertRaisesRegex(TraceError, 'estimator frame index'):
            self.check(bad, summary, receipt)
        bad = copy.deepcopy(self.records); bad[2]['input_index'] = 0
        with self.assertRaisesRegex(TraceError, 'input index'):
            self.check(bad)

    def test_receipt_exit_and_cross_run_binding(self):
        receipt = copy.deepcopy(self.receipt); receipt['exit_code'] = 1
        with self.assertRaisesRegex(TraceError, 'not exit 0'):
            self.check(receipt=receipt)
        receipt = json.loads((CASES[1] / 'command.json').read_text())
        with self.assertRaisesRegex(TraceError, 'argv mismatch'):
            self.check(receipt=receipt)
        receipt = copy.deepcopy(self.receipt); del receipt['exit_code']
        with self.assertRaisesRegex(TraceError, 'native summary/receipt'):
            self.check(receipt=receipt)

    def test_failed_step_tags_survive_and_literal_nan_is_rejected(self):
        records = copy.deepcopy(self.records)
        solver = records[-2]['s2s']; last = solver['iterations'][-1]
        solver['stop'] = 'nonfinite_step'; solver['returned_pairs'] = 0
        last['delta'][0] = 'NaN'; last['H'][0] = '+Infinity'
        last['step_finite'] = last['update_applied'] = False
        records[-2]['s2s_fallback'] = True
        self.assertEqual(self.check(records)['nonfinite_values'], 2)
        last['delta'][0] = float('nan')
        with self.assertRaisesRegex(TraceError, 'nonstandard JSON numeric literal'):
            self.check(records)

    def test_support_shapes_and_flag_corruption(self):
        for field in ['counts', 'matrix', 'flag', 'huge_integer']:
            records = copy.deepcopy(self.records)
            it = records[1 + 1]['s2s']['iterations'][0]
            if field == 'counts': it['class_pairs'][0] += 1
            elif field == 'matrix': it['H'].pop()
            elif field == 'flag': it['step_attempted'] = False
            else: it['H'][0] = 10 ** 500
            with self.subTest(field=field), self.assertRaises(TraceError):
                self.check(records)

    def test_unknown_schema_duplicate_keys_and_uncalled_stage(self):
        records = copy.deepcopy(self.records); records[0]['version'] = 99
        with self.assertRaisesRegex(TraceError, 'schema/version'):
            self.check(records)
        paths = self.write_case(); paths[0].write_text('{"type":"header","type":"frame"}\n')
        with self.assertRaisesRegex(TraceError, 'duplicate JSON key'):
            validate_trace(*paths)
        records = copy.deepcopy(self.records); records[2]['s2s']['called'] = False
        with self.assertRaisesRegex(TraceError, 'called/stop'):
            self.check(records)

    def test_summary_status_and_frame_count_binding(self):
        for field in ['status', 'frames', 'pcd_dir']:
            summary = copy.deepcopy(self.summary)
            if field == 'status': summary['methods'][0]['status'] = 'failed'
            elif field == 'frames': summary['methods'][0]['frames'] -= 1
            else: summary['pcd_dir'] = 'different-input'
            with self.subTest(field=field), self.assertRaises(TraceError):
                self.check(summary=summary)

    def test_cli_only_publishes_after_validation_and_preserves_existing_output(self):
        paths = self.write_case(); output = self.root / 'report.json'
        cmd = [sys.executable, str(SCRIPT), '--trace', str(paths[0]), '--summary', str(paths[1]),
               '--receipt', str(paths[2]), '--output', str(output)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        original = output.read_bytes()
        self.assertEqual(subprocess.run(cmd, capture_output=True).returncode, 1)
        self.assertEqual(output.read_bytes(), original)
        output.unlink(); paths[0].write_text(paths[0].read_text().rsplit('\n', 2)[0] + '\n')
        self.assertEqual(subprocess.run(cmd, capture_output=True).returncode, 1)
        self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
