"""Link-only recovery after successful native compilation/CTest; never rebuild or overwrite."""
from pathlib import Path
import importlib.util,shlex

def main():
    E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
    spec=importlib.util.spec_from_file_location('acceleration_link_build',E/'build_corrected.py')
    build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build)
    assert not (R/'pcd_dogfooding').exists()
    folder=E/'checks/candidate_build_corrected'
    import json
    for label in ('configure','build','ctest'):assert json.loads((folder/(label+'_command.json')).read_text())['exit_code']==0
    metadata=build.OLD/'evaluation/CMakeFiles/pcd_dogfooding.dir'
    link=shlex.split((metadata/'link.txt').read_text());libs=[];pins={}
    for token in link[link.index('-o')+2:]:
        if token.startswith('-'):libs.append(token);continue
        path=Path(token)
        if not path.is_absolute():path=(build.OLD/'evaluation'/path).resolve()
        if path==build.OLD/'papers/inten_loam/libinten_loam.a':path=R/'build_corrected/papers/inten_loam/libinten_loam.a'
        if path.suffix=='.a':pins[str(path)]=build.sha(path)
        libs.append(str(path))
    build.run('runner',build.docker()+['c++',*build.flags(metadata),str(W/'evaluation/src/pcd_dogfooding.cpp'),*libs,'-o',str(R/'pcd_dogfooding')],folder)
    build.write(E/'binaries.json',dict(runner=dict(path=str(R/'pcd_dogfooding'),sha256=build.sha(R/'pcd_dogfooding')),linked_static_libraries=pins,
       resolved_parent_metadata={str(metadata/n):build.sha(metadata/n) for n in ('flags.make','link.txt')},
       explanation='Link-only continuation uses the already compiled/tested accelerated InTEn library from build_corrected. No object or prior binary is overwritten; all other dependencies remain pinned.'))

if __name__=='__main__':main()
