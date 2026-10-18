"""Guard the owned-data deletion boundary used by interrupted-sequence recovery."""
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('range_recovery05', ROOT /
    'experiments/results/shared_near_range/recover_interrupted05.py')
RECOVERY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RECOVERY)


class RecoveryInputGuards(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.raw = self.root / 'velodyne'
        self.raw.mkdir()
        (self.raw / '000000.bin').write_bytes(b'owned scan')
        self.alias = self.root / 'scan_alias'
        self.alias.symlink_to(self.raw)
        self.expected = {'000000.bin': {'bytes': 10, 'sha256': hashlib.sha256(b'owned scan').hexdigest()}}

    def check(self):
        RECOVERY.verify_owned_inputs(self.raw, self.alias, self.expected)

    def test_exact_owned_bytes_and_alias_are_accepted(self):
        self.check()

    def test_same_size_changed_bytes_are_rejected(self):
        (self.raw / '000000.bin').write_bytes(b'other scan')
        with self.assertRaises(RuntimeError):
            self.check()
        self.assertTrue(self.raw.exists())

    def test_additional_file_prevents_cleanup(self):
        (self.raw / 'unowned.bin').write_bytes(b'extra')
        with self.assertRaises(RuntimeError):
            self.check()

    def test_foreign_alias_prevents_cleanup(self):
        self.alias.unlink()
        self.alias.symlink_to(self.root)
        with self.assertRaises(RuntimeError):
            self.check()

    def test_scan_symlink_prevents_cleanup(self):
        path = self.raw / '000000.bin'
        path.unlink()
        external = self.root / 'external.bin'
        external.write_bytes(b'owned scan')
        path.symlink_to(external)
        with self.assertRaises(RuntimeError):
            self.check()
        self.assertTrue(external.exists())


if __name__ == '__main__':
    unittest.main()
