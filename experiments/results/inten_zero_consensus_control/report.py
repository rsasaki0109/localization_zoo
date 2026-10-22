"""Tabulate every predeclared endpoint, keeping native decisions and adverse outcomes."""
from common import *
def main():
    folder=E/'report';folder.mkdir()
    groups={g:read(E/'groups'/g/'result.json') for g in ('old','01','02','08')}
    rows=[];failures=[]
    for group,j in groups.items():
        rows.extend(dict(group=group,**r) for r in j['cases'])
        failures.extend(dict(group=group,**r) for r in j['gate_checks'] if not r['passed'])
    entries=[r for r in rows if r['group']!='old' and r['entry']]
    fixed=[r for r in entries if not r['metrics']['2']['bad_length']]
    sources={g:dict(result_sha256=sha(E/'groups'/g/'result.json'),archive_sha256=sha(E/'groups'/g/'archive.json'),source_commit=j['source_commit']) for g,j in groups.items()}
    write(folder/'summary.json',dict(command=[PY,str(E/'report.py')],cwd=str(W),source_commit=git('rev-parse','HEAD'),exit_code=0,total_endpoints=len(rows),native_calls=8*len(rows),new_immediate_entries=len(entries),new_immediate_entries_no_longer_bad_length=len(fixed),failed_gate_checks=failures,gate_passed=not failures,no_recursive_candidate_adopted=True,groups={g:dict(endpoints=len(j['cases']),entry_frames=[r['frame'] for r in j['cases'] if r['entry']],gate_passed=j['local_gate_passed']) for g,j in groups.items()},sources=sources))
    lines=['| History / sequence | Frame | Role | Joint length/GT | Old length/GT | New length/GT | Old vector error/GT | New vector error/GT |','|---|---:|---|---:|---:|---:|---:|---:|']
    for r in rows:
        ms=r['metrics'];lines.append('| '+('original01' if r['group']=='old' else 'candidate'+r['group'])+' | '+str(r['frame'])+' | '+('old endpoint' if r['group']=='old' else ('damage entry' if r['entry'] else 'preceding control'))+' | '+' | '.join(f'{v:.6f}' for v in [ms['0']['length_ratio'],ms['1']['length_ratio'],ms['2']['length_ratio'],ms['1']['vector_error'],ms['2']['vector_error']])+' |')
    (folder/'all_cases.md').write_text('\n'.join(lines)+'\n')
if __name__=='__main__':main()
