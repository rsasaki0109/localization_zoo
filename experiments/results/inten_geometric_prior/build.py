#!/usr/bin/env python3
"""Build actual production CMake targets in existing container scratch space."""
import io,json,subprocess,tarfile
from pathlib import Path
from receipt import save,sha
root=Path.cwd(); study=root/'experiments/results/inten_geometric_prior'
container='localization-zoo-dev'; target='/dev/inten-geometric-prior-20261023'
commands=[]
def run(argv,stdin=None,binary=False):
    r=subprocess.run(argv,input=stdin,capture_output=True)
    commands.append({'argv':argv,'exit_code':r.returncode,'stdout':None if binary else r.stdout.decode(errors='replace'),
        'stderr':r.stderr.decode(errors='replace'),'stdin_sha256':__import__('hashlib').sha256(stdin).hexdigest() if stdin else None})
    save(study/'build_commands.json',commands)
    if r.returncode: raise RuntimeError('build command failed')
    return r.stdout
run(['docker','exec',container,'mkdir',target])
archive=io.BytesIO()
with tarfile.open(fileobj=archive,mode='w') as tar:
    for folder in ['papers/inten_loam','papers/kiss_icp','common/include']:
        tar.add(root/folder,arcname=folder)
    for file in ['evaluation/src/inten_trace_jsonl.h','experiments/results/inten_geometric_prior/CMakeLists.txt',
                 'experiments/results/inten_geometric_prior/stream_runner.cpp','experiments/results/inten_geometric_prior/preprocess.cpp']:
        tar.add(root/file,arcname=file)
run(['docker','exec','-i',container,'tar','xf','-','-C',target],archive.getvalue())
base=['docker','exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1',container]
run(base+['cmake','-S',target+'/experiments/results/inten_geometric_prior','-B',target+'/build',
          '-DCMAKE_BUILD_TYPE=Release','-DSOURCE_ROOT='+target,
          '-DFROZEN_COMMON_LIBRARY=/workspace/localization-env/inten-natural-traces/build/common/liblocalization_common.a'])
run(base+['cmake','--build',target+'/build','-j1'])
run(base+['ctest','--test-dir',target+'/build','--output-on-failure'])
for name in ['stream_runner','inten_loam/test_inten_loam_paper']:
    data=run(base+['cat',target+'/build/'+name],binary=True)
    # Binary stdout must not be serialized into JSON. Remove its text receipt.
    commands[-1].pop('stdout');commands[-1]['binary_sha256']=__import__('hashlib').sha256(data).hexdigest()
    save(study/'build_commands.json',commands)
    path=root.parent/Path(name).name;path.write_bytes(data);path.chmod(0o755)
save(study/'binaries.json',{'runner_container_path':target+'/build/stream_runner',
    'runner_sha256':sha(root.parent/'stream_runner'),
    'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
    'frozen_common_sha256':sha('/workspace/localization-env/inten-natural-traces/build/common/liblocalization_common.a')})
