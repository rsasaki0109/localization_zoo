"""Import the already-closed native checks; capture every subprocess output to files."""
from pathlib import Path
import gzip,hashlib,json,os,re,shutil,subprocess,time,traceback
R=Path(__file__).resolve().parent;REPO=R/'worktree';E=REPO/'experiments/results/inten_trace_export'
HEAD='79734a800fc4dc3fafc93e0c6b4ca1fe24c1e9e3';SOURCE='f8ee0b08bb160e6c0be91a240190862278c7ba51'
OUT=R/'native_recovery';OUT.mkdir()
receipts=[]
def state(status,**data):
    (R/'resume_state.json').write_text(json.dumps(dict(status=status,pid=os.getpid(),updated_unix=time.time(),**data),indent=2)+'\n')
def sha(data):return hashlib.sha256(data).hexdigest()
def current_head():return subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
def run(name,cmd,expected=0,closed_stdout=False):
    start=time.time()
    with (OUT/(name+'.log')).open('wb') as f:
        if closed_stdout:
            reader,writer=os.pipe();os.close(reader)
            try:result=subprocess.run(cmd,cwd=REPO,stdout=writer,stderr=f)
            finally:os.close(writer)
        else:result=subprocess.run(cmd,cwd=REPO,stdout=f,stderr=subprocess.STDOUT)
    receipts.append(dict(name=name,command=cmd,cwd=str(REPO),exit_code=result.returncode,expected_exit_code=expected,
                         stdout_mode='closed_pipe_for_reproduction' if closed_stdout else 'regular_file',started_unix=start,finished_unix=time.time()))
    (OUT/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
    assert result.returncode==expected,(name,result.returncode)
try:
    assert current_head()==HEAD
    assert subprocess.check_output(['git','branch','--show-current'],cwd=REPO,text=True).strip()=='codex/inten-trace-export'
    assert not subprocess.check_output(['git','status','--porcelain'],cwd=REPO,text=True).strip()
    closed=json.loads((R/'native_checks/exit.json').read_text());assert closed['exit_code']==0
    native=json.loads((R/'native_checks/commands.json').read_text())
    assert [x['name'] for x in native]==['source_guard','configure','build','ctest']
    assert all(x['exit_code']==0 and x['source_commit']==SOURCE for x in native)
    count=re.search(r'100% tests passed, 0 tests failed out of (\d+)',(R/'native_checks/ctest.log').read_text())
    assert count;count=int(count.group(1))
    # Preserve the stopped worker before running any corrected postprocessing.
    shutil.copyfile(R/'finish_state.json',OUT/'original_finish_state.json')
    shutil.copyfile(R/'finish_after_native.py',OUT/'finish_after_native_as_executed.py')
    shutil.copyfile(R/'full_native_preservation.json',OUT/'original_preservation.json')
    state('checking_closed_output_failure',ctest_entries=count)
    cmd=['/workspace/localization-env/host-venv/bin/python',str(E/'verify_preservation.py'),'--output',str(OUT/'closed_pipe_preservation.json')]
    run('closed_pipe_reproduction',cmd,expected=120,closed_stdout=True)
    cmd=['/workspace/localization-env/host-venv/bin/python',str(E/'verify_preservation.py'),'--output',str(OUT/'captured_preservation.json')]
    run('captured_preservation',cmd)
    a=json.loads((OUT/'closed_pipe_preservation.json').read_text());b=json.loads((OUT/'captured_preservation.json').read_text())
    assert a==b and b['exit_code']==0
    (OUT/'summary.json').write_text(json.dumps(dict(exit_code=0,original_subprocess_exit=120,
        original_report_exit=0,controlled_closed_stdout_exit=120,captured_stdout_exit=0,reports_identical=True,
        scope='Same preservation command reproduces Python shutdown BrokenPipeError/exit120 when stdout has no reader, despite its successful JSON report. A regular output file exits0. Original stderr is unavailable, so this establishes the mechanism but does not prove the original pipe state. The already-successful build/CTest are not rerun.'),indent=2)+'\n')
    state('importing_closed_evidence',ctest_entries=count)
    out=E/'evidence/full_native';assert not out.exists();out.mkdir()
    manifest={}
    def copy(source,dest):
        dest.parent.mkdir(parents=True,exist_ok=True);raw=source.read_bytes()
        if source.suffix=='.log':dest=dest.with_suffix(dest.suffix+'.gz');stored=gzip.compress(raw,mtime=0)
        else:stored=raw
        dest.write_bytes(stored)
        manifest[str(dest.relative_to(out))]=dict(original_sha256=sha(raw),stored_sha256=sha(stored),lossless=True)
    for p in sorted((R/'native_checks').iterdir()):
        if p.is_file():copy(p,out/p.name)
    copy(REPO/'build/Testing/Temporary/LastTest.log',out/'LastTest.log')
    for p in sorted(OUT.iterdir()):
        if p.is_file():copy(p,out/'recovery'/p.name)
    for name in ['run_native_checks.py','finish_closed_native.py']:
        copy(R/name,out/name)
    binary=REPO/'build/evaluation/pcd_dogfooding'
    summary=dict(exit_code=0,native_source_commit=SOURCE,ctest_entries=count,commands='commands.json',
        fresh_binary_sha256=sha(binary.read_bytes()),logs=manifest,
        scope='Fresh Release whole-zoo build and full CTest, with fresh user-owned TEST_TMPDIR. No native source changed after the pinned implementation. Recovery imports closed evidence without rerunning tests.')
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    for name,item in manifest.items():
        data=(out/name).read_bytes();assert sha(data)==item['stored_sha256']
        if name.endswith('.log.gz'):data=gzip.decompress(data)
        assert sha(data)==item['original_sha256']
    run('stage_native_evidence',['git','add','-f',str(out)])
    run('staged_native_whitespace',['git','diff','--cached','--check'])
    run('commit_native_evidence',['git','commit','-m','Retain successful whole-zoo checks and captured postprocessing recovery'])
    evidence=current_head()
    state('documenting_committed_native_evidence',evidence_commit=evidence,ctest_entries=count)
    p=E/'README.md';s=p.read_text();assert '## Fresh whole-zoo native validation' not in s
    s+=f'''\n## Fresh whole-zoo native validation\n\nThe separate fresh Release configure/build and all {count} CTest entries\nexit 0. A fresh user-owned `TEST_TMPDIR` is selected before the run.\n[Commands and actual exits](evidence/full_native/commands.json),\n[whole-suite evidence](evidence/full_native/summary.json), lossless logs and\nthe as-executed driver are retained. This evidence was committed at\n`{evidence}` before this prose. The closed build/CTest and fixture\nchecks were not rerun during import recovery.\n\nThe first postprocessor stops when its preservation subprocess exits 120\nafter writing a successful result JSON. The controlled same-command check\nreproduces exit 120 and `BrokenPipeError` with a closed stdout pipe, while\nregular-file capture exits 0 and produces an identical success report.\nOriginal stderr is unavailable, so the original pipe state is not proven.\n[Failure and recovery evidence](evidence/full_native/recovery/summary.json)\nretain this distinction; the recovery captures subprocess output to files.\n''';p.write_text(s)
    p=REPO/'papers/inten_loam/README.md';s=p.read_text();needle='<!-- /INTEN_TRACE_EXPORT -->';assert s.count(needle)==1
    s=s.replace(needle,f'''The separate fresh Release full build and all {count} CTest entries also\nexit 0. [Native evidence](../../experiments/results/inten_trace_export/evidence/full_native/summary.json)\nwas committed at `{evidence}` before this statement. The postprocessor's\ninitial exit 120, closed-stdout reproduction and regular-file recovery exit 0\nare retained separately; the successful native tests were not rerun.\n{needle}''');p.write_text(s)
    code=(R/'check_docs.py').read_text().replace("raw = r / 'docs_checks'","raw = r / 'full_docs_checks'").replace("destination = e / 'checks/docs'","destination = e / 'checks/full_docs'")
    code=code.replace("for name in ['check_docs.py', 'document_results.py']:","for name in ['check_docs_full.py', 'document_results.py', 'finish_closed_native.py']:")
    code=code.replace("'Document validated CLI trace export and preservation checks'","'Document closed native validation and recovered postprocessing checks'")
    (R/'check_docs_full.py').write_text(code)
    state('checking_native_documentation',evidence_commit=evidence,ctest_entries=count)
    run('full_documentation_checks',['/workspace/localization-env/host-venv/bin/python',str(R/'check_docs_full.py')])
    docs=current_head();run('push',['git','push','origin','codex/inten-trace-export'])
    task=json.loads((R/'task-state.json').read_text());task.update(status='FULL_NATIVE_EVIDENCE_PUSHED_PR141_CI_PENDING',head=docs,pushed_head=docs,
        full_native_evidence_commit=evidence,full_native_documentation_commit=docs,full_native_ctest_entries=count,
        full_native_checks_exit=0,original_postprocessor_failure_retained=True,completion_worker=dict(status='closed_recovered',state=str(R/'resume_state.json')))
    (R/'task-state.json').write_text(json.dumps(task,indent=2)+'\n')
    state('complete_pushed_ci_pending',evidence_commit=evidence,documentation_commit=docs,ctest_entries=count)
except BaseException as error:
    state('stopped_for_review',error=str(error),traceback=traceback.format_exc());traceback.print_exc();raise
