from pathlib import Path
import json,subprocess,time,shutil
r=Path('/workspace/localization-env/paper-bindings/python_moving');cmd=json.loads((r/'args.json').read_text());start=time.time()
with (r/'ctest.log').open('w') as f:p=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT)
(r/'command.json').write_text(json.dumps({'command':cmd,'started_unix':start,'finished_unix':time.time(),'exit_code':p.returncode},indent=2)+'\n')
if p.returncode==0:shutil.copy2('/workspace/localization_zoo/build/python/paper_python_fixture.json',r/'paper_python_fixture.json')
