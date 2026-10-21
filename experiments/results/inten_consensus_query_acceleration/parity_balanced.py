"""Bitwise profile/estimated-solve parity at both fixed worker counts before raw acquisition."""
import json,subprocess,os
from common import E,W,R,PY,read,write,sha,git,run,restore,module

def validate(rows,previous,profiles):
    lookup={(v['frame'],v['mode']):v for v in previous}
    expected_profiles={(v['frame'],v['cluster']):v['costs_m2'] for v in profiles if v['type']=='profile'}
    actual_profiles={(v['frame'],v['cluster']):v['costs_m2'] for v in rows if v['type']=='profile'}
    assert actual_profiles==expected_profiles
    count=0
    for v in rows:
        if v['type']!='terminal':continue
        frame,mode=v['frame'],v['mode'];trace=v['trace'];baseline=lookup[(frame,0)]['solver']
        def same(a,b):
            a,b=dict(a),dict(b)
            first=sum(b['iterations'][0]['class_pairs']);assert b.pop('returned_pairs')==first
            returned=a.pop('returned_pairs');assert returned==a['iterations'][-1]['pairs']
            assert a==b
        same(trace['s2s'],baseline)
        expected=lookup[(frame,5 if mode else 0)]
        # Parent raw arm5 is not the declared fallback at247/433.
        if mode:
            c=trace['s2s_consensus'];assert c['original_estimate']==lookup[(frame,0)]['output']
            if c['usable']:
                assert c['applied'];same(c['refinement'],expected['solver'])
            else:
                assert not c['applied'] and not c['refinement']['called'];expected=lookup[(frame,0)]
        assert trace['accepted_relative']==expected['output']
        count+=1
    assert count==16
    return dict(profile_count=len(actual_profiles),native_calls=count,all_profile_costs_bitwise_original=True,all_original_and_candidate_solver_traces_exact_except_declared_forced_count=True)

def main():
    assert not git('status','--porcelain')
    stage=R/'parity_stage_balanced';stage.mkdir()
    previous=E.parent/'inten_fixed_rotation';frames=read(previous/'protocol.json')['frames']
    files=[]
    for folder,name,target in [(E.parent/'inten_current_correspondences/baseline','features.jsonl','features.jsonl'),(previous/'controls','native.stdout.jsonl','previous.jsonl'),(E.parent/'inten_cluster_motion/controls','native.stdout.jsonl','profiles.jsonl')]:
        files.append(restore(folder,name,R/'parity_inputs_balanced'/target))
    write(stage/'input_sources.json',dict(files=files,no_gt=True))
    old=list(map(json.loads,(R/'parity_inputs_balanced/previous.jsonl').read_text().splitlines()));profiles=list(map(json.loads,(R/'parity_inputs_balanced/profiles.jsonl').read_text().splitlines()))
    text=R/'parity_features_balanced.txt'
    with text.open('x') as out,(R/'parity_inputs_balanced/features.jsonl').open() as source:
        for line in source:
            f=json.loads(line);out.write(str(f['frame'])+'\n')
            for features in f['features']:
                out.write(str(len(features))+'\n')
                for x,y,z,ring,cluster,intensity in features:out.write(f'{x:.17g} {y:.17g} {z:.17g} {ring} {cluster} {intensity:.17g}\n')
    assert sha(text)==read(previous/'controls/generated_features.json')['sha256']
    write(stage/'features.json',dict(path=str(text),sha256=sha(text)))
    payload=('\n'.join(str(frame)+' '+' '.join(format(v,'.17g') for v in next(p for p in old if p['frame']==frame and p['mode']==0)['solver']['input']) for frame in frames)+'\n').encode()
    (stage/'input.txt').write_bytes(payload)
    build=module('recursive_parity_build',E/'build_balanced.py');flags=build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags+=['-I'+str(E),'-I'+str(W/'evaluation/src')]
    obj,binary=R/'replay_balanced.o',R/'parity_probe_balanced'
    assert run('compile_replay',build.docker()+['c++',*flags,'-c',str(E/'replay_native.cpp'),'-o',str(obj)],stage)==0
    command=read(W/'experiments/results/inten_current_correspondences/baseline/compile_probe_command.json')['command']
    libs=command[command.index('/workspace/localization-env/inten-current-correspondences/native.o')+1:command.index('-o')]
    assert run('compile_probe',build.docker()+['c++',*flags,str(E/'parity_probe.cpp'),str(obj),*libs,'-o',str(binary)],stage)==0
    write(stage/'binary.json',dict(source_commit=git('rev-parse','HEAD'),sha256=sha(binary),object_sha256=sha(obj),linked_static_libraries={p:sha(p) for p in libs if p.endswith('.a')}))
    records={}
    for threads in (1,4):
        docker=build.docker();docker[docker.index('OMP_NUM_THREADS=1')]='OMP_NUM_THREADS='+str(threads)
        assert run('native_'+str(threads),docker+[str(binary),str(text)],stage,payload,jsonl=True)==0
        records[threads]=list(map(json.loads,(stage/('native_'+str(threads)+'.stdout.jsonl')).read_text().splitlines()))
        result=validate(records[threads],old,profiles);write(stage/('parity_'+str(threads)+'.json'),dict(exit_code=0,threads=threads,**result))
    assert records[1]==records[4]
    write(stage/'result.json',dict(exit_code=0,bitwise_worker_count_parity=True,no_gt_input=True,source_commit=git('rev-parse','HEAD')))
    module('recursive_parity_archive',E/'archive.py').archive(stage,E/'checks/cached_parity_balanced')

if __name__=='__main__':main()
