from pathlib import Path
import json
E=Path(__file__).resolve().parent
def render():
    r=json.loads((E/'report/summary.json').read_text())
    cases=[(g,c) for g,v in r['groups'].items() for c in v['cases']]
    lines=['# Complete fixed-prior control outcomes','',
      'Numerical evidence `a358a572` and full-trace attribution `2eca2585` precede interpretation.',
      'All histories are conditional development controls, not recursive trajectories or held-out data.',
      'The frozen progression gate fails; neither own-prior nor common-prior reciprocal support is promoted.','']
    for title,modes in (('Primary joint comparison',(0,6,12,14)),('Paired consensus comparison',(1,7,13,15))):
        lines+=['## '+title,'',
          '| History/frame | Original10 length/GT | Unfiltered100 | Reciprocal own prior | Reciprocal common prior | Vector errors in same order | Rotation errors [mrad] |',
          '|---|---:|---:|---:|---:|---|---|']
        for g,c in cases:
            values=[c['metrics'][str(m)] for m in modes]
            lengths=' | '.join(f"{v['length_ratio']:.9f}" for v in values)
            vectors=' / '.join(f"{v['vector_error']:.9f}" for v in values)
            rotations=' / '.join(f"{1000*v['rotation_error_rad']:.9f}" for v in values)
            lines.append(f"| {g}/{c['frame']} | {lengths} | {vectors} | {rotations} |")
        lines.append('')
    lines+=['## Controlled contrasts for every endpoint','',
      'Each triplet is total / fixed-information association / prior-feedback contrast.',
      'Total equals the other two contrasts added. Positive error changes are adverse.',
      'These are controlled conditional contrasts with downstream interactions, not trajectory-error percentages.','',
      '| History/frame | Solve | Length-error contrasts | Vector-error contrasts | Rotation-error contrasts [mrad] |',
      '|---|---|---|---|---|']
    for g,c in cases:
        for solve,values in c['controlled_contrasts'].items():
            fields=[]
            for metric,scale in (('length_error',1.),('vector_error',1.),('rotation_error_rad',1000.)):
                fields.append(' / '.join(f"{scale*values[metric][key]:+.9f}" for key in ('total','fixed_information','prior_feedback')))
            lines.append(f"| {g}/{c['frame']} | {solve} | "+' | '.join(fields)+' |')
    labels=['legacy joint10','legacy consensus10','normalized joint10','normalized consensus10','fixed-node joint10','fixed-node consensus10',
      'legacy joint100','legacy consensus100','normalized joint100','normalized consensus100','fixed-node joint100','fixed-node consensus100',
      'reciprocal own-prior joint100','reciprocal own-prior consensus100','reciprocal common-prior joint100','reciprocal common-prior consensus100',
      'unfiltered common-prior joint100','unfiltered common-prior consensus100']
    lines+=['','## All arms, including every negative result','',
      '| History/frame | Arm | Length/GT | Length error | Vector error/GT | Rotation [mrad] | Bad length/vector | S2S stop/iterations |',
      '|---|---|---:|---:|---:|---:|---|---|']
    for g,c in cases:
        for m,label in enumerate(labels):
            v=c['metrics'][str(m)];stop=c['solver_stops'][str(m)]
            lines.append(f"| {g}/{c['frame']} | {m}: {label} | {v['length_ratio']:.9f} | {v['length_error']:.9f} | {v['vector_error']:.9f} | {1000*v['rotation_error_rad']:.9f} | {int(v['bad_length'])}/{int(v['bad_vector'])} | {stop['stop']}/{stop['iterations']} |")
    lines+=['','## Every failed progression condition','']
    lines += ['- '+json.dumps(c,sort_keys=True) for c in r['gate_checks'] if not c['passed']]
    return '\n'.join(lines)+'\n'
if __name__=='__main__':(E/'cases.md').write_text(render())
