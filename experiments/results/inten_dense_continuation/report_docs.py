"""Render every candidate score, selection and metric from committed evidence."""
from pathlib import Path
import json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2]
def main():
    commit=subprocess.check_output(['git','log','-1','--format=%h','--',str(E/'report/summary.json')],cwd=W,text=True).strip()
    if not commit:raise RuntimeError('Commit numerical evidence first')
    s=json.loads((E/'report/summary.json').read_text())
    out=['# Every full-cloud continuation outcome','',f'Numerical `{commit}` precedes interpretation. Conditional same-history controls, not recursive trajectories or official KITTI RTE.','',
        '| History | Frame | Pair | Selected arm | Base length/GT | Selected length/GT | Base vector error/GT | Selected vector error/GT | Base rotation [mrad] | Selected rotation [mrad] | Selected bad length/vector |',
        '|---|---:|---|---:|---:|---:|---:|---:|---:|---:|---|']
    for group,r in s['groups'].items():
        for c in r['cases']:
            for m in range(6):
                a,b=c['baseline_metrics'][str(m)],c['selected_metrics'][str(m)]
                label=('legacy' if m<2 else 'normalized' if m<4 else 'fixed-node')+(' joint' if m%2==0 else ' consensus')
                out.append(f"| {group} | {c['frame']} | {label} | {c['selected_arms'][m]} | {a['length_ratio']:.9f} | {b['length_ratio']:.9f} | {a['vector_error']:.9f} | {b['vector_error']:.9f} | {1000*a['rotation_error_rad']:.9f} | {1000*b['rotation_error_rad']:.9f} | {b['bad_length']} / {b['bad_vector']} |")
    out+=['','## Every native directional cost and coverage','','Every cost includes the capped penalty for all unmatched points. Counts are observations; the selector uses both full-population mean costs.','',
          '| History | Frame | Arm | Forward mean [m²] | Reverse mean [m²] | Forward within gate /population | Reverse within gate /population |',
          '|---|---:|---:|---:|---:|---|---|']
    for group,r in s['groups'].items():
        for c in r['cases']:
            for m in range(12):
                a,b=(c['scores'][str(m)][k] for k in ('forward','reverse'))
                out.append(f"| {group} | {c['frame']} | {m} | {a['mean']:.12f} | {b['mean']:.12f} | {a['within_gate']} / {a['population']} | {b['within_gate']} / {b['population']} |")
    out+=['','## Every failed progression check','','```json',json.dumps([c for c in s['gate_checks'] if not c['passed']],indent=2),'```']
    (E/'cases.md').write_text('\n'.join(out)+'\n')
if __name__=='__main__':main()
