"""Verify every frozen control, common-support equation and passive attribution."""
from pathlib import Path
import json,hashlib,importlib.util,tempfile,subprocess,copy,math
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
V=module('nonfacade_frozen_validation',E/'validate.py')
def equal(a,b):
    if isinstance(a,dict):return isinstance(b,dict) and a.keys()==b.keys() and all(equal(a[k],b[k]) for k in a)
    if isinstance(a,list):return isinstance(b,list) and len(a)==len(b) and all(equal(x,y) for x,y in zip(a,b))
    if isinstance(a,float):return isinstance(b,(float,int)) and math.isclose(a,b,rel_tol=1e-10,abs_tol=1e-12)
    return a==b

def query_adapter(report):
    # A zero-displacement passive native query is checked by the same geometry
    # verifier. This adapter declares no refinement/trajectory result.
    q=report['query']
    return dict(support=report,trace=dict(s2s={'output':q['input']},s2s_consensus=dict(usable=True,refinement=dict(returned_pairs=q['returned_pairs'],output=q['output']),applied=False),accepted_relative=q['output']))

def validate(history=True):
    for protocol in ('protocol.json','attribution_protocol.json'):
        for path,digest in V.read(E/protocol)['source_sha256'].items():V.require(hashlib.sha256((W/path).read_bytes()).hexdigest()==digest,'Pinned source changed: '+path)
    instrument=module('nonfacade_source_transform',E/'instrument.py')
    V.require(instrument.transformed((W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text())==(E/'diagnostic_native.cpp').read_text(),'Native source transformation differs')
    groups={};native=0;queries=0;failures=[];primitive_count=0
    with tempfile.TemporaryDirectory() as temporary:
        tmp=Path(temporary)
        for group in ('old','01','02','08'):
            folder=E/'groups'/group
            for name in V.read(folder/'archive.json')['files']:V.decode(folder,name)
            raw=V.decode(folder,'native.stdout.jsonl');primitives=V.decode(folder,'primitives.jsonl')
            (tmp/'native.jsonl').write_bytes(raw);(tmp/'primitives.jsonl').write_bytes(primitives)
            result=V.evaluate(group,tmp/'native.jsonl',tmp/'primitives.jsonl')
            saved=V.read(folder/'result.json')
            for key,value in result.items():V.require(equal(value,saved[key]),'Guard report reconstruction failed: '+group+'/'+key)
            V.require(saved['exit_code']==0 and V.read(folder/'native_command.json')['exit_code']==0,'Native command failure')
            native+=result['native_calls'];primitive_count+=len(primitives.splitlines());failures.extend(dict(group=group,**f) for f in result['gate_checks'] if not f['passed'])
            groups[group]=dict(cases=len(result['cases']),gate_passed=result['gate_passed'])
            lookup={(r['frame'],r['mode']):r for r in map(json.loads,raw.splitlines()) if r['type']=='terminal'}
            for role in ('joint','refined'):
                prefix=group+'/'+role
                rows=list(map(json.loads,V.decode(E/'attribution',prefix+'.stdout.jsonl').splitlines()))
                ps=list(map(json.loads,V.decode(E/'attribution',prefix+'_primitives.jsonl').splitlines()));primitive_count+=len(ps)
                by={r['frame']:[] for r in rows}
                for r in ps:by[r['frame']].append(r)
                for r in rows:
                    f=r['frame'];s=r['support'];q=s['query'];expected=lookup[f,0]['trace']['s2s']['output'] if role=='joint' else lookup[f,1]['trace']['accepted_relative']
                    V.require(q['input']==q['output']==expected,'Wrong passive estimated query pose')
                    V.check_support(query_adapter(s),by[f]);queries+=1
                    if role=='joint' and lookup[f,2]['support']['evaluated']:
                        old=lookup[f,2]['support'];V.require(s['original_costs']==old['original_costs'] and s['counts']==old['counts'],'Original native query parity changed')
                V.require(V.read(E/'attribution'/(prefix+'_command.json'))['exit_code']==0,'Passive query failed')
        for name in V.read(E/'attribution/archive.json')['files']:V.decode(E/'attribution',name)
    V.require(queries==V.read(E/'attribution/summary.json')['native_calls'],'Passive call coverage differs')
    if history:
        base=V.read(E/'protocol.json')['parent_commit'];changed=subprocess.check_output(['git','diff','--name-only',base,'--'],cwd=W,text=True).splitlines()
        for name in changed:V.require(name.startswith(('experiments/results/inten_nonfacade_consensus_support/','tests/test_inten_nonfacade_consensus_support.py')) or name in ('docs/roadmap.md','papers/inten_loam/README.md'),'Historical/production file changed: '+name)
    return dict(exit_code=0,groups=groups,native_terminal_calls=native,passive_attribution_queries=queries,primitives_verified=primitive_count,gate_passed=not failures,failed_gate_checks=failures,all_controls_exact=True,all_geometry_hg_robust_losses_and_native_decisions_reconstructed=True,all_archives_byte_verified=True,all_passive_queries_no_pose_update=True,original_query_parity_bitwise=True,production_and_history_preserved=history)
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);args=p.parse_args();args.output.write_text(json.dumps(validate(),indent=2)+'\n')
