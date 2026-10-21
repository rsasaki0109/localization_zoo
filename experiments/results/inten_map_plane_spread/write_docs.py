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
        outcome[:0]=[f"The evaluated fixed plane-spread check does not resolve the current01 collapse: official RTE changes {f(a01['kitti_rte_trans_pct'])}% → {f(b01['kitti_rte_trans_pct'])}% and bad-length labels {a01['final_bad_length']} → {b01['final_bad_length']}. It is not adopted as a remedy; the existing default remains off.",'']
    boundary=read(E/'boundary/report.json');matched=read(E/'spread/report.json')
    committed(E/'boundary/report.json');committed(E/'spread/report.json')
    entries=['','Matched current01 S2M controls (identical normal map prefix and S2M input; terminal endpoints only):','',
       '| Frame | Input length / GT | Native output / GT | Spread output / GT | Absolute length error change | Ground pairs, native / spread | Facade pairs, native / spread |',
       '|---|---:|---:|---:|---:|---:|---:|']
    for row in matched['cases']:
        entries.append(f"| {row['frame']} | {f(row['input_length_ratio'])} | {f(row['baseline_length_ratio'])} | {f(row['candidate_length_ratio'])} | {f(row['absolute_length_error_change'])} | {row['baseline_class_pairs'][1]} / {row['candidate_class_pairs'][1]} | {row['baseline_class_pairs'][2]} / {row['candidate_class_pairs'][2]} |")
    entries+=['',f"Matched absolute length error: {matched['lower_absolute_length_error']} lower, {matched['higher_absolute_length_error']} higher, {matched['equal_absolute_length_error']} equal.",
        'These controls use a separately instrumented diagnostic producer whose',
        'only native expression change is the stage-local plane-spread parameter.',
        'Every normal prefix and baseline solver/output/system matches immutable',
        'native controls. Full accuracy uses the unchanged original runner and',
        'its actual existing CLI option, never the diagnostic copy.',
        'Rejected support is not proof of physically incorrect correspondences.',
        'Matched endpoint changes do not include future trajectory/map feedback.', '',
        'Native first-system class contributions, percent of the GT step:', '',
        '| Frame | S2M input / GT | S2M output / GT | Edge | Ground | Facade | Intensity | Total | Translation min / max | Hypothetical prior norm |',
        '|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|']
    for row in boundary['cases']:
        entries.append('| '+str(row['frame'])+' | '+f(row['input_length_ratio'])+' | '+f(row['output_length_ratio'])+' | '+' | '.join(f(x) for x in row['class_first_forward_percent'])+' | '+f(row['first_forward_percent'])+' | '+f(row['data_min_over_max'])+' | '+f(row['hypothetical_ratio03_prior_norm'])+' |')
    entries+=['','Class contributions are initial joint-system linear contributions, not',
        'independent reassociated solves. All class H/g, trace iterations,',
        'roundoff maxima and negative endpoints are retained. Rank or support',
        'counts alone do not prove correctness. The hypothetical existing-ratio',
        'prior is offline diagnosis, not an additional evaluated estimator.', '',
        '![Official full-sequence RTE comparison](report/official_rte.png)']
    intro=['<!-- INTEN_MAP_PLANE_SPREAD -->', '## Optional S2M plane-spread check: full uniform evaluation','',
        '`--inten-loam-paper-set s2m_plane_min_spread=0.2` tests the existing',
        'outside-paper plane-spread check; its default remains zero (off).',
        'Five-neighbour plane PCA must have middle/largest eigenvalue at least',
        'the stated ratio, in addition to the existing distance/planarity checks.',
        'The ratio is inherited unchanged from the existing synthetic street',
        'mechanism test, with earlier07 exploratory exposure disclosed. No sweep',
        'or per-sequence option selection is used. Profile selected07;00 informed',
        'diagnosis; inherited S2S prior fraction came from DiLO01 tuning.',
        'Current01 and all earlier KITTI outcomes are development evidence,',
        'with no independent held-out claim.', '',
        'Both full arms retain mapped-motion prediction, S2S soft prior0.03,',
        'ground/facade PCA and acceleration, row completion off. No production',
        'source, feature extraction, solver budget or stored result changes.',
        'Only S2M plane acceptance changes at a fixed pose; later map/seed',
        'feedback also changes S2S outcomes. More restrictive support is not',
        'proof of correspondence truth or a general failure remedy.', '',
        '[Full study and commands](../../experiments/results/inten_map_plane_spread/README.md).','']
    method=W/'papers/inten_loam/README.md';text=method.read_text();marker='## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)'
    if '<!-- INTEN_MAP_PLANE_SPREAD -->' in text or text.count(marker)!=1:raise ValueError('Unexpected method insertion point')
    section='\n'.join(intro+outcome+entries+['<!-- /INTEN_MAP_PLANE_SPREAD -->'])+'\n\n';method.write_text(text.replace(marker,section+marker))
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected study template')
    (E/'README.md').write_text(template.replace('@@RESULTS@@','\n'.join(outcome+entries)))
    print('All full and matched outcomes written from committed evidence')

if __name__=='__main__':main()
