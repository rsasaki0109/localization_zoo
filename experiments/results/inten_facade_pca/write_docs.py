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
    method=W/'papers/inten_loam/README.md';text=method.read_text();marker='## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)'
    if '<!-- INTEN_FACADE_PCA -->' in text or text.count(marker)!=1:raise ValueError('Unexpected method insertion point')
    intro=['<!-- INTEN_FACADE_PCA -->',
     '## Optional S2S facade PCA normals', '',
     '`--inten-loam-paper-set s2s_facade_pca=1` enables an outside-paper, default-off',
     'extension. For already accepted facade triangles only, use the existing',
     'ground-PCA normal construction on exactly the same gated ten neighbours.',
     'Keep the original nearest-point anchor; insufficient or numerically',
     'degenerate PCA keeps the native triangle normal. No support, acceptance,',
     'gate, feature, iteration-budget, S2M or update rule changes. At a fixed pose,',
     'only facade data equations change. The unchanged adaptive soft-prior rule',
     'can change its information immediately; later associations/maps diverge.',
     'Ground PCA, mapped-motion seed, soft prior0.03 and acceleration are on in',
     'both full arms; row completion stays off. Uniform elevation bins remain',
     'unchanged and are not measured physical rings.', '',
     'The original profile was chosen on07;00 informed diagnosis. Prior fraction',
     'came from DiLO KITTI01 tuning. This candidate was selected after committed',
     'current01 onset controls reproduced the continuous native S2S output and',
     'showed facade backward first-update contributions with inactive prior.',
     'Oracle initialization changed the onset result. These observations motivate',
     'a hypothesis, not a proof of wrong wall normals or an isolated S2M cause.',
     'The PCA construction/tolerance is inherited unchanged; no new numeric',
     'parameter was fitted. Current01 and every earlier KITTI outcome are',
     'development data; no independent held-out claim. One setting is frozen',
     'before all full runs and every outcome is retained without retuning.', '',
     '[Full study and execution commands](../../experiments/results/inten_facade_pca/README.md);',
     '[complete numerical report](../../experiments/results/inten_facade_pca/report/summary.json).', '']
    section='\n'.join(intro+outcome+['<!-- /INTEN_FACADE_PCA -->'])+'\n\n'
    method.write_text(text.replace(marker,section+marker))
    entries=['','## Known diagnostic entries along natural runs','',
        '| Sequence / frame | Mode | Input length / GT | S2S length / GT | Final length / GT | First ground pairs | First facade pairs | Data forward information | Prior forward information |',
        '|---|---|---:|---:|---:|---:|---:|---:|---:|']
    for seq in ('01','02'):
        for mode in ('baseline','candidate'):
            for row in sequences[seq]['modes'][mode]['known_entry_observations']:
                entries.append(f"| {seq} / {row['frame']} | {mode} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_first_ground_pairs']} | {row['s2s_first_facade_pairs']} | {f(row['s2s_first_data_forward_information'])} | {f(row['s2s_first_prior_forward_information'])} |")
    entries+=['','Forward information projects the first sensor-centered data Schur matrix',
        'and fixed prior information along the GT motion direction. GT is used only',
        'in offline diagnosis, never as an estimator input. Different realized',
        'trajectories make these descriptive observations, not matched-seed causal',
        'comparisons. The soft-prior rule is identical while its information may',
        'change immediately with the facade equations.']
    boundary=read(E/'boundary/report.json');matched=read(E/'sequences/01/current_boundary_intervention/result.json')
    committed(E/'boundary/report.json');committed(E/'sequences/01/current_boundary_intervention/result.json')
    entries+=['','## Matched current01 onset controls','',
        'The public forced S2S diagnostic uses the actual saved current-profile',
        'initial poses and separately GT-oracle poses. It repeats native association',
        'through the entire S2S solve. Its mapping-disabled state is deliberate:',
        'this is a same-pair S2S control, not full-sequence accuracy or a replay',
        'of S2M history. GT is never fed to the full estimator.', '',
        '| Initial seed | Pairs | Lower absolute length error | Higher absolute length error | Unchanged |',
        '|---|---:|---:|---:|---:|']
    for mode in ('current_full_natural','ground_truth_oracle'):
        rows=[r for r in matched['cases'] if r['mode']==mode]
        entries.append(f"| {mode} | {len(rows)} | {sum(r['absolute_length_error_change']<0 for r in rows)} | {sum(r['absolute_length_error_change']>0 for r in rows)} | {sum(r['absolute_length_error_change']==0 for r in rows)} |")
    entries+=['','All original-profile outputs and class equations match exactly in the',
        'new facade-off probe. All fixed-pose class counts and every non-facade',
        'equation match between arms. All pairs, including adverse ones, are in',
        '`sequences/01/current_boundary_intervention/result.json`.', '',
        '| Frame | Seed | Reference output length / GT | Facade-PCA output length / GT |',
        '|---|---|---:|---:|']
    for row in matched['cases']:
        if row['frame'] in (200,203,204,217,253,420,433):entries.append(f"| {row['frame']} | {row['mode']} | {f(row['baseline_length_ratio'])} | {f(row['candidate_length_ratio'])} |")
    onset=[r for r in matched['cases'] if r['frame']==204]
    entries+=['','At the current01 onset pair, the matched-initialization results are:','']
    for row in onset:
        direction='higher' if row['absolute_length_error_change']>0 else ('lower' if row['absolute_length_error_change']<0 else 'unchanged')
        entries.append(f"- {row['mode']}: length / GT {f(row['baseline_length_ratio'])} → {f(row['candidate_length_ratio'])}; absolute length error changes by {f(row['absolute_length_error_change'])} ({direction}).")
    entries+=['','These controls retain the actual repeated-association S2S solve. Full-run',
        'RTE also includes earlier seed and map changes; an aggregate improvement',
        'does not establish that facade PCA repairs the onset correspondence or',
        'that a first-system class contribution explains the later collapse.']
    entries+=['','Baseline initial first-order forward-correction contributions as percent of the GT',
        'step, through the joint damped system (classes are additive gradients,',
        'not separately re-associated interventions):', '',
        '| Frame | Seed | Edge | Ground | Facade | Intensity | Total |',
        '|---|---|---:|---:|---:|---:|---:|']
    for row in boundary['cases']:
        if row['frame'] in (200,203,204,420,433):entries.append('| '+str(row['frame'])+' | '+row['mode']+' | '+' | '.join(f(x) for x in row['class_first_forward_percent'])+' | '+f(row['first_forward_percent'])+' |')
    entries+=['','The full native onset S2S output matches the forced control exactly.',
        'Class-wise H/g summation differs from serial totals only at retained',
        'roundoff; numerical maxima and all source/binary hashes are in',
        '`boundary/report.json`. The seed intervention distinguishes optimizer',
        'basins at that pair. It does not identify the physical target surface',
        'or establish that the first facade gradient explains later collapse.']
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
    if report['parent_reference_equal_all_sequences']:entries+=['','Every fresh facade-PCA-off reference exactly reproduces the preceding ground-PCA candidate RTE.']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected study template')
    study_outcome='\n'.join(outcome+entries).replace('../../experiments/results/inten_mapped_prior/README.md','../inten_mapped_prior/README.md')
    (E/'README.md').write_text(template.replace('@@RESULTS@@',study_outcome))
    print('method/study READMEs generated from committed numerical results')


if __name__=='__main__':main()
