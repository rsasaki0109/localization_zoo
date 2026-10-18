"""One-shot continuation of the running study, with no native replays of its own.

Waits for all committed sequence checkpoints, then reports, documents, checks
and optionally pushes the same branch. PR metadata uses the connected tool in
the next active chat turn; this worker does not use the blocked GitHub CLI API.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import traceback

from support import E, REPO, R, CONTAINER_PY, PROTOCOL, SOURCE, docker, run, write, verify_binaries

BRANCH = 'codex/shared-near-range'
SEALED = ('finish_when_complete.py', 'finish_docs.py', 'support.py', 'report.py',
          'verify_preservation.py', 'run_all.py', 'protocol.json', 'selection.json',
          'baseline.json', 'candidate.json', 'native_wrapper.py')


def git(*args):
    return subprocess.check_output(['git', *args], cwd=REPO, text=True).strip()


def seals():
    values = {}
    for name in SEALED:
        path = E / name
        committed = subprocess.check_output(['git', 'show', 'HEAD:' + path.relative_to(REPO).as_posix()], cwd=REPO)
        if committed != path.read_bytes():
            raise RuntimeError('Uncommitted execution script/configuration: ' + name)
        values[name] = hashlib.sha256(committed).hexdigest()
    return values


def check_terminal_complete():
    active_path, exit_path = R / 'active.json', R / 'run_exit.json'
    if not active_path.exists():
        raise RuntimeError('Existing experiment controller state is missing.')
    active = json.loads(active_path.read_text())
    if active.get('status') == 'failed':
        raise RuntimeError('Experiment controller failed; preserve evidence and review: ' + str(active.get('error')))
    if active.get('status') != 'complete' or not exit_path.exists():
        return False
    ended = json.loads(exit_path.read_text())
    if ended.get('exit_code') != 0 or ended.get('completed') != PROTOCOL['order']:
        raise RuntimeError('Controller terminal receipt does not show full completion.')
    if active.get('completed') != PROTOCOL['order'] or active.get('source_commit') != SOURCE:
        raise RuntimeError('Controller terminal state does not match this protocol.')
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--wait', action='store_true', help='Wait for the existing controller; never starts one.')
    parser.add_argument('--push', action='store_true', help='Push the verified completion to the existing study branch.')
    parser.add_argument('--attempt', default='01')
    args = parser.parse_args()
    if not args.attempt.isdigit():
        parser.error('--attempt must contain only digits')
    lock_path = R / 'completion_worker.lock'
    try:
        lock = os.open(lock_path, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    except FileExistsError:
        raise SystemExit('Completion worker lock exists; inspect its recorded PID before continuing.')
    with os.fdopen(lock, 'w') as stream:
        stream.write(str(os.getpid()) + '\n')
    folder = E / 'completion' / args.attempt
    worker_launch_commit = git('rev-parse', 'HEAD')
    state = dict(pid=os.getpid(), source_commit=SOURCE, worker_launch_commit=worker_launch_commit,
                 status='starting', attempt=args.attempt)

    def update(**fields):
        state.update(fields)
        write(R / 'completion_state.json', state)

    try:
        expected = seals()
        update(status='waiting_for_controller', sealed_file_sha256=expected)
        while not check_terminal_complete():
            if not args.wait:
                raise RuntimeError('Study is incomplete; no finalization writes were made.')
            time.sleep(30)
        update(status='checking_completion')
        if git('branch', '--show-current') != BRANCH:
            raise RuntimeError('Working branch changed; do not finalize another branch.')
        if git('status', '--porcelain'):
            raise RuntimeError('Repository has uncommitted work; review it before finalization.')
        if seals() != expected:
            raise RuntimeError('A sealed script/configuration changed during the wait.')
        if folder.exists():
            raise RuntimeError('Completion evidence already exists; choose a fresh attempt after review.')
        verify_binaries()
        write(folder / 'launch.json', dict(command=[sys.executable, *sys.argv], cwd=str(REPO),
              source_commit=SOURCE, worker_launch_commit=worker_launch_commit,
              completed_evidence_commit=git('rev-parse', 'HEAD'), sealed_file_sha256=expected, exit_code=0,
              scope='Controller terminal completion validated; this is not a final-check success receipt.'))
        report = folder / 'report/summary.json'
        run('report', [sys.executable, str(E / 'report.py'), '--output-dir', str(report.parent)], folder / 'checks')
        # Commit the generated report before using any of its numbers in docs.
        subprocess.run(['git', 'add', '-f', str(folder)], cwd=REPO, check=True)
        subprocess.run(['git', 'commit', '-m', 'Record complete shared-range KITTI replay report from all committed checkpoints'], cwd=REPO, check=True)
        result_commit = git('rev-parse', 'HEAD')
        update(status='writing_documents', report_commit=result_commit)
        manifest = folder / 'document_manifest.json'
        run('documents', [sys.executable, str(E / 'finish_docs.py'), '--report', str(report),
            '--apply', '--manifest', str(manifest)], folder / 'checks')
        prefix = docker() + ['env', 'MPLCONFIGDIR=/tmp/localization-range-plots', CONTAINER_PY]
        commands = [
            ('refresh_study_docs', ['evaluation/scripts/refresh_study_docs.py']),
            ('generate_full_variant_table', ['evaluation/scripts/generate_full_variant_table.py']),
            ('generate_paper_ratio_table', ['evaluation/scripts/generate_paper_ratio_table.py']),
            ('python', ['-m', 'unittest', 'discover', '-s', 'tests']),
            ('showcase', ['evaluation/scripts/validate_showcase.py']),
            ('leaderboard', ['evaluation/scripts/generate_leaderboard.py', '--check']),
            ('paper_ready', ['evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
        ]
        update(status='final_checks')
        for name, argv in commands:
            run(name, prefix + argv, folder / 'checks')
        run('preservation', [sys.executable, str(E / 'verify_preservation.py'), '--output',
            str(folder / 'checks/preservation.json')], folder / 'checks')
        run('branch_whitespace', ['git', 'diff', '--check', PROTOCOL['base_commit']], folder / 'checks')
        paths = json.loads(manifest.read_text())['paths']
        paths += ['docs/decisions.md', 'docs/experiments.md', 'docs/interfaces.md', 'docs/paper_assets.md',
                  'docs/paper_captions.md', 'docs/paper_comparison.md', 'docs/paper_roadmap.md',
                  'docs/paper_tracks.md', 'docs/reproduction_status.md', 'docs/variant_analysis.md',
                  'experiments/results/index.json']
        summary = json.loads(report.read_text())
        body = (f"## Summary\n\nShare the existing optional sensor-frame near-range filter without changing defaults. "
                f"All {summary['completed_cell_count']} selected full KITTI replay cells are now complete. "
                f"{len(summary['historical_metric_drift_cells'])} differ from historical metrics; all differing "
                f"cells match frozen pre-extraction controls and remain documented. Original nonzero historical "
                f"audit exits are retained. Fresh NCLT validation remains open.\n\n"
                f"## Type\n\n- [ ] New paper reimplementation\n- [ ] Bug fix\n- [x] Benchmark / evaluation\n- [x] Docs / tooling\n\n"
                f"## Checklist\n\n- [x] Builds clean and ctest passes (frozen native source, preflight evidence)\n"
                f"- [ ] (New paper) ≥3 mechanism unit tests: N/A, no new paper\n"
                f"- [ ] (New paper) KITTI full seq00 / seq07: N/A, historical replay audit\n"
                f"- [ ] (New paper) Results row + docs/methods.json: N/A, no promotion\n"
                f"- [ ] (New paper) Module README: N/A, updated existing method READMEs\n"
                f"- [x] Results reported honestly, including historical differences and nonzero audit exits\n\n"
                f"## Results / notes\n\nNative source `{SOURCE}`; full committed report at "
                f"`{result_commit}:{report.relative_to(REPO).as_posix()}`. "
                f"See the study and method READMEs for all numeric outcomes, exact commands, cleanup and limits. "
                f"Final required generators, Python/doc checks and preservation receipts are under "
                f"`{(folder/'checks').relative_to(REPO).as_posix()}`. "
                f"All stored aggregates and published paper values are preserved. Defaults remain off. "
                f"This body is prepared for the connected GitHub tool; confirm CI at the pushed head before "
                f"marking PR137 ready. No merge is performed.\n")
        (folder / 'PR_BODY.md').write_text(body)
        write(folder / 'completion.json', dict(source_commit=SOURCE, report_commit=result_commit,
              completed_cells=summary['completed_cell_count'], local_required_checks_exit_code=0,
              github_ci_status='not_checked_by_this_worker', pr_metadata_updated=False,
              merge_performed=False, exit_code=0))
        subprocess.run(['git', 'add', '--', *paths], cwd=REPO, check=True)
        subprocess.run(['git', 'add', '-f', str(folder)], cwd=REPO, check=True)
        run('staged_branch_whitespace', ['git', 'diff', '--cached', '--check', PROTOCOL['base_commit']], folder / 'checks')
        subprocess.run(['git', 'add', '-f', str(folder)], cwd=REPO, check=True)
        subprocess.run(['git', 'commit', '-m', 'Publish complete shared-range replay outcomes and final required checks'], cwd=REPO, check=True)
        head = git('rev-parse', 'HEAD')
        update(status='local_completion_committed', head_sha=head)
        if args.push:
            run('push', ['git', 'push', 'origin', BRANCH], R / ('completion_push_' + args.attempt))
        update(status='complete_pending_connected_pr_update', head_sha=head, pushed=args.push,
               pr_body_file=str(folder / 'PR_BODY.md'), github_ci_checked=False, merge_performed=False)
        return 0
    except BaseException as error:
        update(status='failed', error=str(error))
        traceback.print_exc()
        return 1
    finally:
        lock_path.unlink()


if __name__ == '__main__':
    raise SystemExit(main())
