"""Every committed arm and stop; no field or budget chosen after outcomes."""
from pathlib import Path
import json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2]
def main():
    commit=subprocess.check_output(['git','log','-1','--format=%h','--',str(E/'report/summary.json')],cwd=W,text=True).strip()
    if not commit:raise RuntimeError('Commit numbers before rendering')
    s=json.loads((E/'report/summary.json').read_text())
    out=['# Every fixed-budget outcome','',f'Numerical `{commit}` precedes interpretation. Conditional same-history endpoints, not full trajectories or KITTI RTE.','',
         '| History | Frame | Arm | Budget | Length /GT | Vector error /GT | Rotation [mrad] | Joint stop | Joint iterations | Bad length / vector |',
         '|---|---:|---|---:|---:|---:|---:|---|---:|---|']
    for group,r in s['groups'].items():
        for c in r['cases']:
            for m in range(12):
                a=c['metrics'][str(m)];stop=c['solver_stops'][str(m)]
                name=('legacy' if m%6<2 else 'normalized' if m%6<4 else 'fixed-node')+(' joint' if m%2==0 else ' consensus')
                out.append(f"| {group} | {c['frame']} | {name} | {10 if m<6 else 100} | {a['length_ratio']:.9f} | {a['vector_error']:.9f} | {1000*a['rotation_error_rad']:.9f} | {stop['stop']} | {stop['iterations']} | {a['bad_length']} / {a['bad_vector']} |")
    out+=['','## Every failed prospective progression check','','```json',json.dumps([c for c in s['gate_checks'] if not c['passed']],indent=2),'```']
    (E/'cases.md').write_text('\n'.join(out)+'\n')
if __name__=='__main__':main()
