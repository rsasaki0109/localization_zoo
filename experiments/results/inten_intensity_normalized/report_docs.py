"""Render all frozen outcomes from committed numerical evidence; no selection."""
from pathlib import Path
import json,gzip
E=Path(__file__).resolve().parent
def main():
    s=json.loads((E/'report/summary.json').read_text())
    out=['# Every inherited normalized-intensity control','',
         'Conditional same-history native endpoints, not trajectories or official KITTI scores.',
         'Legacy/normalized joint is the primary pair. Consensus columns retain the rejected rule as controls.',
         'All values are reconstructed in `report/summary.json`; numerical commit `3055bb2e` precedes this table.','',
         '## Estimated step length divided by GT step length','',
         '| History | Frame | Legacy joint | Normalized joint | Legacy consensus | Normalized consensus |',
         '|---|---:|---:|---:|---:|---:|']
    for group,result in s['groups'].items():
        for c in result['cases']:
            values=[c['metrics'][str(m)]['length_ratio'] for m in (0,2,1,3)]
            out.append('| '+group+' | '+str(c['frame'])+' | '+' | '.join(f'{v:.9f}' for v in values)+' |')
    out+=['','## Translation vector error divided by GT step length','',
          '| History | Frame | Legacy joint | Normalized joint | Legacy consensus | Normalized consensus |',
          '|---|---:|---:|---:|---:|---:|']
    for group,result in s['groups'].items():
        for c in result['cases']:
            values=[c['metrics'][str(m)]['vector_error'] for m in (0,2,1,3)]
            out.append('| '+group+' | '+str(c['frame'])+' | '+' | '.join(f'{v:.9f}' for v in values)+' |')
    out+=['','## Rotation error and label changes','',
          'Consensus holds each arm\'s joint rotation, so these columns apply to both its joint and consensus output.','',
          '| History | Frame | Legacy rotation [mrad] | Normalized rotation [mrad] | Legacy joint bad length/vector | Normalized joint bad length/vector |',
          '|---|---:|---:|---:|---|---|']
    for group,result in s['groups'].items():
        for c in result['cases']:
            a,b=(c['metrics'][str(m)] for m in (0,2))
            labels=lambda x:f"{x['bad_length']}/{x['bad_vector']}"
            out.append(f"| {group} | {c['frame']} | {a['rotation_error_rad']*1000:.9f} | {b['rotation_error_rad']*1000:.9f} | {labels(a)} | {labels(b)} |")
    out+=['','## Native field limits on the inherited paths','',
          'Only positive-weight limits define the continuous covered domain. Zero-weight faces remain explicit.',
          'All native ULP neighbors, invalid sides, removable limits and gradients are in `report/boundaries.json.gz`.',
          'The `largest` witness retains the largest of all both-valid neighboring limits and may have zero limit weight; the `max_abs_jump`/`max_gradient_jump` summary fields refer to positive-weight limits.','',
          '| History | Frame | Paths | Queries | Positive-weight both-valid faces | Zero-weight faces | Zero-weight faces with two valid neighbors | Mixed / neither-valid faces | Max positive-weight jump |',
          '|---|---|---:|---:|---:|---:|---:|---|---:|']
    for group,fields in s['fields'].items():
        for r in fields:
            out.append(f"| {group} | {r['field']} | {r['paths']} | {r['queries']} | {r['positive_weight_both_valid']} | {r['zero_weight_limits']} | {r['zero_weight_neighbor_both_valid']} | {r['mixed']} / {r['neither_valid']} | {r['max_abs_jump']:.6e} |")
    out+=['','## All zero-weight faces with two valid neighbors','',
          'These are coverage holes, not continuous positive-weight field points. The one-sided limits disagree.',
          'They are excluded from the predeclared positive-denominator continuity domain and retained here without extrapolation.','',
          '| History | Frame | Path | Axis | Face | Right-minus-left intensity limit | Gradient limit difference norm |',
          '|---|---|---:|---:|---:|---:|---:|']
    details=json.loads(gzip.decompress((E/'report/boundaries.json.gz').read_bytes()))
    for group,fields in details.items():
        for frame,bs in fields.items():
            for b in bs:
                if b['both_valid'] and b['zero_weight_limit']:
                    out.append(f"| {group} | {frame} | {b['index']} | {b['axis']} | {b['face']} | {b['jump']:.9f} | {b['gradient_jump']:.9f} |")
    (E/'cases.md').write_text('\n'.join(out)+'\n')
if __name__=='__main__':main()
