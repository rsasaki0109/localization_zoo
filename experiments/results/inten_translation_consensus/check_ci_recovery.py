"""Verify the bounded temporary-Git correction without rerunning scientific data."""
import subprocess
from complete import execute
from common import E,W,R,PY,read,write,module


def main():
    stage=R/'ci_recovery_checks'
    checks=[('saved_path_fixture',[PY,'-m','unittest','discover','-s','tests','-p','test_shared_range_saved02_gate.py']),
            ('python',[PY,'-m','unittest','discover','-s','tests']),
            ('showcase',[PY,'evaluation/scripts/validate_showcase.py']),
            ('leaderboard',[PY,'evaluation/scripts/generate_leaderboard.py','--check']),
            ('paper_ready',[PY,'evaluation/scripts/generate_paper_ready_evidence.py','--check']),
            ('preservation',[PY,str(E/'verify.py'),'--output',str(stage/'preservation.json')]),
            ('whitespace',['git','diff','ca767ad6050b2e2ef95e07d61ad98fd1ef8f0636','--check'])]
    codes={name:execute(name,command,stage) for name,command in checks}
    write(stage/'result.json',dict(exit_code=int(any(codes.values())),codes=codes,
          original_ci_failure_retained=True,scientific_data_and_gate_unchanged=True))
    module('ci_recovery_archive',E/'archive.py').archive(stage,E/'checks/ci_recovery')
    assert not any(codes.values()),'Recovery checks failed; all actual exits retained'
    subprocess.run(['git','add','-f',str(E/'checks/ci_recovery')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Verify temporary-Git fixture stabilization and unchanged consensus evidence'],cwd=W,check=True)


if __name__=='__main__':main()
