"""Execute the prospectively frozen finite passive study, retaining actual exits."""
import common
from common import E,S,W,PREFIX,run,write,module

if __name__=='__main__':
    stage=S/'execution';stage.mkdir()
    names=[line.split('def test_')[1].split('(')[0] for line in (W/'tests/test_inten_local_rigidity.py').read_text().splitlines() if 'def test_' in line]
    names=['test_inten_local_rigidity.LocalRigidityTest.test_'+n for n in names if not n.startswith('complete_derived_report')]
    command=PREFIX+['-m','unittest','-v',*names]
    # unittest imports modules from tests without changing the recorded cwd.
    command[1]='PYTHONPATH='+str(W/'tests')+':/workspace/localization-env/host-venv/lib/python3.12/site-packages'
    code=run('mechanisms',command,stage)
    if not code:code=run('analyze',PREFIX+[str(E/'analyze.py'),'--output-dir',str(S/'output')],stage)
    if not code:code=run('pack',PREFIX+[str(E/'pack.py'),'--source',str(S/'output'),'--execution',str(stage)],stage)
    write(stage/'result.json',dict(exit_code=code,scope='Finite passive geometric witness; no native solve or candidate.'))
    module('rigidity_execution_archive',common.P/'archive.py').archive(stage,E/'checks/initial')
    raise SystemExit(code)
