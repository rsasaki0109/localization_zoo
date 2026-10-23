from pathlib import Path
import ast,importlib.util,json,os,shutil,subprocess
R=Path(__file__).resolve().parent;W=R/'worktree';E=W/'experiments/results/inten_facade_loss'
PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,PYTHONDONTWRITEBYTECODE='1',OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1',
    PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],MPLCONFIGDIR=str(R/'matplotlib'),XDG_CACHE_HOME=str(R/'cache'))
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def git(*args):return subprocess.check_output(['git',*args],cwd=W,text=True).strip()
def commit(message):
    subprocess.run(['git','add','-f',str(E),*map(str,sorted(W.glob('experiments/inten_loam_kitti_seq_*_full_facade_loss_matrix.json')))],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m',message],cwd=W,check=True)


def main():
    proof=json.loads((R/'large_storage.json').read_text())
    if not proof.get('complete') or proof.get('exit_code')!=0 or git('status','--porcelain'):raise RuntimeError('Completed clean storage proof required')
    shutil.copyfile(R/'large_storage.json',E/'checks/historical_large_storage/proof.json')
    archive=module('archive',E/'archive.py').archive
    archive(R/'large_storage_record',E/'checks/historical_large_storage/execution')
    for path in sorted((R/'full_prepared').rglob('*')):
        if path.is_file():
            target=W/path.relative_to(R/'full_prepared');target.parent.mkdir(parents=True,exist_ok=True)
            if target.exists():raise RuntimeError('Prepared source target exists: '+str(target))
            shutil.move(path,target)
    for name in ('collect.py','run_collection.py','capture_runtime.py'):ast.parse((E/name).read_text(),filename=name)
    producer=json.loads((E/'full_protocol.json').read_text())['native_source_commit']
    if git('diff','--name-only',producer,'--','common','evaluation','python',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**','CMakeLists.txt'):raise RuntimeError('Native producer drift')
    commit('Freeze one full00-10 facade Cauchy setting and preserve historical storage proof')
    run=module('commands',E/'run.py')
    if run.run('runtime',run.build.docker()+['python3',str(E/'capture_runtime.py')],R/'runtime_record'):raise RuntimeError('Native runtime capture failed')
    (E/'runtime.json').write_bytes((R/'runtime_record/runtime.stdout.log').read_bytes())
    archive(R/'runtime_record',E/'checks/runtime')
    commit('Pin the actual native runtime before all full facade-loss comparisons')
    if git('status','--porcelain'):raise RuntimeError('Full source must be clean')
    result=subprocess.run([PY,str(E/'run_collection.py')],cwd=W,env=ENV)
    if result.returncode:raise SystemExit(result.returncode)


if __name__=='__main__':main()
