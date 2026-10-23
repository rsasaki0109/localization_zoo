"""Frozen conditional-rotation factorial; only committed estimated rotation is used."""
import json
from common import E,W,R,PY,read,write,sha,git,run,restore,module
from consensus import select

def main():
    assert not git('status','--porcelain')
    protocol=read(E/'protocol.json')
    for name,digest in protocol['source_sha256'].items():assert sha(W/name)==digest,name
    stage=R/'stage';stage.mkdir()
    build=module('rotation_build',W/'experiments/results/inten_facade_loss/build.py')
    flags=build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags+=['-I'+str(W/'evaluation/src'),'-I'+str(E)]
    obj,binary=R/'native.o',R/'probe'
    assert run('mechanism_tests',[PY,'-m','unittest','discover','-s',str(E),'-p','test_*.py'],stage)==0
    assert run('compiler',build.docker(W)+['c++','--version'],stage)==0
    assert run('compile_native',build.docker(W)+['c++',*flags,'-c',str(E/'diagnostic_native.cpp'),'-o',str(obj)],stage)==0
    old=read(W/'experiments/results/inten_current_correspondences/baseline/compile_probe_command.json')['command']
    libs=old[old.index('/workspace/localization-env/inten-current-correspondences/native.o')+1:old.index('-o')]
    assert all(not token.endswith('/libinten_loam.a') for token in libs)
    static={token:sha(token) for token in libs if token.endswith('.a')}
    assert static==read(W/'experiments/results/inten_current_correspondences/baseline/binary.json')['linked_static_libraries']
    assert run('compile_probe',build.docker(W)+['c++',*flags,str(E/'probe.cpp'),str(obj),*libs,'-o',str(binary)],stage)==0
    assert run('linked_libraries',build.docker(W)+['ldd',str(binary)],stage)==0
    write(stage/'binary.json',dict(path=str(binary),sha256=sha(binary),object_sha256=sha(obj),source_commit=git('rev-parse','HEAD'),linked_static_libraries=static,
          flags_source=str(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make'),flags_source_sha256=sha(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make')))
    files=[]
    def load(folder,name,target,label):
        record=restore(folder,name,R/'inputs'/target);record['label']=label;files.append(record)
    original=W/'experiments/results/inten_current_correspondences/baseline'
    load(original,'features.jsonl','features.jsonl','features')
    load(original,'report.json','report.json','offline_truth')
    load(E.parent/'inten_cluster_motion/controls','native.stdout.jsonl','profiles.jsonl','profiles')
    load(E.parent/'inten_translation_consensus/controls','native.stdout.jsonl','previous.jsonl','previous_native')
    write(stage/'input_sources.json',dict(files=files,no_raw_scans_acquired=True,no_gt_in_native_input=True,rotation_source='Only preceding committed estimated native output, live exact control required'))
    profiles=[v for v in map(json.loads,(R/'inputs/profiles.jsonl').read_text().splitlines()) if v['type']=='profile']
    previous={(v['frame'],v['mode']):v for v in map(json.loads,(R/'inputs/previous.jsonl').read_text().splitlines())}
    text=R/'features.txt'
    with text.open('x') as out,(R/'inputs/features.jsonl').open() as source:
        for line in source:
            row=json.loads(line);out.write(str(row['frame'])+'\n')
            for features in row['features']:
                out.write(str(len(features))+'\n')
                for x,y,z,ring,cluster,intensity in features:out.write(f'{x:.17g} {y:.17g} {z:.17g} {ring} {cluster} {intensity:.17g}\n')
    assert sha(text)==read(E.parent/'inten_translation_consensus/controls/generated_features.json')['sha256']
    write(stage/'generated_features.json',dict(path=str(text),bytes=text.stat().st_size,sha256=sha(text),matches_preceding_serialization=True))
    lines,selections=[],[]
    for frame in protocol['frames']:
        records=[v for v in profiles if v['frame']==frame];decision=select(records)
        seed=list(previous[(frame,0)]['solver']['input']);shifted=list(seed)
        direction=records[0]['direction'];assert all(v['direction']==direction for v in records)
        for a in range(3):shifted[4*a+3]+=decision['shift_m']*direction[a]
        fixed=list(seed);shift_fixed=list(shifted);baseline=previous[(frame,0)]['output']
        for a in range(3):
            for b in range(3):fixed[4*a+b]=shift_fixed[4*a+b]=baseline[4*a+b]
        seeds=[seed,shifted,fixed,shift_fixed,fixed,shift_fixed]
        mask=decision['excluded_clusters'];masks=[[],mask,[],[],mask,mask]
        selections.append(dict(frame=frame,decision=decision,direction=direction,seeds=seeds,masks=masks,estimated_rotation_source=baseline))
        for mode in range(6):
            lines.append(' '.join([str(frame),str(mode),str(len(masks[mode])),*map(str,masks[mode]),*(format(v,'.17g') for v in seeds[mode])]))
    write(stage/'selections.json',dict(frames=selections,no_gt=True))
    payload=('\n'.join(lines)+'\n').encode();(stage/'input.txt').write_bytes(payload)
    assert run('native',build.docker(W)+[str(binary),str(text),str(stage/'residuals.jsonl')],stage,payload,jsonl=True)==0
    assert sha(binary)==read(stage/'binary.json')['sha256']
    write(R/'native_complete.json',dict(exit_code=0,source_commit=git('rev-parse','HEAD')))

if __name__=='__main__':main()
