from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import time
import sys

original_native = sys.argv[1]  # Final original native10 cleanup commit

out = Path('/workspace/localization-env/shared-range-final-audit')
repo = out / 'worktree'
check = out / 'checks_final'
check.mkdir(exist_ok=False)
python = '/workspace/localization-env/host-venv/bin/python'
env = dict(os.environ, PATH=str(Path(python).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(out / 'check_matplotlib_final'))
paths = subprocess.check_output(['git', 'ls-files', 'docs', 'README.md', 'experiments/results/index.json'],
                                cwd=repo, text=True).splitlines()
before = {p: (repo / p).read_bytes() for p in paths if (repo / p).is_file()}
receipts = []


def run(label, args):
    started = time.time()
    with (check / (label + '.log')).open('x') as stream:
        result = subprocess.run(args, cwd=repo, env=env, stdout=stream, stderr=subprocess.STDOUT)
    receipts.append(dict(label=label, command=args, cwd=str(repo), source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
                         started_unix=started, finished_unix=time.time(), exit_code=result.returncode))
    (check / 'commands.json').write_text(json.dumps(receipts, indent=2) + '\n')
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)


for label, script in [('refresh', 'refresh_study_docs.py'), ('full_variant', 'generate_full_variant_table.py'),
                      ('paper_ratio', 'generate_paper_ratio_table.py')]:
    run(label, [python, 'evaluation/scripts/' + script])

dates = re.compile(rb'\d{4}-\d{2}-\d{2}(?:T| )\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})?')
disposition = []
for path, old in before.items():
    new = (repo / path).read_bytes()
    if new == old:
        continue
    only_timestamp = dates.sub(b'<timestamp>', old) == dates.sub(b'<timestamp>', new)
    disposition.append(dict(path=path, before_sha256=hashlib.sha256(old).hexdigest(),
                            regenerated_sha256=hashlib.sha256(new).hexdigest(),
                            timestamp_only=only_timestamp, retained_prior_bytes=only_timestamp))
    if only_timestamp:
        (repo / path).write_bytes(old)
(check / 'generation_disposition.json').write_text(json.dumps(disposition, indent=2) + '\n')

run('python', [python, '-m', 'unittest', 'discover', '-s', 'tests'])
run('showcase', [python, 'evaluation/scripts/validate_showcase.py'])
run('leaderboard', [python, 'evaluation/scripts/generate_leaderboard.py', '--check'])
run('paper_ready', [python, 'evaluation/scripts/generate_paper_ready_evidence.py', '--check'])
run('whitespace', ['git', 'diff', '--check', 'a4a0c28a1f1a326cccb9c938364d8ef575e281d2'])

base = 'a4a0c28a1f1a326cccb9c938364d8ef575e281d2'
study = repo / 'experiments/results/shared_near_range'
sys.path.insert(0, str(study))
import finish_docs
report = json.loads((study / 'completion/03/report/summary.json').read_text())
rows, controls = finish_docs.validate_complete(report)
historical = json.loads((study / 'historical_aggregate_sha256.json').read_text())
actual = {p.relative_to(repo).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in (repo / 'experiments/results').glob('*_matrix.json')}
if actual != historical:
    raise RuntimeError('Historical aggregate bytes changed')
published = {}
for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
             'docs/assets/paper/full_variant_results.csv', 'evaluation/data/paper_reported_numbers.json'):
    expected = subprocess.check_output(['git', 'show', base + ':' + path], cwd=repo)
    current = (repo / path).read_bytes()
    if expected != current:
        raise RuntimeError('Published bytes changed: ' + path)
    published[path] = dict(sha256=hashlib.sha256(current).hexdigest(), identical=True)

changed = subprocess.check_output(['git', 'diff', '--name-only', base], cwd=repo, text=True).splitlines()
allowed = {'docs/roadmap.md', 'tests/test_shared_range_saved02_gate.py',
           'experiments/results/shared_near_range/README.md',
           'experiments/results/shared_near_range/diagnose_saved02.py',
           'experiments/results/shared_near_range/final_integration_protocol.json'}
allowed.update('papers/' + module + '/README.md' for module in finish_docs.MODULES.values())
allowed.update('experiments/results/shared_near_range/diagnostics/' + seq + '_protocol.json'
               for seq in ('08', '09', '10'))
prefixes = tuple('experiments/results/shared_near_range/' + suffix for suffix in (
    'sequences/08/', 'sequences/09/', 'sequences/10/',
    'checkpoints/08/', 'checkpoints/09/', 'checkpoints/10/',
    'diagnostics/08/', 'diagnostics/09/', 'diagnostics/10/',
    'completion/03/', 'checks/final_integration/', 'checks/final_integration_startup/',
    'checks/saved08/', 'checks/saved09/', 'checks/saved10/',
    'checks/saved08_preparation/', 'checks/saved09_preparation/', 'checks/saved10_preparation/'))
if any(p not in allowed and not p.startswith(prefixes) for p in changed):
    raise RuntimeError('Unrelated changes: ' + repr(changed))

readmes = {}
for module in finish_docs.MODULES.values():
    path = 'papers/' + module + '/README.md'
    old = subprocess.check_output(['git', 'show', base + ':' + path], cwd=repo, text=True)
    current = (repo / path).read_text()
    for seq in ('08', '09', '10'):
        marker = 'SHARED_NEAR_RANGE_' + seq + '_DIAGNOSTICS'
        current = re.sub(r'\n<!-- ' + marker + r' -->.*?<!-- /' + marker + r' -->\n', '', current, flags=re.S)
    if finish_docs.strip_old_blocks(old) != finish_docs.strip_old_blocks(current):
        raise RuntimeError('Prior method README content changed: ' + path)
    readmes[path] = dict(prior_content_preserved=True)

def without_third_component(text):
    start = text.index('   - Third component implemented')
    end = re.search(r'^7\. ', text[start:], re.M)
    if end is None:
        raise RuntimeError('Roadmap next item missing')
    return text[:start] + text[start + end.start():]

old_roadmap = subprocess.check_output(['git', 'show', base + ':docs/roadmap.md'], cwd=repo, text=True)
if without_third_component(old_roadmap) != without_third_component((repo / 'docs/roadmap.md').read_text()):
    raise RuntimeError('Other roadmap sections changed')
intro = (study / 'README.md').read_text().split('## Complete KITTI replay', 1)[0]
for seq in ('00', '01', '02', '03', '04', '05', '06', '07', '08', '09', '10'):
    if '](' + 'diagnostics/' + seq + '/README.md)' not in intro:
        raise RuntimeError('Saved diagnostic link missing: ' + seq)
if '](runtime295_restore/README.md)' not in intro:
    raise RuntimeError('Runtime recovery link missing')

native_imports = {}
for seq in ('08', '09', '10'):
    identity = json.loads((study / ('checks/saved' + seq + '/import.json')).read_text())
    original = identity['original_native_commit']
    if seq == '10' and original != original_native:
        raise RuntimeError('Original10 argument differs from import identity')
    prefix = 'experiments/results/shared_near_range/sequences/' + seq + '/'
    paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', original, '--', prefix], cwd=repo, text=True).splitlines()
    if not paths or set(paths) != set(identity['native_files_sha256']):
        raise RuntimeError('Native import file set differs: ' + seq)
    for path in paths:
        raw = subprocess.check_output(['git', 'show', original + ':' + path], cwd=repo)
        if raw != (repo / path).read_bytes() or hashlib.sha256(raw).hexdigest() != identity['native_files_sha256'][path]:
            raise RuntimeError('Native import bytes differ: ' + path)
    native_imports[seq] = dict(original=original, file_count=len(paths), identical=True)

completion = json.loads((study / 'checks/final_integration/native_completion/import.json').read_text())
original = completion['original_completion_commit']
prefix = 'experiments/results/shared_near_range/completion/03/'
paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', original, '--', prefix], cwd=repo, text=True).splitlines()
if not paths or set(paths) != set(completion['imported_files_sha256']):
    raise RuntimeError('Completion import file set differs')
for path in paths:
    raw = subprocess.check_output(['git', 'show', original + ':' + path], cwd=repo)
    if raw != (repo / path).read_bytes() or hashlib.sha256(raw).hexdigest() != completion['imported_files_sha256'][path]:
        raise RuntimeError('Completion import bytes differ: ' + path)

terminal = json.loads((study / 'checks/final_integration/native_completion/run_exit.json').read_text())
protocol = json.loads((study / 'protocol.json').read_text())
if terminal != dict(exit_code=0, completed=protocol['order']):
    raise RuntimeError('Controller terminal receipt differs')
for seq in protocol['order']:
    cleanup = json.loads((study / ('sequences/' + seq + '/cleanup.json')).read_text())
    alias = Path('/workspace/localization-env/shared-near-range/data/dogfooding_results/kitti_seq_' + seq + '_full')
    if (not cleanup['hashes_verified_before_removal'] or cleanup['input_dir_exists_after']
            or Path(cleanup['owned_input_dir']).exists() or alias.exists() or alias.is_symlink()):
        raise RuntimeError('Owned input cleanup differs: ' + seq)

primary = Path('/workspace/localization_zoo')
primary_changed = subprocess.check_output(['git', 'diff', '--name-only',
    '0b079dbbcb7c6f58c1d8367ade881c15f003e525', '--', 'common', 'papers', 'evaluation', 'CMakeLists.txt'], cwd=primary, text=True).splitlines()
if any(not path.endswith('/README.md') for path in primary_changed):
    raise RuntimeError('Frozen primary source changed')
sealed = {}
for name in ('finish_when_complete.py','finish_docs.py','support.py','report.py','verify_preservation.py','run_all.py','protocol.json','selection.json','baseline.json','candidate.json','native_wrapper.py'):
    path = 'experiments/results/shared_near_range/' + name
    expected = subprocess.check_output(['git', 'show', 'c3698e6700199b352d846f70a6580ad007ea35cc:' + path], cwd=primary)
    if expected != (primary / path).read_bytes():
        raise RuntimeError('Primary sealed file changed: ' + name)
    sealed[name] = hashlib.sha256(expected).hexdigest()
frozen = {}
for name in ('baseline', 'candidate'):
    pin = json.loads((study / (name + '.json')).read_text())
    h = hashlib.sha256()
    with Path(pin['binary']).open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    if h.hexdigest() != pin['sha256']:
        raise RuntimeError('Frozen binary changed: ' + name)
    frozen[name] = dict(source=pin['source_commit'], sha256=h.hexdigest())

(check / 'preservation.json').write_text(json.dumps(dict(exit_code=0,
    completed_cells=len(rows), historical_matches=len(rows)-len(report['historical_metric_drift_cells']),
    historical_differences=len(report['historical_metric_drift_cells']), frozen_controls=len(controls),
    historical_aggregate_count=len(historical), historical_aggregate_hashes_match=True,
    publication=published, previous_method_readmes=readmes, other_roadmap_sections_preserved=True,
    all_saved_diagnostic_and_recovery_links_preserved=True, native_imports=native_imports,
    original_completion_import=dict(commit=original, file_count=len(paths), identical=True),
    controller_terminal_and_cleanup_verified=True, native_and_evaluation_and_prior_study_bytes_preserved=True,
    primary_native_source_preserved=True, primary_sealed_file_sha256=sealed,
    frozen_binaries=frozen), indent=2) + '\n')
(check / 'driver_source.py').write_bytes(Path(__file__).read_bytes())
print('preservation', 0, flush=True)

