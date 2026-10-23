"""Additional raw-trace attribution audit; no new candidate or changed gate."""
from pathlib import Path
import io,json,sys,shutil
import numpy as np
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
E=W/'experiments/results/inten_prior_control';sys.path.insert(0,str(E))
import verify_portable as v
S=Path('/tmp/localization-zoo-inten-prior-control-20261023')
out=S/'attribution_stage';out.mkdir(exist_ok=True)
shutil.copyfile(Path(__file__),out/'attribution_audit.py')
rows=[];observations=0
for group in ('old','01','02','08'):
    terminals={};priors=[]
    for line in io.BytesIO(v.decode(E/'groups'/group,'native.stdout.jsonl')):
        if line.startswith(b'{"type":"terminal"'):
            r=json.loads(line);terminals[r['frame'],r['mode']]=r['trace']
        elif line.startswith(b'{"type":"prior_observation"'):
            priors.append(json.loads(line))
    observations+=len(priors)
    for record in priors:
        reference=terminals[record['frame'],record['reference_mode']]
        solver=reference['s2s_consensus']['refinement'] if record['translation_only'] else reference['s2s']
        center=np.asarray(solver['input']).reshape(4,4)[:3,3]
        assert np.array_equal(center,record['prediction']), 'Cross-arm estimated prior center differs'
    for frame in v.read(E/'protocol.json')['input_pins'][group]['frames']:
        own,common=(terminals[frame,m] for m in (12,14))
        paired,common_paired=(terminals[frame,m] for m in (13,15))
        def info(trace,refine=False):
            solver=trace['s2s_consensus']['refinement'] if refine else trace['s2s']
            return solver.get('translation_prior_information',[0.]*9)
        joint_zero=not any(info(own)) and not any(info(terminals[frame,6]))
        refinement_zero=not any(info(paired,True)) and not any(info(terminals[frame,7],True))
        row=dict(group=group,frame=frame,joint_full_trace_identical=own==common,
            consensus_full_trace_identical=paired==common_paired,
            joint_reference_and_own_information_zero=joint_zero,
            refinement_reference_and_own_information_zero=refinement_zero,
            own_joint_information=info(own),reference_joint_information=info(terminals[frame,6]))
        if group in ('old','01','08'):
            assert joint_zero and refinement_zero
            assert row['joint_full_trace_identical'] and row['consensus_full_trace_identical']
            assert all(not any(p['information']) for p in priors if p['frame']==frame and p['mode'] in (14,15))
        rows.append(row)
result=dict(exit_code=0,source_measurement_commit='a358a572',rows=rows,
    complete_joint_trace_identities=sum(r['joint_full_trace_identical'] for r in rows),
    complete_consensus_trace_identities=sum(r['consensus_full_trace_identical'] for r in rows),
    changed_joint_cases=[[r['group'],r['frame']] for r in rows if not r['joint_full_trace_identical']],
    prior_observations_with_cross_arm_center_checked=observations,
    all_cross_arm_estimated_prior_centers_exact=True,
    scope='Conditional controlled trace evidence only. No full-trajectory/paper/SOTA claim or changed progression gate.')
(out/'result.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({k:v for k,v in result.items() if k!='rows'}))
