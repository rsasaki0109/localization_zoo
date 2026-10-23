"""Compile the untouched native translation unit and a passive public-API probe."""
from common import *

def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1','-w',str(W),'localization-zoo-dev']

def main():
    assert not git('status','--porcelain')
    for p,digest in read(E/'protocol.json')['source_sha256'].items(): assert sha(W/p)==digest,p
    stage=R/'build_stage';stage.mkdir()
    parent=E.parent/'inten_nonfacade_consensus_support/checks/build'
    command=read(parent/'native_compile_command.json')['command']
    flags=command[command.index('c++')+1:command.index('-c')]
    old='/workspace/localization-env/inten-nonfacade-consensus-support/worktree'
    flags=[v.replace(old,str(W)) for v in flags]
    link=read(parent/'probe_compile_command.json')['command']
    start=link.index('/workspace/localization-env/inten-nonfacade-consensus-support/replay.o')+1
    libs=link[start:link.index('-o')]
    assert not any(v.endswith('/libinten_loam.a') for v in libs)
    assert run('compiler',docker()+['c++','--version'],stage)==0
    assert run('native_compile',docker()+['c++',*flags,'-c',str(W/'papers/inten_loam/src/inten_loam_paper.cpp'),'-o',str(R/'native.o')],stage)==0
    assert run('probe_compile',docker()+['c++',*flags,str(E/'probe.cpp'),str(R/'native.o'),*libs,'-o',str(R/'probe')],stage)==0
    assert run('ldd',docker()+['ldd',str(R/'probe')],stage)==0
    write(stage/'binaries.json',dict(source_commit=git('rev-parse','HEAD'),binaries={p.name:dict(path=str(p),sha256=sha(p)) for p in [R/'probe',R/'native.o']},linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')},source_sha256=read(E/'protocol.json')['source_sha256']))
    module('continuity_archive',E/'archive.py').archive(stage,E/'checks/build')
if __name__=='__main__':main()
