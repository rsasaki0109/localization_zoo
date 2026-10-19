from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import time

out = Path('/workspace/localization-env/shared-range-checkpoint07')
repo = out / 'worktree'
check = out / 'checks'
check.mkdir(exist_ok=False)
python = '/workspace/localization-env/host-venv/bin/python'
env = dict(os.environ, PATH=str(Path(python).parent) + os.pathsep + os.environ['PATH'],
           PYTHONDONTWRITEBYTECODE='1', OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1',
           MPLCONFIGDIR=str(out / 'check_matplotlib'))
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
run('whitespace', ['git', 'diff', '--check', 'c3ebd8284f559ace2f7cfbdc45e0182c2ee1d1ee'])

historical = json.loads((repo / 'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
actual = {p.relative_to(repo).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
          for p in (repo / 'experiments/results').glob('*_matrix.json')}
if actual != historical:
    raise RuntimeError('Historical aggregate bytes changed')
published = {}
for path in ('README.md', 'docs/index.html', 'docs/assets/paper/paper_ratio_table.csv',
             'docs/assets/paper/full_variant_results.csv', 'evaluation/data/paper_reported_numbers.json'):
    base = subprocess.check_output(['git', 'show', 'c3ebd8284f559ace2f7cfbdc45e0182c2ee1d1ee:' + path], cwd=repo)
    current = (repo / path).read_bytes()
    if base != current:
        raise RuntimeError('Published bytes changed: ' + path)
    published[path] = dict(sha256=hashlib.sha256(current).hexdigest(), identical=True)
base = 'c3ebd8284f559ace2f7cfbdc45e0182c2ee1d1ee'
changed = subprocess.check_output(['git', 'diff', '--name-only', base], cwd=repo, text=True).splitlines()
methods = ('aloam','ct_icp','floam','lego_loam','mesh_loam','vlom')
allowed = {'docs/roadmap.md','tests/test_shared_range_saved02_gate.py',
 'experiments/results/shared_near_range/README.md',
 'experiments/results/shared_near_range/diagnose_saved02.py',
 'experiments/results/shared_near_range/diagnostics/07_protocol.json'}
allowed.update('papers/'+m+'/README.md' for m in methods)
prefixes = tuple('experiments/results/shared_near_range/'+p for p in
 ('sequences/07/','checkpoints/07/','diagnostics/07/','checks/saved07/'))
if any(p not in allowed and not p.startswith(prefixes) for p in changed):
    raise RuntimeError('Unrelated file changes: '+repr(changed))
for m in methods:
 method='papers/'+m+'/README.md'
 old=subprocess.check_output(['git','show',base+':'+method],cwd=repo)
 current=(repo/method).read_bytes()
 stripped=re.sub(rb'\n<!-- SHARED_NEAR_RANGE_07_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_07_DIAGNOSTICS -->\n',b'',current,flags=re.S)
 if stripped!=old: raise RuntimeError('Old method README bytes changed: '+method)
raw_prefix='experiments/results/shared_near_range/sequences/07/'
raw_paths=subprocess.check_output(['git','ls-tree','-r','--name-only','ed37e2fdfdfb6a5bd7ce70f1d30323bb0619bfaa','--',raw_prefix],cwd=repo,text=True).splitlines()
for path in raw_paths:
 old=subprocess.check_output(['git','show','ed37e2fdfdfb6a5bd7ce70f1d30323bb0619bfaa:'+path],cwd=repo)
 if old!=(repo/path).read_bytes():raise RuntimeError('Imported raw07 bytes changed: '+path)
primary=Path('/workspace/localization_zoo')
primary_changed=subprocess.check_output(['git','diff','--name-only','0b079dbbcb7c6f58c1d8367ade881c15f003e525','--','common','papers','evaluation','CMakeLists.txt'],cwd=primary,text=True).splitlines()
if any(not p.endswith('/README.md') for p in primary_changed):raise RuntimeError('Frozen primary native/evaluation source changed')
sealed={}
for name in ('finish_when_complete.py','finish_docs.py','support.py','report.py','verify_preservation.py','run_all.py','protocol.json','selection.json','baseline.json','candidate.json','native_wrapper.py'):
 path='experiments/results/shared_near_range/'+name
 expected=subprocess.check_output(['git','show','c3698e6700199b352d846f70a6580ad007ea35cc:'+path],cwd=primary)
 if expected!=(primary/path).read_bytes():raise RuntimeError('Primary sealed file changed: '+name)
 sealed[name]=hashlib.sha256(expected).hexdigest()
frozen={}
for name in ('baseline','candidate'):
 pin=json.loads((repo/('experiments/results/shared_near_range/'+name+'.json')).read_text())
 h=hashlib.sha256()
 with Path(pin['binary']).open('rb') as stream:
  for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
 if h.hexdigest()!=pin['sha256']:raise RuntimeError('Frozen binary changed: '+name)
 frozen[name]=dict(source=pin['source_commit'],sha256=h.hexdigest())
(check/'preservation.json').write_text(json.dumps(dict(exit_code=0,
 historical_aggregate_count=len(historical),historical_aggregate_hashes_match=True,
 publication=published,raw07_imported_file_count=len(raw_paths),raw07_bytes_match_original_commit=True,
 previous_method_readme_bytes_preserved=True,native_and_evaluation_and_prior_study_bytes_preserved=True,
 primary_native_source_preserved=True,primary_sealed_file_sha256=sealed,frozen_binaries=frozen),indent=2)+'\n')
(check/'driver_source.py').write_bytes(Path(__file__).read_bytes())
print('preservation',0,flush=True)
