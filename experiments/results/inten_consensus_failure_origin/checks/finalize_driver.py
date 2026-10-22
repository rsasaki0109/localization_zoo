"""Commit interpretation, retain real final-check exits, then push the clean head."""
from pathlib import Path
import importlib.util,json,subprocess
E=Path(__file__).resolve().parents[1];W=E.parents[2];R=W.parent
PY='/workspace/localization-env/host-venv/bin/python'
spec=importlib.util.spec_from_file_location('study_driver',E/'run_study.py')
a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
def main():
    own=Path(__file__).resolve()
    assert subprocess.check_output(['git','show','HEAD:'+str(own.relative_to(W))],cwd=W)==own.read_bytes()
    assert not a.git('status','--porcelain')
    assert subprocess.check_output(['git','show','HEAD:'+str((E/'report/summary.json').relative_to(W))],cwd=W)==(E/'report/summary.json').read_bytes()
    stage=R/'docs_stage'
    code=a.execute('write_docs',[PY,str(E/'write_docs.py')],stage)
    a.archive(stage,E/'docs_execution')
    if code:
        a.commit('Preserve failed consensus-origin documentation command')
        raise SystemExit(code)
    a.commit('Explain observed consensus seed damage and all adverse stage crossings',str(W/'papers/inten_loam/README.md'),str(W/'docs/roadmap.md'))
    stage=R/'checks_stage'
    checks=[('python',[PY,'-m','unittest','discover','-s','tests']),
            ('showcase',[PY,'evaluation/scripts/validate_showcase.py']),
            ('leaderboard',[PY,'evaluation/scripts/generate_leaderboard.py','--check']),
            ('paper_ready',[PY,'evaluation/scripts/generate_paper_ready_evidence.py','--check']),
            ('preservation',[PY,str(E/'verify.py'),'--output',str(E/'checks/preservation.json')]),
            ('whitespace',['git','diff','ed57c87ff2c128ac8d683d5e20938a3141cba7aa','--check'])]
    result={label:a.execute(label,command,stage) for label,command in checks}
    (stage/'result.json').write_text(json.dumps(dict(exit_code=int(any(result.values())),actual_command_exits=result),indent=2)+'\n')
    a.archive(stage,E/'checks/final')
    a.commit('Verify complete consensus failure-origin diagnosis and preserve historical results')
    if any(result.values()):raise RuntimeError('Final checks failed; all actual receipts retained before correction')
    subprocess.run(['git','push'],cwd=W,check=True)
    print('Complete archived diagnosis, interpretation and final exits committed and pushed',flush=True)
if __name__=='__main__':main()
