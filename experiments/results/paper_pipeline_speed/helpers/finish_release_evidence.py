"""Archive verified release evidence, clean inputs, then run final doc checks."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import time
import traceback

r = Path('/workspace/localization-env/paper-speed')
repo = Path('/workspace/localization_zoo')
e = repo / 'experiments/results/paper_pipeline_speed'
python = '/workspace/localization-env/host-venv/bin/python'
out = r / 'finish_release_evidence'
out.mkdir(exist_ok=False)
steps = []

def checked(label, cmd):
    started = time.time()
    with (out / (label + '.log')).open('w') as log:
        result = subprocess.run(cmd, cwd=repo, stdout=log, stderr=subprocess.STDOUT)
    steps.append({'label': label, 'command': cmd, 'cwd': str(repo),
                  'started_unix': started, 'finished_unix': time.time(),
                  'exit_code': result.returncode})
    (out / 'commands.json').write_text(json.dumps(steps, indent=2) + '\n')
    if result.returncode:
        raise SystemExit(result.returncode)

def commit(message, paths):
    subprocess.run(['git', 'add', *[str(p.relative_to(repo)) for p in paths]], cwd=repo, check=True)
    if subprocess.run(['git', 'diff', '--cached', '--quiet'], cwd=repo).returncode:
        subprocess.run(['git', 'commit', '-m', message], cwd=repo, check=True)
    return subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()

def helper(name):
    source = r / name
    dest = e / 'helpers' / name
    if dest.exists() and dest.read_bytes() != source.read_bytes():
        dest = dest.with_name(source.stem + '_' + hashlib.sha256(source.read_bytes()).hexdigest()[:12] + source.suffix)
    shutil.copy2(source, dest)
    return dest

try:
    ready = r / 'release_verification_ready.json'
    while not ready.exists():
        time.sleep(5)
    assert json.loads(ready.read_text())['exit_code'] == 0, 'Inspect failed release checks before cleanup'
    assert not subprocess.check_output(['git', 'status', '--porcelain'], cwd=repo, text=True).strip(), 'Review uncommitted changes before archiving'
    release = json.loads((r / 'release_source_ready.json').read_text())
    source, tag = release['source_commit'], release['tag']
    verification = r / 'release_verification' / tag
    assert json.loads((verification / 'exit.json').read_text())['exit_code'] == 0
    checks = json.loads((verification / 'trajectory_checks.json').read_text())
    assert set(checks) == {'mesh_loam_accelerated', 'imls_slam_default', 'imls_slam_accelerated', 'inten_loam_default', 'inten_loam_accelerated'}
    assert all(v['saved_poses_equal'] and v['verification_only'] for v in checks.values())
    binary = json.loads((verification / 'binary.json').read_text())
    assert binary['source_commit'] == source
    assert binary['retained_library_sha256_before'] == binary['retained_library_sha256_after']
    assert binary['mesh_cpp_matches_baseline'] and binary['retained_cpp_matches_measured_candidate']
    assert all(v['exit_code'] == 0 for v in json.loads((verification / 'commands.json').read_text()))
    dest = e / 'release_verification' / tag
    shutil.copytree(verification, dest)
    shutil.copy2(r / 'release_source_ready.json', dest / 'source_ready.json')
    shutil.copy2(ready, dest / 'controller_exit.json')
    helpers = [helper(n) for n in ['verify_release_candidate.py', 'run_release_checks_after_measurement.py', 'release_final_validation.py', 'verify_and_cleanup_kitti07.py', 'finish_release_evidence.py']]
    raw_commit = commit('Record retained-code build, full CTest and real-scan trajectory verification', [dest, *helpers])

    readme = e / 'README.md'
    text = readme.read_text()
    marker = '## Profiles and development history'
    assert '## Retained-code verification and final checks' not in text
    block = f'''## Retained-code verification and final checks

The release verification was committed in `{raw_commit}` before this account.
Its [commands](release_verification/{tag}/commands.json),
[binary/library identities](release_verification/{tag}/binary.json) and
[saved-pose comparisons](release_verification/{tag}/trajectory_checks.json)
identify retained source `{source}` separately from the measured candidate.
The complete Release build and CTest pass with their actual exit codes.
Source checks confirm that Mesh matches the frozen baseline, and the retained
IMLS/InTEn implementation and test files match the measured candidate. Their
compiled static-library hashes are unchanged across the release rebuild.

The retained binary also replays the first development window for IMLS and
InTEn with the option off and on, and for Mesh with the global flag ignored.
All saved trajectories match their recorded references. These are verification
runs; their execution times are excluded from the speed ratios. Unit tests
also compare internal poses, feature order, queries and normal equations across
worker counts; serialization equality alone is not the only accuracy check.

Input cleanup and final Python/document checks retain commands and exit codes
in `cleanup_kitti07.json` and `checks/release_final_validation/`. Cleanup verifies
all scan hashes before deleting the raw sequence. The preservation check covers
historical aggregates, the paper reference data, Table 6 and published
leaderboards. Neither the speed experiment nor the retained opt-in flag
replaces historical paper-number results.

'''
    assert marker in text
    readme.write_text(text.replace(marker, block + marker, 1))
    docs_commit = commit('Explain retained-source verification separately from measured candidates', [readme])
    checked('verify_and_cleanup_kitti07', [python, str(r / 'verify_and_cleanup_kitti07.py')])
    cleanup = json.loads((r / 'cleanup_kitti07.json').read_text())
    assert cleanup['exit_code'] == 0 and cleanup['input_sha256_match']
    assert not cleanup['raw_root_exists_after'] and not cleanup['symlink_exists_after']
    shutil.copy2(r / 'cleanup_kitti07.json', e / 'cleanup_kitti07.json')
    cleanup_commit = commit('Verify KITTI 07 hashes and delete the raw input sequence', [e / 'cleanup_kitti07.json'])
    checked('release_final_validation', [python, str(r / 'release_final_validation.py')])
    validation = r / 'release_final_validation'
    assert json.loads((validation / 'exit.json').read_text())['exit_code'] == 0
    shutil.copytree(validation, e / 'checks/release_final_validation')
    shutil.copytree(out, e / 'checks/release_final_steps')
    paths = [repo / 'docs', repo / 'experiments/results/index.json', e / 'checks/release_final_validation', e / 'checks/release_final_steps']
    checks_commit = commit('Regenerate study documents and record final regression and preservation checks', paths)
    result = {'exit_code': 0, 'release_source_commit': source, 'measured_source_commit': 'f47555735c94d43d6f8e1554ee7bf50be77ac159', 'raw_verification_commit': raw_commit, 'verification_docs_commit': docs_commit, 'cleanup_commit': cleanup_commit, 'final_checks_commit': checks_commit, 'roadmap_3x_goal_met': False, 'next': 'Review diff, update PR132, push and verify exact HEAD CI; leave unmerged pending user review.'}
    (r / 'release_evidence_ready.json').write_text(json.dumps(result, indent=2) + '\n')
except BaseException as err:
    code = err.code if isinstance(err, SystemExit) and isinstance(err.code, int) else 1
    (r / 'release_evidence_ready.json').write_text(json.dumps({'exit_code': code, 'error': str(err), 'finished_unix': time.time()}, indent=2) + '\n')
    traceback.print_exc()
    raise
