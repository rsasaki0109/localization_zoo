"""Compile only the changed runner; link pinned, unchanged PR140 libraries."""
from pathlib import Path
import hashlib,json,shlex,subprocess,time
r=Path(__file__).resolve().parent;repo=r/'worktree';old=Path('/workspace/localization-env/inten-passive-trace/worktree')
out=r/'focused_build';out.mkdir(exist_ok=True)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
source='f8ee0b08bb160e6c0be91a240190862278c7ba51'
paths=['common',':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/CMakeLists.txt']
subprocess.run(['git','diff','--exit-code',source,'45721068c788392e5070145de346707451d2543c','--',*paths],cwd=repo,check=True)
subprocess.run(['git','diff','--exit-code',source,'--','common','papers','evaluation/src'],cwd=repo,check=True)
assert sha(old/'build/evaluation/pcd_dogfooding')=='e34c6294bdd41c5d1efe64b576283511cbe6d12dfb138ae62a5bf84d110b8459'
make_dir=old/'build/evaluation/CMakeFiles/pcd_dogfooding.dir'
flag_text=(make_dir/'flags.make').read_text();flags={}
for line in flag_text.splitlines():
 if ' = ' in line:
  k,v=line.split(' = ',1);flags[k]=shlex.split(v)
link=shlex.split((make_dir/'link.txt').read_text())
lib_cwd=old/'build/evaluation'
prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-w',str(lib_cwd),'localization-zoo-dev']
link_paths=[]
for item in link[1:]:
 p=Path(item)
 if p.suffix in ('.a','.so') or '.so.' in p.name:
  p=(lib_cwd/p).absolute() if not p.is_absolute() else p
  link_paths.append(str(p))
def link_hashes():
 code="import json,sys,pathlib,hashlib; print(json.dumps({p:hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest() for p in json.loads(sys.argv[1])}))"
 cmd=prefix+['python3','-c',code,json.dumps(link_paths)]
 result=subprocess.run(cmd,cwd=repo,text=True,capture_output=True,check=True)
 return json.loads(result.stdout)
artifacts=link_hashes()
(out/'pinned_link_inputs.json').write_text(json.dumps(dict(reference_native_source_commit='67cb05b24a7566c6cf00415784f36cb03593ea71',reference_head='45721068c788392e5070145de346707451d2543c',reference_binary_sha256=sha(old/'build/evaluation/pcd_dogfooding'),libraries=artifacts,flags_make_sha256=sha(make_dir/'flags.make'),link_txt_sha256=sha(make_dir/'link.txt')),indent=2)+'\n')
obj=out/'pcd_dogfooding.cpp.o';binary=out/'pcd_dogfooding'
compile_cmd=['/usr/bin/c++',*flags['CXX_DEFINES'],*flags['CXX_INCLUDES'],*flags['CXX_FLAGS'],'-c',str(repo/'evaluation/src/pcd_dogfooding.cpp'),'-o',str(obj)]
link[link.index('CMakeFiles/pcd_dogfooding.dir/src/pcd_dogfooding.cpp.o')]=str(obj)
link[link.index('-o')+1]=str(binary)
prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-w',str(lib_cwd),'localization-zoo-dev']
receipts=[]
for name,argv in [('compile',compile_cmd),('link',link)]:
 start=time.time();cmd=prefix+argv
 with (out/(name+'.log')).open('w') as f:result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 receipts.append(dict(name=name,command=cmd,cwd=str(repo),native_cwd=str(lib_cwd),native_source_commit=source,exit_code=result.returncode,started_unix=start,finished_unix=time.time()))
 (out/'commands.json').write_text(json.dumps(receipts,indent=2)+'\n')
 if result.returncode:raise SystemExit(result.returncode)
assert link_hashes()==artifacts, "Link inputs changed during the build"
(out/'summary.json').write_text(json.dumps(dict(exit_code=0,native_source_commit=source,binary=str(binary),binary_sha256=sha(binary),scope='Changed runner only; pinned unchanged PR140 static/system libraries. Separate fresh full CMake build remains in progress.'),indent=2)+'\n')
print('Focused runner build passed')
