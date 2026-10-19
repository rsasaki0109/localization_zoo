"""Guard checks for publishing closed checkpoints, with no network/native work."""
import importlib.util
import json
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import patch
from types import SimpleNamespace

spec = importlib.util.spec_from_file_location('bootstrap', Path(__file__).with_name('launch_and_back_up.py'))
bootstrap = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bootstrap)


class BackupGuards(unittest.TestCase):
    def check_rejected(self, paths, cleanup=None):
        def git(*args):
            if args[0] == 'diff':
                return paths
            if args[0] == 'show':
                return json.dumps(cleanup)
            self.fail(args)
        with patch.object(bootstrap, 'git', side_effect=git), patch.object(bootstrap.subprocess, 'run') as run:
            with self.assertRaises(RuntimeError):
                bootstrap.push_checkpoint('new-head', bootstrap.BASE)
            # Only the read-only ancestry check may run before rejection.
            self.assertEqual(run.call_args_list[0].args[0][:3], ['git', 'merge-base', '--is-ancestor'])
            self.assertFalse(any(c.args[0][:2] == ['git', 'push'] for c in run.call_args_list))

    def test_rejects_estimator_changes(self):
        self.check_rejected('common/src/filter.cpp')
        for name in ('pcd_dogfooding', 'baseline_pcd_dogfooding', 'candidate_pcd_dogfooding',
                     'run_all.py', 'recover_interrupted05.py', 'finish_when_complete.py'):
            with self.subTest(worker=name):
                self.assertTrue(bootstrap.owned_worker([b'python', ('/frozen/' + name).encode()]))
        self.assertFalse(bootstrap.owned_worker([b'python', b'/other/analysis.py']))

    def test_rejects_rewriting_closed_prefix(self):
        self.check_rejected('experiments/results/shared_near_range/sequences/03/cleanup.json')

    def test_rejects_unverified_or_retained_inputs(self):
        for verified, remains in ((False, False), (True, True)):
            with self.subTest(verified=verified, remains=remains):
                self.check_rejected('experiments/results/shared_near_range/sequences/05/cleanup.json',
                                    dict(hashes_verified_before_removal=verified, input_dir_exists_after=remains))

    def test_pushes_exact_commit_without_force_and_retains_real_exit(self):
        def git(*args):
            if args[0] == 'diff':
                return 'experiments/results/shared_near_range/sequences/05/cleanup.json'
            if args[0] == 'show':
                self.assertTrue(args[1].startswith('new-head:'))
                return json.dumps(dict(hashes_verified_before_removal=True, input_dir_exists_after=False))
            self.fail(args)
        for exit_code in (0, 1):
            with self.subTest(exit_code=exit_code), TemporaryDirectory() as tmp:
                calls = []
                def run(command, **kwargs):
                    calls.append(command)
                    return SimpleNamespace(returncode=exit_code, stdout='stdout', stderr='stderr')
                with patch.object(bootstrap, 'git', side_effect=git), patch.object(bootstrap, 'OUT', Path(tmp)), \
                     patch.object(bootstrap.subprocess, 'run', side_effect=run):
                    self.assertEqual(bootstrap.push_checkpoint('new-head', bootstrap.BASE), exit_code == 0)
                self.assertEqual(calls[-1], ['git', 'push', 'origin', 'new-head:' + bootstrap.BACKUP])
                receipt = json.loads((Path(tmp) / 'backup_commands.jsonl').read_text())
                self.assertEqual(receipt['exit_code'], exit_code)
                self.assertEqual(receipt['stderr'], 'stderr')


if __name__ == '__main__':
    unittest.main()
