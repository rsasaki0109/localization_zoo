"""Import closed checks, retaining failures and source-scope corrections."""
from pathlib import Path
import gzip
import hashlib
import json
import re
import subprocess
import time

r = Path(__file__).resolve().parent
repo = r / 'worktree'
e = repo / 'experiments/results/inten_passive_trace'
out = e / 'evidence'
assert not out.exists()
source = '67cb05b24a7566c6cf00415784f36cb03593ea71'
base = '7505a2914328d53ac9b319d5f8042189b378d457'
head = subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip()
assert not subprocess.check_output(['git','status','--porcelain'],cwd=repo,text=True).strip()
assert json.loads((r/'native_checks/commands.json').read_text())[-1]['exit_code'] == 8
assert json.loads((r/'native_checks_retry/exit.json').read_text())['exit_code'] == 0
fixture = json.loads((r/'native_fixture/summary.json').read_text())
assert fixture['exit_code']==0 and fixture['native_exits']==[0,0,0,0]
for b in fixture['binaries'].values():
    assert hashlib.sha256(Path(b['path']).read_bytes()).hexdigest()==b['sha256']
checks = r/'final_source_checks'
checks.mkdir()
native = ['common','evaluation/src','evaluation/CMakeLists.txt',':(glob)papers/*/src/**',
          ':(glob)papers/*/include/**',':(glob)papers/*/test/**',
          ':(glob)papers/*/CMakeLists.txt','CMakeLists.txt']
commands = [
    ('native_source', ['git','diff','--exit-code',source,'--',*native],repo),
    ('reference_source', ['git','diff','--exit-code',base,
      '0b079dbbcb7c6f58c1d8367ade881c15f003e525','--',*native],repo),
    ('main_frozen_source', ['git','diff','--exit-code',
      '0b079dbbcb7c6f58c1d8367ade881c15f003e525','--',*native],Path('/workspace/localization_zoo')),
    ('preservation', ['/workspace/localization-env/host-venv/bin/python',
      str(e/'verify_preservation.py'),'--output',str(checks/'preservation.json')],repo),
]
receipts=[]
for name, command, cwd in commands:
    start=time.time()
    with (checks/(name+'.log')).open('w') as log:
        result=subprocess.run(command,cwd=cwd,stdout=log,stderr=subprocess.STDOUT)
    receipts.append(dict(name=name,command=command,cwd=str(cwd),exit_code=result.returncode,
        checked_head=head,compiled_source_commit=source,started_unix=start,finished_unix=time.time()))
    (checks/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
    assert result.returncode==0,(name,result.returncode)
log=(r/'native_checks_retry/LastTest.log').read_text()
new_tests=sorted(set(re.findall(r'\[       OK \] (InTenLoamPaper\.Passive\w+)',log)))
assert len(new_tests)==5
match=re.search(r'100% tests passed, 0 tests failed out of (\d+)',(r/'native_checks_retry/ctest.log').read_text())
assert match
out.mkdir()
imports=[]
def copy(path,target):
    target.parent.mkdir(parents=True,exist_ok=True)
    data=path.read_bytes()
    if path.suffix=='.log':
        target=target.with_name(target.name+'.gz')
        target.write_bytes(gzip.compress(data,mtime=0))
        assert gzip.decompress(target.read_bytes())==data
    else:
        target.write_bytes(data)
    imports.append(dict(source=str(path),destination=str(target.relative_to(repo)),
        original_sha256=hashlib.sha256(data).hexdigest(),stored_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
        lossless_gzip=path.suffix=='.log'))
for folder,label in [(r/'native_checks','initial_attempt'),(r/'native_checks_retry','native_checks'),
                     (r/'native_fixture','fixture'),(checks,'final_source_checks')]:
    for p in sorted(folder.rglob('*')):
        if p.is_file():
            # GoogleTest's generated temp file is not scientific evidence.
            if 'tmp' in p.relative_to(folder).parts: continue
            assert p.suffix not in ('.pcd','.bin','.a','.o','.so')
            copy(p,out/label/p.relative_to(folder))
for name in ['fixture_command.json','fixture.log','preservation_command.json','preservation.log','preservation.json']:
    copy(r/name,out/name)
for name in ['run_checks.py','import_native.py','retry_ctest.py','import_native_retry.py','complete_import.py']:
    copy(r/name,out/name)
for name in ['validate_fixture.py','verify_preservation.py']:
    path='experiments/results/inten_passive_trace/'+name
    raw=subprocess.check_output(['git','show',fixture['recorded_head']+':'+path],cwd=repo)
    (out/(name.removesuffix('.py')+'_as_executed.py')).write_bytes(raw)
(out/'import.json').write_text(json.dumps(dict(files=imports,exit_code=0),indent=2)+'\n')
(out/'summary.json').write_text(json.dumps(dict(exit_code=0,native_source_commit=source,
    validation_helper_commit=head,ctest_entries_passed=int(match.group(1)),
    new_passive_test_cases_passed=new_tests,initial_ctest_exit_code=8,
    initial_failure='Root-owned /tmp/posture_profile.yaml is unwritable for user1000; the unchanged existing test uses unchecked fopen then fputs.',
    correction='Only user-owned TEST_TMPDIR for targeted and full-suite rerun; binaries, tests and thresholds unchanged.',
    initial_preservation_exit_code=1,preservation_failure='Nonrecursive Git pathspec omitted source files, causing a false native-scope failure; explicit recursive globs correct it.',
    build_commands='initial_attempt/commands.json',test_commands='native_checks/commands.json',
    fixture_comparison='fixture/summary.json',preservation='final_source_checks/preservation.json',
    scope='Synthetic tracing-on parity and short-fixture tracing-disabled compatibility; no full-sequence trace or cause finding.'),indent=2)+'\n')
subprocess.run(['git','add','-f',str(out)],cwd=repo,check=True)
subprocess.run(['git','diff','--cached','--check'],cwd=repo,check=True)
subprocess.run(['git','commit','-m','Record passive trace checks, original failures and verified fixture parity'],cwd=repo,check=True)
print('Closed native results and both corrected-check histories committed before numeric prose.',flush=True)
