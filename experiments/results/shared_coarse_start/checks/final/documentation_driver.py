from pathlib import Path
import json,subprocess
repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/shared_coarse_start';report=e/'report'
for name in ('summary.json','table.md','diagnostics.md'):
 path=report/name
 assert subprocess.check_output(['git','show','HEAD:'+path.relative_to(repo).as_posix()],cwd=repo)==path.read_bytes()
s=json.loads((report/'summary.json').read_text());assert s['complete'] and not s['missing_sequences'] and len(s['rows'])==11
a=s['audit_preservation_summary'];assert a['cells']==22 and a['drifted_cells']==0 and all(v==0 for k,v in a.items() if k.startswith('max_abs'))
raw_checks=s['raw_control_preservation_checks'];assert len(raw_checks)==11 and all(x['official_scan_bin_hashes_match'] and all(v==0 for v in x['absolute_metric_delta'].values()) for x in raw_checks)
frozen=json.loads(subprocess.check_output(['git','show','HEAD:'+str((e/'sequences/04/frozen_comparison.json').relative_to(repo))],cwd=repo))
pose_identity=all(x['saved_pose_bytes_equal'] for x in frozen)
frozen_text='The frozen pre-change full 04 comparison also has identical saved-pose bytes for both methods.' if pose_identity else 'The frozen pre-change full 04 comparison retains printed-metric and saved-pose deltas separately.'
base='../../experiments/results/shared_coarse_start/'
table=(report/'table.md').read_text().strip();diagnostics=(report/'diagnostics.md').read_text().strip()
worse=s['worse_sequences'];negative='RTE is higher on '+', '.join(worse)+'. These negative pairs are retained without reselection.' if worse else 'No sequence has higher RTE at the report threshold; this known-data transfer is not a held-out accuracy claim.'
worse_ate=[row['sequence'] for row in s['rows'] if row['candidate_ate_m']-row['control_ate_m']>1e-6]
if worse_ate:negative+=' ATE is higher on '+', '.join(worse_ate)+'. These tradeoffs remain in the full paired table.'
ranges=['| Seq with bad length steps | Longest coarse bad-length intervals (destination frames) |','|---|---|']
for row in s['rows']:
 if row['candidate_bad_length_steps']==0:continue
 longest=sorted(row['candidate_bad_length_ranges'],key=lambda p:(-(p[1]-p[0]+1),p[0]))[:3]
 text=', '.join(str(p[0]) if p[0]==p[1] else f'{p[0]}–{p[1]}' for p in longest) or 'none above threshold'
 ranges.append(f"| {row['sequence']} | {text} |")
range_table='\n'.join(ranges) if len(ranges)>2 else ''
range_explanation='Intervals above select up to three longest contiguous groups for every sequence with bad length steps, including sequences whose RTE improves.' if len(ranges)>2 else 'No candidate moving step crosses the local length-error threshold.'
target=next(row for row in s['rows'] if row['sequence']=='01')
blocks=target['candidate_largest_error_growth_blocks']
if blocks:
 b=blocks[0]
 target_note=f"Sequence 01 has ATE {target['candidate_ate_m']:.6f} m and {target['candidate_bad_length_steps']}/{target['candidate_moving_steps']} bad length steps. Its largest position-error growth block is {b['first_pose_frame']}–{b['last_pose_frame']}: GT travel {b['gt_distance_m']:.3f} m, position-error increase {b['position_error_increase_m']:.3f} m."
else:target_note=''
block=f'''**Completed fixed-setting experiment, full KITTI 00–10.** All candidates use
`--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1
--suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
unchanged, with translation remapping off. All native notes report one coarse
registration; the iteration budget is a cap and numerical stopping can occur earlier.

{table}

[Full committed report and input snapshot]({base}report/summary.json),
[exact report command and checked exit]({base}report/command.json),
[raw commands, trajectories and cleanup]({base}sequences/).
The control is freshly measured raw SuMa with one fixed setting across the cohort.
{negative}
No setting is reselected, no paper ratio or speed claim is made, and defaults stay off.

{diagnostics}

{range_table}

{target_note}

[01 trace]({base}plots/01/trace.png), [numeric trace]({base}plots/01/trace.csv)
and [provenance]({base}plots/01/provenance.json) show successive displacement
and pointwise position error from committed poses without fitting or rescaling.
These curves are distinct from the official segment-based RTE.

{range_explanation}
Complete ranges and the largest positive pointwise error-growth blocks
remain in the report. RTE can worsen without a moving step crossing this local
threshold. Saved-pose diagnostics locate where errors grow; they do not establish
a solver-level cause. Historical SuMa results were already known, so this is not
a new held-out evaluation.

All 22 existing selected IMLS-SLAM/SuMa full-sequence audit cells match RPE, ATE,
frame count and official translation/rotation RTE, with zero maximum deltas.
{frozen_text}
The audit covers the startup helper's two native consumers, not the whole zoo.
The raw controls also match the prior committed raw SuMa controls on every sequence,
and official scan bytes/GT identity match that study. The separate translation-remap
experiment and published results remain unchanged.
'''
p=repo/'papers/suma/README.md';t=p.read_text();start='<!-- SHARED_COARSE_RESULTS:START -->';end='<!-- SHARED_COARSE_RESULTS:END -->';before,rest=t.split(start,1);_,after=rest.split(end,1);p.write_text((before+start+'\n'+block+'\n'+end+after).replace('will retain full KITTI 00–10 results and compatibility checks.', 'record full KITTI 00–10 results and compatibility checks.'))
p=repo/'papers/imls_slam/README.md';t=p.read_text();needle='は別に記録し、以下の既存結果は更新しない。\n';assert needle in t
note='''
00–10 の既存採用セルを IMLS-SLAM・SuMa 各 11 件、計 22 件照合し、RPE・ATE・
フレーム数・公式並進／回転 RTE の差はすべて 0 だった
([全監査集計](../../experiments/results/shared_coarse_start/report/summary.json))。
共通化前の固定バイナリとの full 04 比較も記録した
([比較](../../experiments/results/shared_coarse_start/sequences/04/frozen_comparison.json))。
監査対象は初期段ヘルパーを使うこの 2 手法であり、全手法の再実行ではない。
'''
audit_start='<!-- SHARED_COARSE_AUDIT:START -->';audit_end='<!-- SHARED_COARSE_AUDIT:END -->'
audit_before,audit_rest=t.split(audit_start,1);_,audit_after=audit_rest.split(audit_end,1)
p.write_text(audit_before+audit_start+'\n'+note.strip()+'\n'+audit_end+audit_after)
p=repo/'docs/roadmap.md';t=p.read_text();start=t.index('   - In progress in [PR #136]');end=t.index('\n7. **Bindings.**',start)
block=f'''   - Second component implemented and evaluated in [PR #136](https://github.com/rsasaki0109/localization_zoo/pull/136):
     IMLS-SLAM and opt-in SuMa share the initial coarse/fine stage policy.
     Per-method correspondences, residuals and numerical stopping remain local;
     this is not a shared numerical aligner. The fixed budget and distance scale
     derive from the existing IMLS 04 choice without a SuMa sweep. Full KITTI
     00-10 paired results and saved-pose diagnostics are committed, including
     every negative pair. SuMa 01 RTE changes from {target['control_rte_pct']:.6f}%
     to {target['candidate_rte_pct']:.6f}%, but ATE remains {target['candidate_ate_m']:.6f} m
     and later drift remains. No setting is reselected and defaults stay off.
     All 22 affected existing cells match
     RPE, ATE, frame count and official translation/rotation RTE.
     [Protocol, completed results and commands](../experiments/results/shared_coarse_start/README.md).
     Near-range sharing and NCLT evaluation remain open.'''
p.write_text((t[:start]+block+t[end:]).replace('Coarse registration, near-range sharing and NCLT evaluation remain open.', 'Near-range sharing and NCLT evaluation remain open.'))
p=e/'README.md';t=p.read_text()
t=t.replace('Native evaluation\ncontinues with the fixed protocol.', 'Native evaluation used the fixed protocol.')
interim_start=t.find('<!-- COARSE_PROGRESS:START -->')
if interim_start!=-1:
 interim_end=t.index('<!-- COARSE_PROGRESS:END -->',interim_start)+len('<!-- COARSE_PROGRESS:END -->')
 t=t[:interim_start]+t[interim_end:].lstrip('\n')
old='''Full KITTI 00–10 evaluation and affected-method compatibility checks will be
recorded here, with commands and actual exits.''';new='''Full KITTI 00–10 evaluation and the affected-method compatibility checks are
complete. [Accuracy](report/table.md), [diagnostics](report/diagnostics.md),
[CSV](report/comparison.csv) and the [immutable input snapshot](report/summary.json)
retain every sequence. '''+negative+' Commands and actual exits are retained.';assert old in t;t=t.replace(old,new)
t+='''
The full report was generated with the default all-sequence guard from committed
inputs before updating numeric documentation:

```bash
python experiments/results/shared_coarse_start/summarize_results.py --output-dir experiments/results/shared_coarse_start/report
```

[Exact command and checked exit](report/command.json) retain the Python path and
cwd. [Final controller completion](attempts/final_completion.json) records full
coverage and input cleanup. The report verifies complete native argv, executable
hashes, thread counts, full frames, GT CSV identity, saved-pose RTE agreement and
old official translation/rotation RTE. Official scan .bin hashes/bytes and GT CSV
match the prior study, and raw control accuracy/frame metrics are unchanged there.
Explicit historical partial reports do not
replace the complete report.

All 22 selected existing IMLS-SLAM/SuMa cells have zero changes in RPE, ATE,
frame count and official translation/rotation RTE. '''+frozen_text+'''
[Final regeneration and checks](checks/final/commands.json) retain the required
three doc generators and actual exits. [Preservation](checks/final/preservation.json)
checks all historical aggregates, published README/HTML/Table 6/paper metadata,
fixed manifest settings, native source and the previous remap experiment.
[01 figure](plots/01/trace.png), [numeric trace](plots/01/trace.csv) and
[provenance](plots/01/provenance.json) preserve the remaining later drift, with
no path fitting or rescaling. Concurrent independent runs do not support speed
comparisons. Near-range sharing and NCLT remain open; this completes only the
second roadmap component.
''';p.write_text(t)
print(json.dumps({'input_commit':s['input_commit'],'worse_sequences':worse,'frozen_04_pose_bytes_identical':pose_identity,'docs_written':True}))
