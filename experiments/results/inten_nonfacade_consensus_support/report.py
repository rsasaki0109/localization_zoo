"""Summarize every frozen endpoint and per-class counterexample without selection."""
from common import *
V=module('support_report_validation',E/'validate.py')
def main():
    folder=E/'report';folder.mkdir();groups={g:read(E/'groups'/g/'result.json') for g in ('old','01','02','08')}
    cases=[dict(group=g,**r) for g,j in groups.items() for r in j['cases']]
    failures=[dict(group=g,**r) for g,j in groups.items() for r in j['gate_checks'] if not r['passed']]
    write(folder/'summary.json',dict(command=[PY,str(E/'report.py')],cwd=str(W),source_commit=git('rev-parse','HEAD'),exit_code=0,endpoints=len(cases),native_terminal_calls=sum(j['native_calls'] for j in groups.values()),passive_attribution_queries=read(E/'attribution/summary.json')['native_calls'],gate_passed=not failures,failed_gate_checks=failures,guard_allowed=sum(r['guard_applied'] for r in cases),guard_rejected=sum(r['support']['queried'] and not r['support']['allowed'] for r in cases),fallback_no_query=sum(not r['support']['queried'] for r in cases),no_production_promotion=True,sources={g:dict(result_sha256=sha(E/'groups'/g/'result.json'),archive_sha256=sha(E/'groups'/g/'archive.json')) for g in groups}))
    lines=['| History | Frame | Original length/GT | Old length/GT | Guard length/GT | Original vector error/GT | Guard vector error/GT | Guard decision | Missing original field samples |','|---|---:|---:|---:|---:|---:|---:|---|---:|']
    for r in cases:
        ms=r['metrics'];s=r['support'];decision='accept' if r['guard_applied'] else ('veto' if s['queried'] else 'fallback')
        values=[ms['0']['length_ratio'],ms['1']['length_ratio'],ms['2']['length_ratio'],ms['0']['vector_error'],ms['2']['vector_error']]
        lines.append('| '+r['group']+' | '+str(r['frame'])+' | '+' | '.join(f'{v:.6f}' for v in values)+' | '+decision+' | '+str(s['missing_field'])+' |')
    (folder/'all_cases.md').write_text('\n'.join(lines)+'\n')
    parts=[]
    for group in groups:
        rows=list(map(json.loads,V.decode(E/'groups'/group,'native.stdout.jsonl').splitlines()))
        for row in rows:
            if row['type']=='terminal' and row['mode']==2:
                s=row['support'];parts.append(dict(group=group,frame=row['frame'],evaluated=s['evaluated'],complete=s['complete'],allowed=s['allowed'],counts=s['counts'],original_costs=s['original_costs'],candidate_available_costs=s['candidate_costs'],candidate_cost_is_partial=not s['complete']))
    write(folder/'all_class_costs.json',dict(classes=['edge','ground','facade_excluded_from_guard','reflector'],cost='Exact Huber rho. Candidate available sum is partial when field coverage is missing and MUST NOT be treated as a complete comparison.',cases=parts))
if __name__=='__main__':main()
