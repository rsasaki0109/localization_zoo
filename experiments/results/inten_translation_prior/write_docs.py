"""Write all outcomes only from committed full-sequence numerical evidence."""
from pathlib import Path
import json
import subprocess

E=Path(__file__).resolve().parent
W=E.parents[2]


def read(path):return json.loads(path.read_text())
def committed(path):
    if subprocess.check_output(['git','show','HEAD:'+path.relative_to(W).as_posix()],cwd=W)!=path.read_bytes():
        raise ValueError('Numerical source must be committed: '+str(path))
def f(value):return 'n/a' if value is None else (str(value) if isinstance(value,int) else f'{value:.6f}')
def intervals(ranges):return ', '.join(str(a) if a==b else f'{a}–{b}' for a,b in ranges) or 'none'


def main():
    for path in (Path(__file__).resolve(),E/'postprocessing_protocol.json',E/'readme_template.md',E/'report/summary.json'):committed(path)
    report=read(E/'report/summary.json');sequences=report['sequences']
    if not report['all_sequences_present'] or len(sequences)!=11:raise ValueError('All full sequences required')
    commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip()
    lower=', '.join(report['lower_rte_sequences']) or 'none';higher=', '.join(report['higher_rte_sequences']) or 'none';equal=', '.join(report['equal_rte_sequences']) or 'none'
    table=['| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |','|---|---:|---:|---:|---:|---:|']
    for seq,data in sequences.items():
        a,b=data['modes']['baseline'],data['modes']['candidate']
        table.append(f"| {seq} | {f(a['kitti_rte_trans_pct'])} | {f(b['kitti_rte_trans_pct'])} | {f(a['ate_m'])} | {f(b['ate_m'])} | {a['final_bad_length']} / {b['final_bad_length']} |")
    metrics=dict(kitti_rte_trans_pct='official RTE [%]',kitti_rte_rot_deg_per_100m='rotation [deg/100 m]',ate_m='ATE [m]',final_bad_length='bad-length steps',final_bad_vector='bad-vector steps')
    adverse=[]
    for seq,keys in report['adverse_metrics'].items():
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        if keys:adverse.append('- '+seq+': '+'; '.join(f'{metrics[k]} {f(a[k])} → {f(b[k])}' for k in keys)+'.')
    failures=[]
    for seq in ('01','02','10'):
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        failures.append(f"- {seq}: final bad-length frames {intervals(a['bad_length_frame_ranges'])} → {intervals(b['bad_length_frame_ranges'])}; S2S bad-length outputs {a['s2s_bad_length']} → {b['s2s_bad_length']}; final rejected S2M {a['s2m_rejected']} → {b['s2m_rejected']}.")
    growth=['','Candidate sequences exceeding twice the cited paper RTE:','',
        '| Sequence | Option RTE [%] | Paper RTE [%] | Option / paper | Largest position-error growth block | Position error start → end [m] | Local block error [%] |',
        '|---|---:|---:|---:|---|---:|---:|']
    for seq in report['over_twice_paper_sequences']:
        d=sequences[seq];b=d['modes']['candidate']
        for block in b['largest_position_error_growth_blocks']:
            growth.append(f"| {seq} | {f(b['kitti_rte_trans_pct'])} | {f(d['paper_rte_pct'])} | {f(d['candidate_over_paper'])} | {block['first_pose_frame']}–{block['last_pose_frame']} | {f(block['position_error_start_m'])} → {f(block['position_error_end_m'])} | {f(block['block_relative_translation_error_pct'])} |")
    if not report['over_twice_paper_sequences']:growth=['','No candidate sequence exceeds twice the cited paper RTE.']
    growth+=['','These fixed-size motion blocks locate error growth; their relative errors are',
        'not official distance-segment RTE and do not establish the solver-level cause.',
        'Paper values are cited context; this extension is not the paper algorithm.']
    local=['','New local bad labels in the candidate relative to the reference (all frames):','',
        '| Sequence | Newly bad length steps | Reference-only bad length steps | Newly bad vector steps | Reference-only bad vector steps |',
        '|---|---:|---:|---:|---:|']
    for seq,d in sequences.items():
        changes=d['local_label_changes'];length=changes['length'];vector=changes['vector']
        local.append(f"| {seq} | {len(length['candidate_only_frames'])} | {len(length['baseline_only_frames'])} | {len(vector['candidate_only_frames'])} | {len(vector['baseline_only_frames'])} |")
    local+=['','Each pair uses the same input frame and GT labels but different realized',
        'trajectories. These are outcome differences, not matched-seed interventions.',
        'Every affected frame index is retained in the numerical report.']
    prior=[]
    outcome=[*prior,f"Geometric mean of option / reference official RTE: **{f(report['geometric_mean_candidate_over_baseline'])}x** across all sequences.",f'Lower RTE: {lower}. Higher RTE: {higher}. Unchanged RTE: {equal}.',
        'This is a fixed experimental comparison; no per-sequence default is adopted.',
        'Bad-length labels use the inherited local 20% threshold for GT motion at least 0.1 m,',
        'separate from official KITTI 100–800 m RTE. Raw paired timing is not an FPS benchmark.',
        '',f'Numerical sources committed before prose: `{commit}`.', '',*table,'',
        'Every adverse comparison (reference → option):','',*(adverse or ['No adverse values in the declared metric set.']),
        *local,'', 'Failure-sequence observations along each realized trajectory:', '',*failures,*growth,'',
        'These are natural full-run observations, not matched-seed causal interventions.',
        'The complete report retains every step and all motion-error blocks. The option',
        'remains outside the paper and default off; earlier results are preserved.']
    method=W/'papers/inten_loam/README.md';text=method.read_text();marker='## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)'
    if '<!-- INTEN_TRANSLATION_PRIOR -->' in text or text.count(marker)!=1:raise ValueError('Unexpected method insertion point')
    intro=['<!-- INTEN_TRANSLATION_PRIOR -->','## Optional weak-translation prediction prior','',
        '`--inten-loam-paper-set s2s_translation_prior_ratio=0.03` is a repository',
        'extension **outside the paper, off by default** (default ratio zero).',
        'At the first S2S iteration, the data Hessian is expressed at the sensor',
        'origin and rotation is marginalized. The prior adds only the information',
        'needed to raise weak translation eigenvalues to the declared fraction',
        'of the strongest. Its quadratic position residual is anchored to the',
        'initial predicted translation; its information stays fixed during the solve.',
        'It may also resist real acceleration or preserve an incorrect prediction.',
        'Data associations, feature weights, iteration budgets, S2M and the original',
        'damped solve remain unchanged. Trace systems/cost include the added prior;',
        'feature counts and forced diagnostic class equations remain data-only.',
        '', 'Both arms enable the preceding rigid mapped-motion seed and acceleration;',
        'row completion stays off. The new ratio alone differs: zero versus 0.03.',
        'The range/elevation/plane/edge profile was selected on 07, with 00 informing',
        'earlier diagnosis. The fraction was borrowed from the existing',
        '[DiLO KITTI01-tuned control](../dilo/README.md) and fixed before any new',
        'InTEn accuracy run. Here it is a soft information floor after rotation',
        'marginalization, whereas DiLO hard-remaps the raw translation block.',
        'It was not retuned on a new InTEn sequence score. Known 01/02/10 failures',
        'and prior whole-sequence outcomes are',
        'development evidence, so there is no independent held-out claim.',
        'Every sequence uses the same settings, continuous prediction and no GT seed.',
        '', '[Full study and commands](../../experiments/results/inten_translation_prior/README.md);',
        '[complete numerical report](../../experiments/results/inten_translation_prior/report/summary.json).','']
    section='\n'.join(intro+outcome+['<!-- /INTEN_TRANSLATION_PRIOR -->'])+'\n\n'
    method.write_text(text.replace(marker,section+marker))
    entries=['','## Known diagnostic entries along natural runs','',
        '| Sequence / frame | Mode | Input length / GT | S2S length / GT | Final length / GT | First ground pairs | First facade pairs |',
        '|---|---|---:|---:|---:|---:|---:|']
    for seq in ('01','02'):
        for mode in ('baseline','candidate'):
            for row in sequences[seq]['modes'][mode]['known_entry_observations']:
                entries.append(f"| {seq} / {row['frame']} | {mode} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_first_ground_pairs']} | {row['s2s_first_facade_pairs']} |")
    information=read(E/'information.json');committed(E/'information.json')
    entries+=['','## Inherited first-system control before new accuracy runs','',
        'All inherited GT-seeded first systems are development controls, not trajectories.',
        'The same fixed ratio reduces the absolute first-forward correction in',
        ', '.join(f"{seq}: {d['smaller_absolute_first_forward']} of {d['moving']} moving pairs" for seq,d in information['sequences'].items())+'.',
        'Every unchanged/adverse case and its eigenvalues are retained in information.json.',
        'These controls alone do not establish recovery by iterative registration.']
    entries+=['','The inherited oracle entry controls use exactly the same first data system:', '',
        '| Oracle case | Smallest / largest translation information | Weak dimensions | Forward in weak subspace | First forward correction [%] | With soft prior [%] |',
        '|---|---:|---:|---:|---:|---:|']
    for seq,frame in (('01',420),('02',1876),('02',4460)):
        item=next(c for c in information['sequences'][seq]['cases_detail'] if c['frame']==frame)
        entries.append(f"| {seq} / {frame} | {f(item['smallest_over_largest'])} | {item['weak_dimensions']} | {f(item['forward_in_weak_subspace'])} | {f(100*item['first_forward_fraction'])} | {f(100*item['prior_first_forward_fraction'])} |")
    entries+=['','01 / 420 is an inherited case from the earlier reference, not the remaining',
        'collapse interval of the current mapped reference (shown above). At this',
        'selected control the criterion supplies no extra information. The',
        '02 controls retain a backward correction even with the soft prior; these',
        'controls neither establish a normal-bias remedy nor predict the full-run outcome.']
    entries+=['','## Active prior and native solve checks','',
        '| Sequence | Frames with active prior | Recorded S2S systems checked | Maximum prior-information reconstruction error | Maximum relative damped-system residual |',
        '|---|---:|---:|---:|---:|']
    for seq,d in sequences.items():
        b=d['modes']['candidate']
        entries.append(f"| {seq} | {b['active_translation_prior_frames']} | {b['translation_prior_system_checks']} | {b['max_prior_information_reconstruction_error']:.6g} | {b['max_relative_linear_solve_residual']:.6g} |")
    entries+=['','The stored information is independently reconstructed from the first',
        'data Hessian after subtracting the recorded prior. Every recorded finite',
        'update is checked against its total damped H/g using relative backward error.',
        'Independent LAPACK/Eigen step differences are retained without treating',
        'poorly conditioned world-coordinate forward errors as implementation drift.',
        'This checks the declared',
        'implementation, not the correctness of its correspondences.']
    entries+=['','The candidate changes the preceding trajectory as well as the current seed.',
        'Support counts alone do not establish the cause of a change.', '',
        '![Official full-sequence RTE comparison](report/official_rte.png)']
    if report['parent_reference_equal_all_sequences']:entries+=['','Every fresh ratio-zero reference exactly reproduces the preceding mapped-prior candidate RTE.']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected study template')
    study_outcome='\n'.join(outcome+entries).replace('../../experiments/results/inten_mapped_prior/README.md','../inten_mapped_prior/README.md')
    (E/'README.md').write_text(template.replace('@@RESULTS@@',study_outcome))
    print('method/study READMEs generated from committed numerical results')


if __name__=='__main__':main()
