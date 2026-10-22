"""Build the unchanged paper solver, budget controls and actual production CLI."""
from common import *
import shlex

def docker():
    return ['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS',
            '-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker',
            '--host=unix:///var/run/docker.sock','exec','-i','--user','1000:1000',
            '-e','OMP_NUM_THREADS=4','-e','OPENBLAS_NUM_THREADS=1',
            '-w',str(W),'localization-zoo-dev']

def transformed(source):
    replacements=[
        ('const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);',
         'const PaperFrame frame = replay::frame ? *replay::frame : extractFeatures(deskewed.empty() ? points : deskewed, intensity);'),
        ('Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;',
         'Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;\n  if (replay::seed) rel = *replay::seed;')]
    for a,b in replacements:
        assert source.count(a)==1
        source=source.replace(a,b)
    return '#include "replay_hooks.h"\n'+source

def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    stage=R/'build_stage';stage.mkdir()
    sources={}
    for name,digest in read(E/'protocol.json')['source_sha256'].items():
        target=stage/'sources'/(name+'.gz');target.parent.mkdir(parents=True,exist_ok=True)
        target.write_bytes(gzip.compress((W/name).read_bytes(),mtime=0))
        sources[name]=dict(path=str(target.relative_to(stage)),sha256=digest)
    write(stage/'source_manifest.json',sources)
    prior=E.parent/'inten_intensity_continuity/checks/build'
    command=read(prior/'native_compile_command.json')['command']
    flags=[v.replace('/workspace/localization-env/inten-intensity-continuity/worktree',str(W))
           for v in command[command.index('c++')+1:command.index('-c')]]
    flags+=['-I'+str(E)]
    link=read(prior/'probe_compile_command.json')['command']
    libs=link[link.index('/workspace/localization-env/inten-intensity-continuity/native.o')+1:link.index('-o')]
    assert not any(v.endswith('/libinten_loam.a') for v in libs)
    assert run('baseline_assessment',[PY,str(E/'assess.py'),str(stage/'baseline_assessment.json')],stage)==0
    assert run('compiler',docker()+['c++','--version'],stage)==0
    native=W/'papers/inten_loam/src/inten_loam_paper.cpp'
    replay=R/'replay_native.cpp';replay.write_text(transformed(native.read_text()))
    assert run('native_compile',docker()+['c++',*flags,'-c',str(native),'-o',str(R/'native.o')],stage)==0
    assert run('replay_compile',docker()+['c++',*flags,'-c',str(replay),'-o',str(R/'replay.o')],stage)==0
    for name,obj in (('probe','replay'),):
        assert run(name+'_compile',docker()+['c++',*flags,str(E/(name+'.cpp')),str(R/(obj+'.o')),*libs,'-o',str(R/name)],stage)==0
    assert run('unit_compile',docker()+['c++',*flags,str(W/'papers/inten_loam/test/test_inten_loam_paper.cpp'),str(R/'native.o'),*libs,'-lgtest_main','-lgtest','-o',str(R/'unit_tests')],stage)==0
    assert run('unit_tests',docker()+[str(R/'unit_tests')],stage)==0
    metadata=Path('/workspace/localization-env/inten-natural-traces/build/evaluation/CMakeFiles/pcd_dogfooding.dir')
    flags_runner=[]
    for line in (metadata/'flags.make').read_text().splitlines():
        if line.startswith(('CXX_DEFINES =','CXX_INCLUDES =','CXX_FLAGS =')):
            flags_runner += shlex.split(line.split('=',1)[1].replace('/workspace/localization-env/inten-natural-traces/worktree',str(W)))
    link_runner=shlex.split((metadata/'link.txt').read_text())
    runner_libs=[]
    for token in link_runner[link_runner.index('-o')+2:]:
        if token.startswith('-'):runner_libs.append(token)
        else:
            path=Path(token)
            runner_libs.append(str(path if path.is_absolute() else (metadata.parents[1]/path).resolve()))
    assert not any('--whole-archive' in v for v in runner_libs)
    # The fresh paper object resolves all paper symbols first. The old archive
    # supplies only the unchanged simplified InTenLoam object; duplicate paper
    # extraction would be a link error. All library bytes are pinned below.
    assert run('runner_compile',docker()+['c++',*flags_runner,str(W/'evaluation/src/pcd_dogfooding.cpp'),str(R/'native.o'),*runner_libs,'-o',str(R/'pcd_dogfooding')],stage)==0
    write(stage/'runner_libraries.json',{p:sha(p) for p in runner_libs if p.endswith('.a')})
    assert run('ldd',docker()+['ldd',str(R/'probe')],stage)==0
    write(stage/'binaries.json',dict(source_commit=git('rev-parse','HEAD'),
        binaries={p.name:dict(path=str(p),sha256=sha(p)) for p in [R/n for n in ('probe','pcd_dogfooding','unit_tests','native.o','replay.o')]},
        source_sha256=read(E/'protocol.json')['source_sha256'],
        replay_sha256=sha(replay),replay_transform='Only cached extracted frame and estimated initial-seed hooks; solver/grid production code unchanged.',
        linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')}))
    module('normalized_archive',E/'archive.py').archive(stage,E/'checks/build')
if __name__=='__main__':main()
