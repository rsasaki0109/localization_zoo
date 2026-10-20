"""Actual-exit checks of byte-pinned existing native tests, then runtime capture."""
from pathlib import Path
import importlib.util,json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
s=importlib.util.spec_from_file_location('build',E/'build.py');b=importlib.util.module_from_spec(s);s.loader.exec_module(b)

def main():
    pins=json.loads((E/'producer_provenance.json').read_text());folder=E/'checks/frozen_native_tests'
    for label,item in pins['test_binaries'].items():
        if b.sha(item['path'])!=item['sha256']:raise RuntimeError('Frozen test binary changed')
        b.run(label,b.docker(R)+[item['path']],folder)
    b.run('runtime',b.docker()+['python3',str(E/'capture_runtime.py')],folder)
    runtime=json.loads((folder/'runtime.stdout').read_text());(E/'runtime.json').write_text(json.dumps(runtime,indent=2)+'\n')
    (folder/'result.json').write_text(json.dumps(dict(exit_code=0,test_source_commit=pins['native_source_commit'],test_binaries=pins['test_binaries'],policy='Direct unchanged native test executables run in the new study directory, preserving all historical build metadata. Final full project CTest and ROS2 CI still required.'),indent=2)+'\n')
    subprocess.run(['git','add','-f',str(folder),str(E/'runtime.json')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Record pinned native mechanism tests and actual runtime before full comparison'],cwd=W,check=True)
if __name__=='__main__':main()
