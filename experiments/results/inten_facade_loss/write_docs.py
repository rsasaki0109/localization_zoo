"""Write all outcomes only from committed full-sequence numerical evidence."""
from pathlib import Path
import json
import gzip
import hashlib
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
    metrics=dict(rpe_trans_pct='RPE [%]',kitti_rte_trans_pct='official RTE [%]',kitti_rte_rot_deg_per_100m='rotation [deg/100 m]',ate_m='ATE [m]',final_bad_length='bad-length steps',final_bad_vector='bad-vector steps')
    adverse=[]
    for seq,keys in report['adverse_metrics'].items():
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        if keys:adverse.append('- '+seq+': '+'; '.join(f'{metrics[k]} {f(a[k])} → {f(b[k])}' for k in keys)+'.')
    failures=[]
    for seq in ('01','02','10'):
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        failures.append(f"- {seq}: final bad-length frames {intervals(a['bad_length_frame_ranges'])} → {intervals(b['bad_length_frame_ranges'])}; raw S2S bad-length outputs {a['s2s_bad_length']} → {b['s2s_bad_length']}; S2S fallbacks {a['s2s_fallback']} → {b['s2s_fallback']}; final rejected S2M {a['s2m_rejected']} → {b['s2m_rejected']}.")
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
    a01,b01=[sequences['01']['modes'][m] for m in ('baseline','candidate')]
    if b01['kitti_rte_trans_pct']>a01['kitti_rte_trans_pct'] and b01['final_bad_length']>=a01['final_bad_length']:
        outcome[:0]=[f"The evaluated S2S facade Cauchy influence does not resolve the current 01 collapse: official RTE changes {f(a01['kitti_rte_trans_pct'])}% → {f(b01['kitti_rte_trans_pct'])}% and bad-length labels {a01['final_bad_length']} → {b01['final_bad_length']}. It is not adopted as a remedy; the existing default remains off.",'']
    terminal=read(E/'analysis/report.json');committed(E/'analysis/report.json')
    entries=['','## Matched-seed loss controls before full evaluation','',
        f"Every one of {terminal['native_calls']} terminal calls completes; all {terminal['pairs']} unchanged outputs and first data-class systems match the archived reference. Every baseline solver iteration and state field also matches, with only the explicitly checked forced diagnostic first-count convention separated from the normal caller last count.",
        'The Cauchy arm keeps every first association/count and non-facade first data equation. Later reassociation and the adaptive S2S prior can change. GT appears only in offline errors/directions. No GT seed, hand region, DOR mask or fitted scale enters either terminal arm. These are terminal solves from identical prefixes, without future map/prediction feedback.',
        'Terminal bad labels refer to the S2S estimate. Full-run table labels refer to the final mapped increment, so their baseline counts need not agree.','',
        '| Reference group | Frames | Length error: lower / higher / equal | Vector error: lower / higher / equal | Rotation error: lower / higher / equal | Remaining bad length / vector | Newly bad length / vector |',
        '|---|---:|---:|---:|---:|---:|---:|']
    for name,g in terminal['summary'].items():
        counts=lambda key:' / '.join(str(g[key][k]) for k in ('lower','higher','equal'))
        entries.append(f"| {name} | {g['frames']} | {counts('length_error')} | {counts('vector_error')} | {counts('rotation_error_mrad')} | {len(g['remaining_bad_length_frames'])} / {len(g['remaining_bad_vector_frames'])} | {len(g['new_bad_length_frames'])} / {len(g['new_bad_vector_frames'])} |")
    entries+=['','| Endpoint | Reference length / GT | Cauchy length / GT | Reference vector error / GT | Cauchy vector error / GT | Reference rotation error [mrad] | Cauchy rotation error [mrad] |',
        '|---|---:|---:|---:|---:|---:|---:|']
    for row in terminal['endpoints']:
        a,b=row['arms']
        entries.append(f"| {row['frame']} | {f(a['length_ratio'])} | {f(b['length_ratio'])} | {f(a['vector_error'])} | {f(b['vector_error'])} | {f(a['rotation_error_mrad'])} | {f(b['rotation_error_mrad'])} |")
    entries+=['',
        f"The unchanged first-query set contains {terminal['residuals']['n']} accepted facades; {terminal['native_huber_inlier_pairs']} are inside the native Huber scale. Median absolute residual is {f(terminal['residuals']['p50'])} m. Median Huber/Cauchy influence is {f(terminal['first_weights']['0']['p50'])}/{f(terminal['first_weights']['1']['p50'])}. Cauchy changes weights inside the Huber inlier region too; residual magnitude is not a static/moving or correct-correspondence label.",
        f"Independent reconstruction of every first facade H/g has maximum relative error {terminal['first_facade_reweighting_reconstruction_max_relative_error']:.3e}; maximum recorded native linear-solve backward error is {terminal['max_native_linear_solve_backward_error']:.3e}. Weighted squared residual is the native IRLS surrogate, not a robust loss energy comparable across the two loss definitions.",'',
        '| Current01 frame204 cluster | Accepted pairs | Reference joint first forward contribution [% of GT step] | Cauchy weights through fixed reference system | Cauchy weights through its own joint system |',
        '|---|---:|---:|---:|---:|']
    committed(E/'analysis/archive.json')
    metadata=read(E/'analysis/archive.json')['files']['cluster_influence.jsonl'];decoded=[]
    for part in metadata['parts']:
        path=E/'analysis'/part['path'];committed(path)
        if hashlib.sha256(path.read_bytes()).hexdigest()!=part['gzip_sha256']:raise ValueError('Cluster influence archive changed')
        decoded.append(gzip.decompress(path.read_bytes()))
    raw=b''.join(decoded)
    if hashlib.sha256(raw).hexdigest()!=metadata['sha256']:raise ValueError('Cluster influence roundtrip failed')
    rows={r['cluster']:r for r in map(json.loads,raw.splitlines()) if r['frame']==204}
    for cluster in (600,249,506):
        row=rows[cluster];a,b=row['arms']
        entries.append(f"| {cluster} | {row['pairs']} | {f(a['own_system_forward_percent'])} | {f(b['fixed_reference_system_forward_percent'])} | {f(b['own_system_forward_percent'])} |")
    entries+=['','These are inherited onset examples, not selected detector rules. The fixed reference inverse isolates the algebraic weight change; the alternative inverse also includes changed information and adaptive prior. Conditional first-system contributions are not independent nonlinear interventions or physical-motion truth. Every cluster and every pair, including adverse and neutral outcomes, is retained. The improved204 and adverse247 controls motivated an unchanged-scale recursive trial; they did not establish recovery.','',
        '## Full-run error boundaries','',
        '| Sequence | Arm | Bad-interval start | S2S input / GT | S2S output / GT | Final step / GT | First accepted facades |',
        '|---|---|---:|---:|---:|---:|---:|']
    for seq in sorted(set(report['over_twice_paper_sequences'])|{'01','02','10'}):
        for mode in ('baseline','candidate'):
            data=sequences[seq]['modes'][mode];starts={a for a,b in data['bad_length_frame_ranges']}
            for row in data.get('bad_interval_boundary_observations',[]):
                if row['frame'] not in starts:continue
                entries.append(f"| {seq} | {mode} | {row['frame']} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_first_facade_pairs']} |")
    entries+=['','Every bad-interval boundary and adjacent frame is retained in the numerical report; every native step is in its CSV. These full-run stages use different realized histories and are not matched-history causal interventions. Fixed-size error-growth blocks above also locate gradual drift without a local threshold crossing.','',
        '![Full official RTE comparison](report/official_rte.png)']
    for seq in report['over_twice_paper_sequences']:
        entries+=['',f'![KITTI {seq} full position error and final step length](report/seq_{seq}_error_growth.png)']
    entries+=['','Position-error curves normalize the first pose and are distinct from globally aligned ATE. Step ratios include only the declared moving frames; dashed lines show the same local20% label boundaries, not official KITTI distance-segment RTE.']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected template')
    (E/'README.md').write_text(template.replace('@@RESULTS@@','\n'.join(outcome+entries)))
    method=W/'papers/inten_loam/README.md';text=method.read_text()
    if '<!-- INTEN_FACADE_LOSS -->' in text:raise ValueError('Section already exists')
    intro=['','<!-- INTEN_FACADE_LOSS -->','## S2S facade Cauchy influence: one fixed full00–10 comparison','',
        '`--inten-loam-paper-set s2s_facade_cauchy=1` is outside the paper and default off. It replaces only S2S facade Huber IRLS influence by Cauchy at the same inherited native scale0.1. It changes no extraction or association gate. Other classes and S2M retain Huber definitions; changed first information also changes the adaptive prior, and later trajectory/map feedback can affect all classes.','',
        'One Boolean at the original scale was selected after committed current01 matched-seed controls f52b84a0, before all full runs. Every adverse case is retained, including247 and204 rotation error. No scale fitting, sweep, GT seed/input, hand region or per-sequence winner. Original07 profile selection,00 diagnosis, inherited DiLO01 prior tuning and all prior KITTI development exposure are disclosed; no independent held-out claim. Both full arms use the same new binary, with mapped seed/prior0.03/ground-facadePCA/acceleration on, predictedDOR/spread/row completion off.','',
        '[All commands, phase-specific sources, hashes, failures and full results](../../experiments/results/inten_facade_loss/README.md).','']
    method.write_text(text+'\n'.join(intro+outcome+[line.replace('(report/','(../../experiments/results/inten_facade_loss/report/') for line in entries]+['<!-- /INTEN_FACADE_LOSS -->'])+'\n')
    print('Every full outcome and negative terminal control written from committed evidence')


if __name__=='__main__':main()
