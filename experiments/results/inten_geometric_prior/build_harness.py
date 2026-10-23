#!/usr/bin/env python3
"""Compile current CLI/serializer against unchanged pinned native dependencies."""
import json,re,shlex,subprocess
from pathlib import Path
from receipt import save,sha
root=Path.cwd();study=root/'experiments/results/inten_geometric_prior'
old=Path('/workspace/localization-env/inten-natural-traces/build')
source=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
container='localization-zoo-dev';scratch='/dev/inten-geometric-prior-20261023'
commands=[]
def run(argv,stdin=None):
    r=subprocess.run(argv,input=stdin,capture_output=True)
    commands.append({'argv':argv,'exit_code':r.returncode,'stdout':r.stdout.decode(errors='replace'),
                     'stderr':r.stderr.decode(errors='replace'),
                     'stdin_sha256':__import__('hashlib').sha256(stdin).hexdigest() if stdin else None})
    save(study/'harness_build_commands.json',commands)
    if r.returncode:raise RuntimeError('CLI build/check failed')
base=['docker','exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-e','TMPDIR='+scratch,container]
for rel in ['evaluation/src/pcd_dogfooding.cpp','evaluation/test/test_inten_trace_jsonl.cpp']:
    run(base+['mkdir','-p',str(Path(scratch)/Path(rel).parent)])
    run(['docker','exec','-i',container,'python3','-c',
         'import pathlib,sys; pathlib.Path(sys.argv[1]).write_bytes(sys.stdin.buffer.read())',scratch+'/'+rel],(root/rel).read_bytes())
flags=(old/'evaluation/CMakeFiles/pcd_dogfooding.dir/flags.make').read_text()
args=[]
for name in ['CXX_DEFINES','CXX_INCLUDES','CXX_FLAGS']:
    args+=shlex.split(re.search(r'^'+name+r' = (.*)$',flags,re.M).group(1))
args=[s.replace('/workspace/localization-env/inten-natural-traces/worktree',str(source)) for s in args]
args=['-I'+scratch+'/papers/inten_loam/include','-I'+scratch+'/papers/kiss_icp/include',
      '-I'+str(source/'evaluation/src'),*args]
run(base+['c++',*args,'-c',scratch+'/evaluation/src/pcd_dogfooding.cpp','-o',scratch+'/pcd_dogfooding.o'])
link=shlex.split((old/'evaluation/CMakeFiles/pcd_dogfooding.dir/link.txt').read_text())
libs=[]
for token in link:
    if token.endswith('.a') or token.endswith('.so') or '.so.' in token or token.startswith('-l'):
        if token.endswith('papers/inten_loam/libinten_loam.a'):
            libs += [scratch+'/build/inten_loam/libinten_loam.a',scratch+'/build/kiss_icp/libkiss_icp.a']
        elif token.startswith('../'):
            libs.append(str((old/'evaluation'/token).resolve()))
        else: libs.append(token)
run(base+['c++','-O3','-fopenmp',scratch+'/pcd_dogfooding.o','-o',scratch+'/pcd_dogfooding',*libs])
run(base+['c++',*args,scratch+'/evaluation/test/test_inten_trace_jsonl.cpp','-o',scratch+'/test_inten_trace_jsonl',
          scratch+'/build/inten_loam/libinten_loam.a',scratch+'/build/kiss_icp/libkiss_icp.a',
          '-lgtest_main','-lgtest','-lpthread',*libs])
run(base+[scratch+'/test_inten_trace_jsonl'])
run(base+['sha256sum',scratch+'/pcd_dogfooding'])
save(study/'harness_dependencies.json',{'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
     'flags_sha256':sha(old/'evaluation/CMakeFiles/pcd_dogfooding.dir/flags.make'),
     'link_sha256':sha(old/'evaluation/CMakeFiles/pcd_dogfooding.dir/link.txt'),
     'unchanged_static_libraries':{lib:sha(lib) for lib in libs if lib.endswith('.a') and Path(lib).is_file()}})
