from common import *
def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH',
        'docker','--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(W),'localization-zoo-dev']
def main():
    assert not git('status','--porcelain')
    stage=R/'build_stage';stage.mkdir();sources={}
    for name,digest in read(E/'protocol.json')['source_sha256'].items():
        assert sha(W/name)==digest,name
        target=stage/'sources'/(name+'.gz');target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(gzip.compress((W/name).read_bytes(),mtime=0))
        sources[name]=dict(path=str(target.relative_to(stage)),sha256=digest)
    write(stage/'source_manifest.json',sources)
    old=E.parent/'inten_iteration_budget/checks/build'
    command=read(old/'native_compile_command.json')['command'];flags=command[command.index('c++')+1:command.index('-c')]+['-I'+str(E)]
    link=read(old/'probe_compile_command.json')['command'];libs=link[link.index('/workspace/localization-env/inten-iteration-budget-20261022/replay.o')+1:link.index('-o')]
    assert run('compiler',docker()+['c++','--version'],stage)==0
    assert run('probe_compile',docker()+['c++',*flags,str(E/'probe.cpp'),*libs,'-o',str(R/'probe')],stage)==0
    assert run('unit_compile',docker()+['c++',*flags,str(E/'unit.cpp'),*libs,'-lgtest_main','-lgtest','-o',str(R/'unit')],stage)==0
    assert run('unit',docker()+[str(R/'unit')],stage)==0
    assert run('ldd',docker()+['ldd',str(R/'probe')],stage)==0
    write(stage/'binaries.json',dict(binaries={p.name:dict(path=str(p),sha256=sha(p)) for p in (R/'probe',R/'unit')},
        linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')},source_sha256=read(E/'protocol.json')['source_sha256']))
    module('dense_archive',E/'archive.py').archive(stage,E/'checks/build')
if __name__=='__main__':main()
