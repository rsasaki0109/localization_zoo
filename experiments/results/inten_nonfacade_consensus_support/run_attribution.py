"""Use the SAME pinned native object to compare fresh native query supports."""
from common import *
V=module('support_attribution_validation',E/'validate.py')

def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'attribution_protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    stage=R/'attribution_stage';stage.mkdir()
    old=read(E/'checks/build/probe_compile_command.json')['command'];command=[str(E/'query_probe.cpp') if v==str(E/'probe.cpp') else str(R/'query_probe') if v==str(R/'probe') else v for v in old]
    assert sha(R/'replay.o')==read(E/'checks/build/binaries.json')['binaries']['replay.o']['sha256']
    assert run('compile_query',command,stage)==0
    write(stage/'query_binary.json',dict(path=str(R/'query_probe'),sha256=sha(R/'query_probe'),object_sha256=sha(R/'replay.o'),linked_static_libraries=read(E/'checks/build/binaries.json')['linked_static_libraries'],native_original_object_unchanged=True,source_commit=git('rev-parse','HEAD')))
    all_reports={}
    for group in ('old','01','02','08'):
        folder=stage/group;folder.mkdir()
        meta=read(E/'groups'/group/'input_sources.json');features=Path(meta['feature_cache_private']);assert sha(features)==meta['feature_cache_sha256']
        native=list(map(json.loads,V.decode(E/'groups'/group,'native.stdout.jsonl').splitlines()))
        lookup={(r['frame'],r['mode']):r for r in native if r['type']=='terminal'}
        roles={}
        for role in ('joint','refined'):
            seeds={f:(lookup[f,0]['trace']['s2s']['output'] if role=='joint' else lookup[f,1]['trace']['accepted_relative']) for f in meta['frames']}
            payload=('\n'.join(str(f)+' '+' '.join(format(v,'.17g') for v in pose) for f,pose in seeds.items())+'\n').encode();(folder/(role+'_input.txt')).write_bytes(payload)
            assert run(role,old[:old.index('c++')]+[str(R/'query_probe'),str(features),str(folder/(role+'_primitives.jsonl'))],folder,payload,jsonl=True)==0
            rows=list(map(json.loads,(folder/(role+'.stdout.jsonl')).read_text().splitlines()))
            roles[role]={r['frame']:r for r in rows};assert sorted(roles[role])==meta['frames']
            for f,row in roles[role].items():
                s=row['support'];q=s['query'];assert q['input']==q['output']==seeds[f]
                assert q['stop']=='no_iterations' and len(q['iterations'])==1 and not q['iterations'][0]['step_attempted']
                assert s['complete'] and s['original_costs']==s['candidate_costs'] and not s['allowed']
                if role=='joint' and lookup[f,2]['support']['evaluated']:
                    old_s=lookup[f,2]['support'];assert s['original_costs']==old_s['original_costs'] and s['counts']==old_s['counts']
        report=[]
        for f in meta['frames']:
            frozen=lookup[f,2]['support'];j=roles['joint'][f]['support'];r=roles['refined'][f]['support']
            report.append(dict(frame=f,joint_counts=j['counts'],refined_counts=r['counts'],joint_costs=j['original_costs'],refined_fresh_costs=r['original_costs'],refined_frozen_available_costs=frozen['candidate_costs'],frozen_coverage_complete=frozen['complete'],guard_evaluated=frozen['evaluated'],guard_allowed=frozen['allowed'],joint_mean_costs=[v/n if n else None for v,n in zip(j['original_costs'],j['counts'])],refined_mean_costs=[v/n if n else None for v,n in zip(r['original_costs'],r['counts'])]))
        write(folder/'report.json',dict(exit_code=0,group=group,cases=report,query_poses_unchanged=True,original_guard_query_parity_bitwise=True,source_commit=git('rev-parse','HEAD')))
        all_reports[group]=report
    write(stage/'summary.json',dict(exit_code=0,native_calls=sum(2*len(v) for v in all_reports.values()),all31_endpoints_both_roles_retained=True,no_optimizer_no_gt_no_guard_amendment=True,source_commit=git('rev-parse','HEAD')))
    module('support_attribution_archive',E/'archive.py').archive(stage,E/'attribution')
if __name__=='__main__':main()
