"""Fresh diagnostic binaries; all inherited build files/libraries remain immutable."""
from common import *
import shlex

def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000','-e','OMP_NUM_THREADS=4','-e','OPENBLAS_NUM_THREADS=1','-w',str(W),'localization-zoo-dev']

def main():
    assert not git('status','--porcelain')
    for p,digest in read(E/'protocol.json')['source_sha256'].items(): assert sha(W/p)==digest,p
    stage=R/'build_stage';stage.mkdir()
    command=read(E.parent/'inten_consensus_query_acceleration/checks/cached_parity_balanced/compile_probe_command.json')['command']
    flags=[v.replace('/workspace/localization-env/inten-recursive-consensus-accelerated/worktree',str(W)).replace('inten_consensus_query_acceleration','inten_nonfacade_consensus_support') for v in command[command.index('c++')+1:command.index('/workspace/localization-env/inten-recursive-consensus-accelerated/worktree/experiments/results/inten_consensus_query_acceleration/parity_probe.cpp')]]
    libs=command[command.index('/workspace/localization-env/inten-recursive-consensus-accelerated/replay_balanced.o')+1:command.index('-o')]
    assert not any(v.endswith('/libinten_loam.a') for v in libs)
    assert run('compiler',docker()+['c++','--version'],stage)==0
    assert run('policy_compile',docker()+['c++',*flags,str(E/'policy_test.cpp'),'-o',str(R/'policy_test')],stage)==0
    assert run('policy_test',docker()+[str(R/'policy_test')],stage)==0
    assert run('native_compile',docker()+['c++',*flags,'-c',str(E/'diagnostic_native.cpp'),'-o',str(R/'replay.o')],stage)==0
    for name in ('probe',):
        assert run(name+'_compile',docker()+['c++',*flags,str(E/(name+'.cpp')),str(R/'replay.o'),*libs,'-o',str(R/name)],stage)==0
    assert run('ldd',docker()+['ldd',str(R/'probe')],stage)==0
    write(stage/'binaries.json',dict(source_commit=git('rev-parse','HEAD'),binaries={p.name:dict(path=str(p),sha256=sha(p)) for p in [R/'probe',R/'policy_test',R/'replay.o']},linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')},source_sha256=read(E/'protocol.json')['source_sha256']))
    module('archive_zero',E/'archive.py').archive(stage,E/'checks/build')
if __name__=='__main__':main()
