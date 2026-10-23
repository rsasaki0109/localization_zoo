from pathlib import Path
import json
E=Path(__file__).resolve().parent
def render():
    r=json.loads((E/'report/summary.json').read_text())
    lines=['# Complete reciprocal-support outcomes','',
      'Numerical evidence `f22127ab` precedes interpretation. Every inherited endpoint',
      'and all14 arms remain. Original-history01 and the other rejected-recursive',
      'candidate histories are conditional controls, not new trajectories or held-out data.',
      'The fixed local progression gate fails. No production integration/full trial follows.','',
      '## Primary legacy joint comparison','',
      '| History/frame | Length/GT10 | Length/GT100 | Reciprocal100 | Vector10 | Vector100 | Reciprocal vector | Rotation10/100/reciprocal [mrad] |',
      '|---|---:|---:|---:|---:|---:|---:|---|']
    all_cases=[(g,c) for g,v in r['groups'].items() for c in v['cases']]
    for g,c in all_cases:
        values=[c['metrics'][str(m)] for m in (0,6,12)]
        fields=[f"{v['length_ratio']:.9f}" for v in values]+[f"{v['vector_error']:.9f}" for v in values]
        rotation='/'.join(f"{v['rotation_error_rad']*1000:.9f}" for v in values)
        lines.append('| '+f"{g}/{c['frame']} | "+' | '.join(fields)+' | '+rotation+' |')
    lines+=['','## Paired consensus comparison','',
      '| History/frame | Length/GT10 | Length/GT100 | Reciprocal100 | Vector10 | Vector100 | Reciprocal vector |',
      '|---|---:|---:|---:|---:|---:|---:|']
    for g,c in all_cases:
        values=[c['metrics'][str(m)] for m in (1,7,13)]
        fields=[f"{v['length_ratio']:.9f}" for v in values]+[f"{v['vector_error']:.9f}" for v in values]
        lines.append('| '+f"{g}/{c['frame']} | "+' | '.join(fields)+' |')
    labels=['legacy joint10','legacy consensus10','normalized joint10','normalized consensus10','fixed-node joint10','fixed-node consensus10',
      'legacy joint100','legacy consensus100','normalized joint100','normalized consensus100','fixed-node joint100','fixed-node consensus100',
      'reciprocal legacy joint100','reciprocal legacy consensus100']
    lines+=['','## Every arm and every favorable/adverse endpoint','',
      '| History/frame | Arm | Length/GT | Length error | Vector error/GT | Rotation [mrad] | Bad length/vector | S2S stop/iterations |',
      '|---|---|---:|---:|---:|---:|---|---|']
    for g,c in all_cases:
        for m,label in enumerate(labels):
            v=c['metrics'][str(m)];s=c['solver_stops'][str(m)]
            lines.append(f"| {g}/{c['frame']} | {m}: {label} | {v['length_ratio']:.9f} | {v['length_error']:.9f} | {v['vector_error']:.9f} | {1000*v['rotation_error_rad']:.9f} | {int(v['bad_length'])}/{int(v['bad_vector'])} | {s['stop']}/{s['iterations']} |")
    lines+=['','## Every failed progression condition','']
    for c in r['gate_checks']:
        if not c['passed']:lines.append('- '+json.dumps(c,sort_keys=True))
    return '\n'.join(lines)+'\n'
if __name__=='__main__':(E/'cases.md').write_text(render())
