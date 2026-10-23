"""Historical provenance must survive development and still detect corruption."""
import gzip
import importlib.util
import json
from pathlib import Path
import tempfile
import threading
import time
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
STUDY = ROOT / 'experiments/results/inten_support_information'
spec = importlib.util.spec_from_file_location(
    'support_source_binding', ROOT / 'evaluation/scripts/verify_inten_support_history.py')
history = importlib.util.module_from_spec(spec)
spec.loader.exec_module(history)
SOURCE = 'papers/inten_loam/src/inten_loam_paper.cpp'
ARCHIVE = STUDY / 'checks/build/sources' / (SOURCE + '.gz')


class HistoricalSourceBindingTest(unittest.TestCase):
    def test_all_affected_studies_pass_full_source_preflight(self):
        studies = [STUDY, STUDY.parent / 'inten_prior_control', STUDY.parent / 'inten_local_rigidity']
        for study in studies:
            options = history.rigidity_sources(study) if study.name == 'inten_local_rigidity' else {}
            with self.subTest(study=study.name), history.historical_sources(study, **options) as root:
                protocol = json.loads((study / 'protocol.json').read_text())
                for name, expected in protocol['source_sha256'].items():
                    self.assertEqual(history.digest((root / name).read_bytes()), expected)

    def test_changed_historical_test_snapshot_is_rejected(self):
        study = STUDY.parent / 'inten_local_rigidity'
        options = history.rigidity_sources(study)
        snapshot = options['extra_sources']['tests/test_inten_local_rigidity.py']
        original = Path.read_bytes
        with patch.object(Path, 'read_bytes', lambda p: b'changed test' if p == snapshot else original(p)):
            with self.assertRaisesRegex(ValueError, 'Archived production source changed'):
                with history.historical_sources(study, **options):
                    self.fail('Changed historical test snapshot accepted')

    def test_current_production_changes_do_not_rebind_historical_results(self):
        original = Path.read_bytes
        def changed(path):
            return b'new estimator implementation' if path == ROOT / SOURCE else original(path)
        with patch.object(Path, 'read_bytes', changed):
            with history.historical_sources(STUDY) as root:
                self.assertEqual((root / SOURCE).read_bytes(), gzip.decompress(original(ARCHIVE)))
                self.assertEqual((root / 'experiments').resolve(), ROOT / 'experiments')
        self.assertFalse(root.exists())

    def test_changed_archived_source_is_rejected_even_with_valid_gzip(self):
        original = Path.read_bytes
        corrupted = gzip.compress(gzip.decompress(original(ARCHIVE)) + b'\nchanged\n')
        with patch.object(Path, 'read_bytes', lambda p: corrupted if p == ARCHIVE else original(p)):
            with self.assertRaisesRegex(ValueError, 'Archived production source changed'):
                with history.historical_sources(STUDY):
                    self.fail('Corrupt archive accepted')

    def test_changed_manifest_is_rejected_against_frozen_protocol(self):
        manifest = STUDY / 'checks/build/source_manifest.json'
        original = Path.read_text
        data = json.loads(original(manifest)); data[SOURCE]['sha256'] = '0' * 64
        with patch.object(Path, 'read_text', lambda p: json.dumps(data) if p == manifest else original(p)):
            with self.assertRaisesRegex(ValueError, 'Archived source manifest differs'):
                with history.historical_sources(STUDY):
                    self.fail('Corrupt manifest accepted')

    def test_current_historical_verifier_corruption_is_not_hidden_by_archive(self):
        target = STUDY / 'verify.py'
        original = Path.read_bytes
        with patch.object(Path, 'read_bytes', lambda p: b'changed verifier' if p.resolve() == target else original(p)):
            with self.assertRaisesRegex(ValueError, 'Frozen source changed'):
                with history.historical_sources(STUDY):
                    self.fail('Corrupt verifier accepted')

    def test_missing_archive_has_no_working_tree_fallback(self):
        original = Path.read_bytes
        def missing(path):
            if path == ARCHIVE:
                raise FileNotFoundError(path)
            return original(path)
        with patch.object(Path, 'read_bytes', missing):
            with self.assertRaises(FileNotFoundError):
                with history.historical_sources(STUDY):
                    self.fail('Missing archive accepted')

    def test_verifier_failure_restores_source_root(self):
        checker = SimpleNamespace(W=ROOT)
        def fail(folder):
            self.assertNotEqual(checker.W, ROOT)
            self.assertTrue((checker.W / SOURCE).exists())
            raise RuntimeError('original verifier failure')
        original_executor = object()
        verifier = SimpleNamespace(E=STUDY, checker=checker, validate_in=fail,
                                   ThreadPoolExecutor=original_executor)
        with tempfile.TemporaryDirectory() as folder:
            with self.assertRaisesRegex(RuntimeError, 'original verifier failure'):
                history.validate_in(verifier, folder)
        self.assertEqual(checker.W, ROOT)
        self.assertIs(verifier.ThreadPoolExecutor, original_executor)

    def test_all_workers_execute_once_without_overlapping(self):
        active = 0
        peak = 0
        seen = []
        lock = threading.Lock()
        def work(group):
            nonlocal active, peak
            with lock:
                active += 1
                peak = max(peak, active)
                seen.append(group)
            time.sleep(.01)
            with lock:
                active -= 1
            return group
        with history.SerialWorkers(max_workers=2) as pool:
            results = list(pool.map(work, ['old', '01', '08', '02']))
        self.assertEqual(results, ['old', '01', '08', '02'])
        self.assertEqual(seen, results)
        self.assertEqual(peak, 1)

    def test_archive_paths_cannot_escape_source_fixture(self):
        for value in ('/etc/passwd', '../outside', 'sources/../../outside', ''):
            with self.subTest(value=value), self.assertRaises(ValueError):
                history.relative_path(value)


if __name__ == '__main__':
    unittest.main()
