"""Write the study and method section only from committed full-run results."""
from pathlib import Path
import json
import subprocess

E=Path(__file__).resolve().parent
W=E.parents[2]


def read(p):return json.loads(p.read_text())
def committed(path):
    name=str(path.relative_to(W))
    if subprocess.check_output(['git','show','HEAD:'+name],cwd=W)!=path.read_bytes():
        raise ValueError('Numerical source must be committed: '+name)
def f(value):return f'{value:.6f}'
def intervals(ranges):return ', '.join(str(a) if a==b else f'{a}–{b}' for a,b in ranges) or 'none'


def main():
    committed(Path(__file__).resolve());committed(E/'postprocessing_protocol.json');committed(E/'readme_template.md')
    committed(E/'report/summary.json')
    report=read(E/'report/summary.json');sequences=report['sequences']
    if not report['all_sequences_present'] or len(sequences)!=11:raise ValueError('Full comparison required')
    evidence_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip()
    lines=[
        '<!-- INTEN_ROW_SUPPORT -->',
        '## Optional S2S missing-row completion',
        '',
        '`--inten-loam-paper-set s2s_row_support=1` enables a repository extension',
        '**outside the paper, off by default**. For ground and facade planes, the',
        'original ten-neighbour query still chooses the closest point and any',
        'available same-row / adjacent-row support. Only a missing support point is',
        'completed from the same retained feature cloud, using nearest eligible',
        'points under the original distance gate and row gaps. Existing choices,',
        'triangle checks, residuals, solver budgets and S2M remain unchanged.',
        'Rows are elevation bins inferred by the extractor, not measured laser rings.',
        'No new points, PCA model or ground-truth feedback are introduced.',
        '',
        'The [full fixed-setting experiment](../../experiments/results/inten_row_support/README.md)',
        'uses the inherited 07-selected profile on every KITTI 00–10 sequence.',
        '00 influenced earlier diagnosis, and known 01/02 failures motivated this',
        'boolean option; these are development data. The preceding diagnosis also',
        'inspected 10; this study makes no independent held-out claim. Full 07 ran first, followed',
        'by every remaining sequence without retuning. No numerical threshold was',
        'selected anew. Existing acceleration is enabled identically in both arms;',
        'both use passive traces, continuous natural prediction and `--no-gt-seed`.',
        f'Committed numerical evidence: `{evidence_commit[:8]}`, [all metrics and steps](../../experiments/results/inten_row_support/report/summary.json).',
        '',
        '| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |',
        '|---|---:|---:|---:|---:|---:|',
    ]
    for seq,d in sequences.items():
        a,b=d['modes']['baseline'],d['modes']['candidate']
        lines.append(f"| {seq} | {f(a['kitti_rte_trans_pct'])} | {f(b['kitti_rte_trans_pct'])} | {f(a['ate_m'])} | {f(b['ate_m'])} | {a['final_bad_length']} / {b['final_bad_length']} |")
    lines += ['',f"The geometric mean of option / reference official RTE is **{f(report['geometric_mean_candidate_over_baseline'])}x** across all sequences.",
        'This is an experimental paired comparison, not a replacement paper ratio.',
        'Bad-length labels mean a local step-length error above 20% with GT motion',
        'at least 0.1 m; they are distinct from official KITTI 100–800 m RTE.',
        '', 'Every adverse comparison is retained (reference → option):', '']
    metric_labels=dict(kitti_rte_trans_pct='official RTE [%]',kitti_rte_rot_deg_per_100m='rotation [deg/100 m]',
        ate_m='ATE [m]',final_bad_length='bad-length steps',final_bad_vector='bad-vector steps')
    for seq,metrics in report['adverse_metrics'].items():
        if not metrics:continue
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        values=[]
        for key in metrics:
            v=lambda x:str(x) if isinstance(x,int) else f(x)
            values.append(f"{metric_labels[key]} {v(a[key])} → {v(b[key])}")
        lines.append(f"- {seq}: "+'; '.join(values)+'.')
    lines += ['', 'Full failure-sequence observations:', '']
    for seq in ('01','02','10'):
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        lines.append(f"- {seq}: final bad-length frames {intervals(a['bad_length_frame_ranges'])} → {intervals(b['bad_length_frame_ranges'])}; S2S bad-length outputs {a['s2s_bad_length']} → {b['s2s_bad_length']}. These are each run’s realized trajectory, not matched-seed interventions.")
    lines += ['',
        'More support does not establish correct surfaces or resolve the highway',
        'failure. The option remains experimental and is not promoted per sequence.',
        'All earlier aggregates, published paper values and defaults are preserved.',
        'Raw timings are retained, but concurrent runners are not a standalone FPS',
        'benchmark. Full 07 legacy-off compatibility and all inherited default-off',
        'pair controls, native tests and command/execution records are linked in',
        'the study. Sensor-transfer behavior and the remaining solver failure are open.',
        '<!-- /INTEN_ROW_SUPPORT -->',
    ]
    method=W/'papers/inten_loam/README.md';text=method.read_text()
    if '<!-- INTEN_ROW_SUPPORT -->' in text:raise ValueError('Refusing repeated section insertion')
    marker='## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)'
    if text.count(marker)!=1:raise ValueError('Unexpected method section')
    text=text.replace('row-aware matcher and full-sequence improvement remain to be evaluated.',
        'row-aware matcher was subsequently evaluated on every full sequence; see\n'
        '[the optional missing-row completion results](#optional-s2s-missing-row-completion).')
    method.write_text(text.replace(marker,'\n'.join(lines)+'\n\n'+marker))
    table_start=next(i for i,line in enumerate(lines) if line.startswith('| Sequence | Reference RTE'))
    table_end=next(i for i in range(table_start,len(lines)) if not lines[i])
    results=['The geometric mean of candidate / reference official RTE is '
        +f"**{f(report['geometric_mean_candidate_over_baseline'])}x** across all full sequences.",
        'Lower RTE: '+', '.join(report['lower_rte_sequences'])+'. Higher RTE: '+', '.join(report['higher_rte_sequences'])+'.',
        'The highway/corridor failure is not resolved. All adverse metrics and every',
        'step remain in [the report](report/summary.json) and [comparison CSV](report/comparison.csv).',
        'The extension is not adopted as a general accuracy fix or promoted per sequence.',
        '',f'Numerical sources were committed at `{evidence_commit}` before this prose.',
        '',*lines[table_start:table_end],
        '', 'Bad-length labels inherit the local 20% threshold at GT motion at least 0.1 m.',
        'They are distinct from official 100–800 m segment RTE.',
        '', '![Full fixed-setting RTE comparison](report/official_rte.png)', '',
        'All adverse comparisons (reference → option):','']
    begin=lines.index('Every adverse comparison is retained (reference → option):')+2
    end=lines.index('Full failure-sequence observations:')-1
    results+=lines[begin:end]
    results+=['','## Natural observations at the known diagnostic entries','',
        'These are observations along each complete run, not GT-seeded or matched-seed',
        'interventions. The row option changes the prior trajectory as well as the',
        'current solve. Support counts alone cannot establish the cause of a change.','',
        '| Sequence / frame | Mode | S2S input length / GT | S2S output length / GT | Final length / GT | First ground pairs | First facade pairs |',
        '|---|---|---:|---:|---:|---:|---:|']
    for seq in ('01','02'):
        for mode in ('baseline','candidate'):
            for row in sequences[seq]['modes'][mode]['known_entry_observations']:
                results.append(f"| {seq} / {row['frame']} | {mode} | {f(row['s2s_input_length_ratio'])} | {f(row['s2s_length_ratio'])} | {f(row['final_length_ratio'])} | {row['s2s_first_ground_pairs']} | {row['s2s_first_facade_pairs']} |")
    results+=['','Every step, full error-growth blocks, all class support, stage stops and failure',
        'intervals are retained in the report and per-sequence motion diagnostics.']
    if report['historical_reference_equal_all_sequences']:
        results+=['Every fresh option-off reference official RTE exactly equals the existing',
            'fixed paper-comparison aggregate for the same sequence.']
    template=(E/'readme_template.md').read_text()
    if template.count('@@RESULTS@@')!=1:raise ValueError('Unexpected study template')
    (E/'README.md').write_text(template.replace('@@RESULTS@@','\n'.join(results)))
    print('method and study READMEs written from committed evidence')


if __name__=='__main__':main()
