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
    old=W/'experiments/results/inten_mapped_prior'
    for path in (old/'sequences/07/baseline/summary.json',old/'sequences/07/candidate/summary.json',old/'sequences/00/candidate/native_command.json',old/'sequences/00/geometric_diagnosis/summary.json'):committed(path)
    old_reference=read(old/'sequences/07/baseline/summary.json')['methods'][0]
    old_candidate=read(old/'sequences/07/candidate/summary.json')['methods'][0]
    old_failure=read(old/'sequences/00/candidate/native_command.json')
    old_diagnosis=read(old/'sequences/00/geometric_diagnosis/summary.json')
    report=read(E/'report/summary.json');sequences=report['sequences']
    if not report['all_sequences_present'] or len(sequences)!=11:raise ValueError('All full sequences required')
    commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip()
    lower=', '.join(report['lower_rte_sequences']) or 'none';higher=', '.join(report['higher_rte_sequences']) or 'none'
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
    prior=[
        'The initial unprojected trial is a retained negative result: full 07 official',
        f"RTE {f(old_reference['kitti_rte_trans_pct'])} → {f(old_candidate['kitti_rte_trans_pct'])}%,",
        f"ATE {f(old_reference['ate_m'])} → {f(old_candidate['ate_m'])} m;00 aborted with",
        f"exit {old_failure['exit_code']} after {old_diagnosis['completed_frame_records']} frame records",
        '(no full 00 RTE). Its non-rigid rotation feedback informed the fixed rotation',
        'restoration used here. All 11 sequences restart with the revised setting.',
        '[Original source, commands and diagnosis](../../experiments/results/inten_mapped_prior/README.md).','']
    outcome=[*prior,f"Geometric mean of option / reference official RTE: **{f(report['geometric_mean_candidate_over_baseline'])}x** across all sequences.",f'Lower RTE: {lower}. Higher RTE: {higher}.',
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
    if '<!-- INTEN_MAPPED_PRIOR -->' in text or text.count(marker)!=1:raise ValueError('Unexpected method insertion point')
    intro=['<!-- INTEN_MAPPED_PRIOR -->','## Optional recursive mapped-motion S2S seed','',
        '`--inten-loam-paper-set s2s_mapped_prior=1` is a repository extension',
        '**outside the paper, off by default**. It replaces only the normal S2S',
        'initial value with the previous final mapped relative pose, updated recursively.',
        'Its rotation uses a normalized quaternion; translation is preserved.',
        'The initial unprojected trial completed 07 worse and aborted 00; see the',
        '[preserved failure](../../experiments/results/inten_mapped_prior/README.md).',
        '00 informed this numerical correction; this fixed revision restarts every sequence.',
        'The original accepted S2S motion still controls low-count fallback and deskew.',
        'Startup is identity; mapping-off and forced-pose diagnostics are unchanged.',
        'Skipped/rejected S2M uses the actual final relative pose for the next seed.',
        'No association, gate, residual, damping or budget changes; row completion stays off.',
        '', 'The inherited profile was chosen on 07 with 00 influencing diagnosis.',
        'Known 01/02/10 failures motivated this parameter-free boolean, so no independent',
        'held-out claim is made. Every KITTI 00–10 sequence uses the same settings,',
        'acceleration and passive trace, with continuous prediction and no GT seed.',
        'The earlier one-shot mapped-seed control did not test recursive prevention.',
        '', '[Full study and commands](../../experiments/results/inten_mapped_prior_rigid/README.md);',
        '[complete numerical report](../../experiments/results/inten_mapped_prior_rigid/report/summary.json).','']
    section='\n'.join(intro+outcome+['<!-- /INTEN_MAPPED_PRIOR -->'])+'\n\n'
    method.write_text(text.replace(marker,section+marker))
    entries=['','## Known diagnostic entries along natural runs','',
        '| Sequence / frame | Mode | Input length / GT | S2S length / GT | Final length / GT | First ground pairs | First facade pairs |',
        '|---|---|---:|---:|---:|---:|---:|']
    for seq in ('01','02'):
        for mode in ('baseline','candidate'):
            for row in sequences[seq]['modes'][mode]['known_entry_observations']:
                entries.append(f"| {seq} / {row['frame']} | {mode} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_first_ground_pairs']} | {row['s2s_first_facade_pairs']} |")
    entries+=['','The candidate changes the preceding trajectory as well as the current seed.',
        'Support counts alone do not establish the cause of a change.', '',
        '![Official full-sequence RTE comparison](report/official_rte.png)']
    if report['historical_reference_equal_all_sequences']:entries+=['','Every fresh default-off official RTE exactly equals the historical primary aggregate.']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected study template')
    study_outcome='\n'.join(outcome+entries).replace('../../experiments/results/inten_mapped_prior/README.md','../inten_mapped_prior/README.md')
    (E/'README.md').write_text(template.replace('@@RESULTS@@',study_outcome))
    print('method/study READMEs generated from committed numerical results')


if __name__=='__main__':main()
