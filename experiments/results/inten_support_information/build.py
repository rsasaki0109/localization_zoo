from common import *
def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH',
      'docker','--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OPENBLAS_NUM_THREADS=1','-w',str(W),'localization-zoo-dev']
from transform import transformed
def main():
    assert not git('status','--porcelain')
    R.mkdir(exist_ok=True);S.mkdir(exist_ok=True)
    assert not any(R.iterdir()),'Refuse stale native outputs'
    stage=S/'build_stage';stage.mkdir()
    protocol=read(E/'protocol.json');sources={}
    for name,digest in protocol['source_sha256'].items():
        assert sha(W/name)==digest,name
        path=stage/'sources'/(name+'.gz');path.parent.mkdir(parents=True,exist_ok=True)
        path.write_bytes(gzip.compress((W/name).read_bytes(),mtime=0));sources[name]=dict(path=str(path.relative_to(stage)),sha256=digest)
    write(stage/'source_manifest.json',sources)
    old=E.parent/'inten_iteration_budget/checks/build'
    cmd=read(old/'native_compile_command.json')['command'];flags=['-I'+str(E)]+cmd[cmd.index('c++')+1:cmd.index('-c')]
    link=read(old/'probe_compile_command.json')['command'];libs=link[link.index('/workspace/localization-env/inten-iteration-budget-20261022/replay.o')+1:link.index('-o')]
    assert not any(x.endswith('/libinten_loam.a') for x in libs)
    replay=R/'replay_native.cpp';replay.write_text(transformed((W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text()))
    (stage/'replay_native.cpp').write_bytes(replay.read_bytes())
    for label,command in (
      ('compiler',docker()+['c++','--version']),
      ('replay_compile',docker()+['c++',*flags,'-c',str(replay),'-o',str(R/'replay.o')]),
      ('probe_compile',docker()+['c++',*flags,str(E/'probe.cpp'),str(R/'replay.o'),*libs,'-o',str(R/'probe')]),
      ('unit_compile',docker()+['c++',*flags,str(E/'unit.cpp'),*libs,'-lgtest_main','-lgtest','-o',str(R/'unit')]),
      ('unit',docker()+[str(R/'unit')]),('ldd',docker()+['ldd',str(R/'probe')])):
        assert run(label,command,stage)==0
    write(stage/'binaries.json',dict(source_sha256=protocol['source_sha256'],replay_sha256=sha(replay),
        binaries={x:dict(path=str(R/x),sha256=sha(R/x)) for x in ('probe','unit','replay.o')},
        linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')}))
    module('reciprocal_archive',E/'archive.py').archive(stage,E/'checks/build')
if __name__=='__main__':main()
