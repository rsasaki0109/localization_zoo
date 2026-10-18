"""Finish this isolated draft after its already-running native check closes."""
from pathlib import Path
import gzip,hashlib,json,os,re,shutil,subprocess,time,traceback
R=Path(__file__).resolve().parent;REPO=R/'worktree';E=REPO/'experiments/results/inten_trace_export'
LAUNCH_HEAD='79734a800fc4dc3fafc93e0c6b4ca1fe24c1e9e3'
SOURCE='f8ee0b08bb160e6c0be91a240190862278c7ba51'
STATE=R/'finish_state.json'

def state(status,**details):
    data=dict(status=status,pid=os.getpid(),launch_head=LAUNCH_HEAD,native_source_commit=SOURCE,updated_unix=time.time(),**details)
    STATE.write_text(json.dumps(data,indent=2)+'\n')

def run(cmd,**kwargs):return subprocess.run(cmd,cwd=REPO,check=True,**kwargs)

def head():return subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()

try:
    state('waiting_for_native_checks')
    while True:
        p=R/'native_checks/exit.json'
        try:closed=json.loads(p.read_text()) if p.exists() else None
        except json.JSONDecodeError:closed=None
        if closed is not None:break
        time.sleep(10)
    if closed['exit_code']!=0:
        state('stopped_native_failure',native_result=closed)
        raise RuntimeError('Native check failed; preserve logs, no automatic rerun or numeric success prose')
    assert head()==LAUNCH_HEAD,'Independent branch advanced; stop for review rather than overwrite'
    assert subprocess.check_output(['git','branch','--show-current'],cwd=REPO,text=True).strip()=='codex/inten-trace-export'
    assert not subprocess.check_output(['git','status','--porcelain'],cwd=REPO,text=True).strip()
    native=json.loads((R/'native_checks/commands.json').read_text())
    assert [x['name'] for x in native]==['source_guard','configure','build','ctest']
    assert all(x['exit_code']==0 and x['source_commit']==SOURCE for x in native)
    testlog=(R/'native_checks/ctest.log').read_text()
    count=re.search(r'100% tests passed, 0 tests failed out of (\d+)',testlog)
    assert count,'Actual CTest exit0 must also contain its whole-suite summary'
    count=int(count.group(1))
    state('importing_closed_native_evidence',ctest_entries=count)
    run(['/workspace/localization-env/host-venv/bin/python',str(E/'verify_preservation.py'),'--output',str(R/'full_native_preservation.json')])
    out=E/'evidence/full_native';assert not out.exists();out.mkdir()
    manifest={}
    for p in sorted((R/'native_checks').iterdir()):
        if not p.is_file():continue
        raw=p.read_bytes();stored=gzip.compress(raw,mtime=0) if p.suffix=='.log' else raw
        dest=out/(p.name+'.gz' if p.suffix=='.log' else p.name);dest.write_bytes(stored)
        manifest[dest.name]=dict(original_sha256=hashlib.sha256(raw).hexdigest(),stored_sha256=hashlib.sha256(stored).hexdigest(),lossless=True)
    last=REPO/'build/Testing/Temporary/LastTest.log'
    if last.exists():
        raw=last.read_bytes();(out/'LastTest.log.gz').write_bytes(gzip.compress(raw,mtime=0))
        manifest['LastTest.log.gz']=dict(original_sha256=hashlib.sha256(raw).hexdigest(),stored_sha256=hashlib.sha256((out/'LastTest.log.gz').read_bytes()).hexdigest(),lossless=True)
    for name in ['run_native_checks.py','finish_after_native.py','full_native_preservation.json']:
        shutil.copyfile(R/name,out/name)
    summary=dict(exit_code=0,native_source_commit=SOURCE,ctest_entries=count,commands='commands.json',logs=manifest,
                 scope='Fresh Release whole-zoo build and full CTest, with a fresh user-owned TEST_TMPDIR. No native source changed after the pinned implementation.')
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    run(['git','add','-f',str(out)]);run(['git','diff','--cached','--check'])
    run(['git','commit','-m','Retain closed whole-zoo build and CTest evidence before reporting'])
    evidence=head()
    state('documenting_committed_native_evidence',ctest_entries=count,evidence_commit=evidence)
    p=E/'README.md';s=p.read_text();s+=f'''\n## Fresh whole-zoo native validation\n\nThe separate fresh Release configure/build and all {count} CTest entries\nexit 0. The suite includes the new exporter cases and inherited estimator\nmechanism tests. A fresh user-owned `TEST_TMPDIR` is selected before the run.\n[Exact commands and exit codes](evidence/full_native/commands.json),\n[whole-suite evidence](evidence/full_native/summary.json), lossless logs and\nthe as-executed driver are retained. This evidence was committed at\n`{evidence}` before this prose; no fixture is rerun or reselected.\n''';p.write_text(s)
    p=REPO/'papers/inten_loam/README.md';s=p.read_text();needle='<!-- /INTEN_TRACE_EXPORT -->';assert s.count(needle)==1
    s=s.replace(needle,f'''The separate fresh Release full build and all {count} CTest entries also\nexit 0; [native evidence](../../experiments/results/inten_trace_export/evidence/full_native/summary.json)\nwas committed at `{evidence}` before this statement.\n{needle}''');p.write_text(s)
    code=(R/'check_docs.py').read_text().replace("raw = r / 'docs_checks'","raw = r / 'full_docs_checks'").replace("destination = e / 'checks/docs'","destination = e / 'checks/full_docs'")
    code=code.replace("for name in ['check_docs.py', 'document_results.py']:","for name in ['check_docs_full.py', 'document_results.py', 'finish_after_native.py']:")
    code=code.replace("'Document validated CLI trace export and preservation checks'","'Document whole-zoo native validation and refreshed preservation checks'")
    (R/'check_docs_full.py').write_text(code)
    state('checking_native_documentation',evidence_commit=evidence,ctest_entries=count)
    run(['/workspace/localization-env/host-venv/bin/python',str(R/'check_docs_full.py')])
    docs=head();run(['git','push','origin','codex/inten-trace-export'])
    data=json.loads((R/'task-state.json').read_text())
    data.update(status='FULL_NATIVE_EVIDENCE_PUSHED_PR141_DRAFT_CI_PENDING',head=docs,pushed_head=docs,
                full_native_evidence_commit=evidence,full_native_documentation_commit=docs,
                full_native_ctest_entries=count,full_native_checks_exit=0,
                next=['Inspect exact-head PR141 CI and reviews; update PR body once green. Keep unmerged until user asks.',
                      'Original177 replay and finalizer remain frozen. Inspect new completed sequence only; no duplicate workers.'])
    (R/'task-state.json').write_text(json.dumps(data,indent=2)+'\n')
    state('complete_pushed_ci_pending',evidence_commit=evidence,documentation_commit=docs,ctest_entries=count)
    print('Closed native evidence, refreshed checks and documentation pushed; PR remains draft/unmerged.',flush=True)
except BaseException as error:
    state('stopped_for_review',error=str(error),traceback=traceback.format_exc())
    traceback.print_exc();raise
