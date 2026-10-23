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
        outcome[:0]=[f"The evaluated predicted-DOR facade gate does not resolve the current 01 collapse: official RTE changes {f(a01['kitti_rte_trans_pct'])}% → {f(b01['kitti_rte_trans_pct'])}% and bad-length labels {a01['final_bad_length']} → {b01['final_bad_length']}. It is not adopted as a remedy; the existing default remains off.",'']
    passive=read(E/'baseline/report.json');terminal=read(E/'terminal/report.json')
    committed(E/'baseline/report.json');committed(E/'terminal/report.json')
    entries=['','## Detector and terminal controls','',
        f"The unmodified reference traverses all {passive['normal_full_frames']} frames. Every unanchored estimator/solver/count/DOR field matches; the display-anchor and scan-path conventions are declared separately. Detailed range samples independently reproduce native covered/flagged counts.",
        '', '| Endpoint | First accepted facades | Source prediction mask | Source final mask | Any previous-final-masked target support |',
        '|---|---:|---:|---:|---:|']
    for row in passive['endpoints']:
        g=row['accepted_facade_groups']
        count=lambda key:g.get(key,{}).get('pairs',0)
        entries.append(f"| {row['frame']} | {count('all')} | {count('source_prediction_prior_local')} | {count('source_final')} | {count('target_previous_final_any_support')} |")
    entries+=['','These are native detector labels on accepted first-iteration queries, not motion precision/recall or semantic-object truth. Stage0 prediction/prior-local support is available before S2S. Stage1 uses the same prediction with the later current local map and is retrospective only. Stage2 uses the final pose/current local map. Map and pose effects must not be conflated.','',
        '| Reference frame group | Frames | Accepted first-iteration facades | Marked at prediction/prior map | Extracted facades in clusters with insufficient render coverage |',
        '|---|---:|---:|---:|---:|']
    for group,d in passive['summary'].items():
        stage=d['stages'][0]
        entries.append(f"| {group} | {d['frames']} | {stage['accepted_facade_pairs']} | {stage['source_masked_pairs']} | {stage['cannot_reach_threshold_facade_features']} |")
    entries+=['','The last column counts extracted features over frames, not accepted pairs or unique physical points. Because native DOR divides flagged points by all cluster points, render coverage below the existing object threshold prevents that cluster from being marked. Adequate coverage alone does not ensure the radial threshold flags most points.','',
        '| Current01 frame 204 cluster | Accepted facade pairs | Conditional joint first forward update [% of GT step] | Prediction/prior map: total / covered / flagged / dynamic | Same prediction/current map | Final pose/same current map |',
        '|---|---:|---:|---|---|---|']
    committed(E/'baseline/archive.json')
    archive=read(E/'baseline/archive.json')['files']['cluster_contributions.jsonl']
    decoded=[]
    for part in archive['parts']:
        path=E/'baseline'/part['path'];committed(path)
        if hashlib.sha256(path.read_bytes()).hexdigest()!=part['gzip_sha256']:raise ValueError('Cluster archive hash changed')
        decoded.append(gzip.decompress(path.read_bytes()))
    raw=b''.join(decoded)
    if hashlib.sha256(raw).hexdigest()!=archive['sha256']:raise ValueError('Cluster archive roundtrip changed')
    clusters={r['cluster']:r for r in map(json.loads,raw.splitlines()) if r['frame']==204}
    for cluster in (600,249,506):
        row=clusters[cluster]
        counts=lambda stage:' / '.join(str(v) for v in row['masks'][stage][1:5])
        entries.append(f"| {cluster} | {row['pairs']} | {f(row['first_forward_percent'])} | {counts(0)} | {counts(1)} | {counts(2)} |")
    entries+=['','These committed onset examples distinguish inadequate coverage, weak radial-majority response despite coverage, and pose-feedback changes. The conditional update uses the unchanged joint damped first system; it is not an isolated re-solve or a motion label. Comparing the last two columns holds the rendered local map fixed. No coverage denominator or detector threshold change is selected from these observations.','',
        '| Endpoint | Baseline terminal S2S length / GT | Source prediction gate | Previous-final target filter | Baseline vector error / GT | Source gate vector error / GT |',
        '|---|---:|---:|---:|---:|---:|']
    for row in terminal['endpoints']:
        a,b,c=row['arms']
        entries.append(f"| {row['frame']} | {f(a['length_ratio'])} | {f(b['length_ratio'])} | {f(c['length_ratio'])} | {f(a['vector_error'])} | {f(b['vector_error'])} |")
    entries+=['',f"Every one of {terminal['native_calls']} terminal calls completed successfully. All unchanged natural-seed outputs and non-facade initial data equations match. Source/target masks are the original detector decisions from identical prefixes; no GT or hand box enters either arm.",'',
        '| Group | Arm | Length error: lower / higher / equal | Vector error: lower / higher / equal |',
        '|---|---|---:|---:|']
    for group,arms in terminal['summary'].items():
        for mode,d in arms.items():
            counts=lambda metric:' / '.join(str(d[metric][k]) for k in ('lower','higher','equal'))
            entries.append(f"| {group} | {('Source prediction gate' if mode=='1' else 'Previous-final target filter')} | {counts('length_error')} | {counts('vector_error')} |")
    entries+=['','All adverse and neutral terminal cases are retained. The source gate was selected after these committed development controls, before all full runs. A terminal repair does not include future map/prediction feedback. Target filtering is a diagnostic comparator and is not the implemented option. Ground truth appears only in offline errors.','',
        '## Where full-run errors grow','',
        '| Sequence | Arm | Boundary frame | S2S input / GT | S2S output / GT | Final step / GT | Raw masked facade features |',
        '|---|---|---:|---:|---:|---:|---:|']
    for seq in sorted(set(report['over_twice_paper_sequences'])|{'01','02','10'}):
        for mode in ('baseline','candidate'):
            data=sequences[seq]['modes'][mode]
            starts={a for a,b in data['bad_length_frame_ranges']}
            for row in data.get('bad_interval_boundary_observations',[]):
                if row['frame'] not in starts:continue
                entries.append(f"| {seq} | {mode} | {row['frame']} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_prediction_dor_masked_facades']} |")
    entries+=['','Every bad-interval boundary and adjacent frame is retained in the report, and every native step is retained in the CSVs. These stage errors are along different realized trajectories, not matched-history class ablations. The fixed-size growth blocks above also cover gradual drift without a local threshold crossing.','',
        '![Full official RTE comparison](report/official_rte.png)']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected template')
    (E/'README.md').write_text(template.replace('@@RESULTS@@','\n'.join(outcome+entries)))
    method=W/'papers/inten_loam/README.md';text=method.read_text()
    if '<!-- INTEN_PREDICTION_DOR -->' in text:raise ValueError('Section already exists')
    intro=['','<!-- INTEN_PREDICTION_DOR -->','## Predicted-DOR facade gating: one fixed full 00–10 comparison','',
        '`--inten-loam-paper-set s2s_predicted_dor_facade=1` is an outside-paper extension, default off. It runs existing DOR at the initial motion prediction with the prior local map, then gates only source facade S2S queries. Raw feature extraction, other query classes and S2M/post-S2M insertion rules are preserved at fixed history; later trajectory/map feedback can change all classes. Mapping and DOR must be on; forced diagnostics remain unchanged.','',
        'The Boolean was chosen on committed current 01 full detector and terminal controls. Existing DOR thresholds 0.1/0.5 were not tuned. Original 07 profile selection,00 diagnosis, inherited DiLO 01 prior tuning and all prior KITTI development exposure are disclosed; no independent held-out claim or per-sequence adoption. Both full arms use the same new native binary and retain mapped prediction, S2S prior 0.03, ground/facade PCA and acceleration. Spread/row completion stay off.','',
        '[All protocols, commands/cwd/environment, source/library/binary hashes, failures, local cases and full data](../../experiments/results/inten_prediction_dor/README.md).','']
    method.write_text(text+'\n'.join(intro+outcome+[line.replace('(report/official_rte.png)','(../../experiments/results/inten_prediction_dor/report/official_rte.png)') for line in entries]+['<!-- /INTEN_PREDICTION_DOR -->'])+'\n')
    print('Full outcomes and all negative controls written from committed evidence')

if __name__=='__main__':main()
