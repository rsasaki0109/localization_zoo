"""Render all cases from committed numerical evidence, without selecting outcomes."""
from pathlib import Path
import gzip,json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2]
def main():
    commit=subprocess.check_output(['git','log','-1','--format=%h','--',str(E/'report/summary.json')],cwd=W,text=True).strip()
    if not commit:raise RuntimeError('Commit numerical evidence before interpretation')
    s=json.loads((E/'report/summary.json').read_text())
    out=['# Every inherited fixed-node intensity control','',
         'Conditional same-history endpoints, not trajectories or official KITTI accuracy.',
         'All cases and arms are retained. Legacy/fixed-node joint is the primary pair; consensus stays a rejected-rule control.',
         f'Numerical commit `{commit}` precedes this interpretation. Full metrics and gate decisions are in `report/summary.json`.','']
    names=('Legacy joint','Normalized joint','Fixed-node joint','Legacy consensus','Normalized consensus','Fixed-node consensus')
    for metric,title,scale in (('length_ratio','Estimated length divided by GT step length',1),('vector_error','Vector error divided by GT step length',1),('rotation_error_rad','Rotation error [mrad]',1000)):
        out+=['## '+title,'','| History | Frame | '+' | '.join(names)+' |','|---|---:|'+'---:|'*6]
        for group,result in s['groups'].items():
            for c in result['cases']:
                vals=[c['metrics'][str(m)][metric]*scale for m in (0,2,4,1,3,5)]
                out.append('| '+group+' | '+str(c['frame'])+' | '+' | '.join(f'{v:.9f}' for v in vals)+' |')
        out.append('')
    out+=['## Every failed progression check','','```json',json.dumps([c for c in s['gate_checks'] if not c['passed']],indent=2),'```','',
          '## All inherited field paths','','Validity still requires the original observed-control count. All mixed and invalid outcomes are retained.','',
          '| History | Field | Paths | Queries | Both-valid faces | Mixed / neither-valid | Maximum intensity limit jump | Maximum gradient limit difference |',
          '|---|---|---:|---:|---:|---|---:|---:|']
    for group,fields in s['fields'].items():
        for f in fields:out.append(f"| {group} | {f['field']} | {f['paths']} | {f['queries']} | {f['both_valid']} | {f['mixed']} / {f['neither_valid']} | {f['max_abs_jump']:.9e} | {f['max_gradient_jump']:.9e} |")
    out+=['','## Every previous zero-weight hole with two valid neighbors','',
          '| History | Field | Path | Axis | Face | Previous limit jump | Fixed-node limit jump | Fixed-node gradient difference |',
          '|---|---|---:|---:|---:|---:|---:|---:|']
    prior=json.loads(gzip.decompress((E.with_name('inten_intensity_normalized')/'report/boundaries.json.gz').read_bytes()))
    now=json.loads(gzip.decompress((E/'report/boundaries.json.gz').read_bytes()))
    for group,fields in prior.items():
        for field,faces in fields.items():
            for old in faces:
                if old['both_valid'] and old['zero_weight_limit']:
                    b=next(b for b in now[group][field] if all(b[k]==old[k] for k in ('index','axis','face')))
                    out.append(f"| {group} | {field} | {old['index']} | {old['axis']} | {old['face']} | {old['jump']:.9f} | {b['jump']:.9e} | {b['gradient_jump']:.9e} |")
    (E/'cases.md').write_text('\n'.join(out)+'\n')
if __name__=='__main__':main()
