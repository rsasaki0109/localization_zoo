"""Report every full recursive outcome only after numerical evidence is committed."""
from pathlib import Path
import json,subprocess
E=Path(__file__).resolve().parent;W=E.parents[2]
def read(p):return json.loads(p.read_text())
def f(v):return 'n/a' if v is None else str(v) if isinstance(v,int) else f'{v:.6f}'
def intervals(ranges):return ', '.join(str(a) if a==b else f'{a}–{b}' for a,b in ranges) or 'none'
def committed(p):
    assert subprocess.check_output(['git','show','HEAD:'+str(p.relative_to(W))],cwd=W)==p.read_bytes(),p

def main():
    for p in (Path(__file__).resolve(),E/'postprocessing_protocol.json',E/'report/summary.json'):committed(p)
    report=read(E/'report/summary.json');sequences=report['sequences'];assert report['all_sequences_present'] and len(sequences)==11
    commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip()
    head='''# Recursive geometry consensus at the original estimated rotation

This outside-paper, default-off extension runs the same setting on every full
KITTI 00–10 sequence. It tests the mechanism that passed the preceding conditional
feasibility gate, now with its own changed mapping and prediction history. It is
not an independent held-out evaluation: original profile on 07, diagnosis on 00, inherited
DiLO 01 prior 0.03 and every previous KITTI observation are development.

`--inten-loam-paper-set s2s_geometry_consensus=1` keeps the original native joint
S2S estimate, computes the unchanged full-point translation-consensus profiles at
the original estimated input, corrects seed translation and facade support when
strict-majority/interior consensus is available, then actually re-solves translation
while retaining the original estimated rotation. No GT, hand region, threshold
sweep, per-sequence winner or parameter retuning enters the estimator.

All points, profile grid/trimming/admission-band/vote rules are inherited unchanged
from the committed conditional study. Every cluster can admit multiple shifts.
Cluster votes do not establish static-object truth. Invalid original solves,
unusable consensus or unsuccessful refinement retain the original result. The
native return rule requires at least 30 returned pairs and a finite pose; an
iteration-limit stop can still be accepted. Applied does not mean converged or
accurate. Native S2M, DOR/insertion and mapped prediction definitions stay fixed, but their realized
inputs and maps change. Fixing local S2S rotation does not fix global mapped
rotation or guarantee that the original rotation is correct.

Both arms keep mapped seed, prior 0.03, ground/facade PCA and acceleration on;
facade Cauchy, predicted DOR, row completion and plane spread off; range 120 m,
correction 0.17 deg, S2M plane check 0.05 m, no GT seed. The new Boolean is the only accuracy-setting
difference. Baseline OMP 1/candidate OMP 4 are fixed across sequences, with bitwise
cached-cost/estimated-solver agreement at both worker counts. Raw times remain
available, but unequal fixed worker counts on the shared host do not constitute
a standalone throughput benchmark.

'''
    result=[f'Numerical evidence `{commit}` precedes this interpretation.','',
      f"Geometric mean candidate/reference official RTE: **{f(report['geometric_mean_candidate_over_baseline'])}x**.",
      'Lower: '+(', '.join(report['lower_rte_sequences']) or 'none')+'. Higher: '+(', '.join(report['higher_rte_sequences']) or 'none')+'. Equal: '+(', '.join(report['equal_rte_sequences']) or 'none')+'.',
      'Every sequence and every adverse metric is retained. The option stays default off; no per-sequence default is adopted.','',
      '| Sequence | Official RTE ref → candidate [%] | Rotation ref → candidate [deg/100m] | ATE ref → candidate [m] | RPE ref → candidate [%] | Bad length ref → candidate | Bad vector ref → candidate |',
      '|---|---:|---:|---:|---:|---:|---:|']
    keys=('kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m','ate_m','rpe_trans_pct','final_bad_length','final_bad_vector')
    for seq,d in sequences.items():
        a,b=d['modes']['baseline'],d['modes']['candidate']
        result.append('| '+seq+' | '+' | '.join(f(a[k])+' → '+f(b[k]) for k in keys)+' |')
    result+=['','Local bad labels use the inherited 20% threshold when GT translation is at least 0.1 m. They are distinct from official distance-segment RTE.','',
      '## Every adverse comparison','']
    names=dict(kitti_rte_trans_pct='RTE [%]',kitti_rte_rot_deg_per_100m='rotation [deg/100m]',ate_m='ATE [m]',rpe_trans_pct='RPE [%]',final_bad_length='bad length',final_bad_vector='bad vector')
    for seq,adverse in report['adverse_metrics'].items():
        if adverse:
            a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
            result.append('- '+seq+': '+'; '.join(names[k]+' '+f(a[k])+' → '+f(b[k]) for k in adverse)+'.')
    if not any(report['adverse_metrics'].values()):result.append('No adverse values in the declared metric set.')
    result+=['','| Sequence | New bad length frames | Reference-only bad length | New bad vector frames | Reference-only bad vector |','|---|---:|---:|---:|---:|']
    for seq,d in sequences.items():
        labels=d['local_label_changes']
        result.append(f"| {seq} | {len(labels['length']['candidate_only_frames'])} | {len(labels['length']['baseline_only_frames'])} | {len(labels['vector']['candidate_only_frames'])} | {len(labels['vector']['baseline_only_frames'])} |")
    result+=['','Every affected frame index is retained in the numerical report. These are realized-trajectory comparisons, not matched-history causal interventions.','',
      '## Consensus use and remaining motion deficits','',
      '| Sequence | Evaluated | Usable | Applied | Original-result fallbacks | Excluded cluster decisions |','|---|---:|---:|---:|---:|---:|']
    for seq,d in sequences.items():
        c=d['modes']['candidate']['consensus_validation']['counts']
        result.append(f"| {seq} | {c['evaluated']} | {c['usable']} | {c['applied']} | {c['fallback']} | {c['excluded']} |")
    result+=['','Counts condition on this realized trajectory. The original joint trace remains in `s2s`; actual conditional refinement is in `s2s_consensus.refinement`. S2S error statistics use the accepted relative motion. First ground/facade counts and prior-information statistics describe the original joint solve; the refinement has separate complete equations and checks.','']
    result+=['## Error before and after each applied refinement','',
      '| Sequence | Moving applied frames | Vector error lower / higher / equal | Absolute length error lower / higher / equal |',
      '|---|---:|---|---|']
    for seq,d in sequences.items():
        c=d['modes']['candidate']['within_frame_refinement']
        counts=lambda key:' / '.join(str(c[key][name]) for name in ('lower','higher','equal'))
        result.append(f"| {seq} | {c['moving_applied_frames']} | {counts('vector_error')} | {counts('absolute_length_error')} |")
    result+=['','These compare the valid original joint estimate with the actually accepted refinement at the same frame and realized history, restricted to applied refinements and GT steps of at least 0.1 m. Rotation is held at the original estimate. GT is used only for this diagnostic. The comparison measures the immediate local change; it does not reconstruct a counterfactual full baseline or attribute later map drift to one cause. Exact joint/accepted errors and every frame remain in the step CSVs.','']
    for seq in ('01','02','10'):
        a,b=sequences[seq]['modes']['baseline'],sequences[seq]['modes']['candidate']
        result.append(f"- {seq}: final bad-length intervals {intervals(a['bad_length_frame_ranges'])} → {intervals(b['bad_length_frame_ranges'])}; accepted S2S bad-length steps {a['s2s_bad_length']} → {b['s2s_bad_length']}; original S2S fallbacks {a['s2s_fallback']} → {b['s2s_fallback']}; rejected S2M {a['s2m_rejected']} → {b['s2m_rejected']}.")
    result+=['','## Error growth above twice paper','',
      '| Sequence | Candidate RTE [%] | Paper RTE [%] | Ratio | Growth block | Position error start → end [m] | Block translation error [%] |',
      '|---|---:|---:|---:|---|---:|---:|']
    for seq in report['over_twice_paper_sequences']:
        d=sequences[seq];b=d['modes']['candidate']
        for block in b['largest_position_error_growth_blocks']:
            result.append(f"| {seq} | {f(b['kitti_rte_trans_pct'])} | {f(d['paper_rte_pct'])} | {f(d['candidate_over_paper'])} | {block['first_pose_frame']}–{block['last_pose_frame']} | {f(block['position_error_start_m'])} → {f(block['position_error_end_m'])} | {f(block['block_relative_translation_error_pct'])} |")
    if not report['over_twice_paper_sequences']:result+=['','No candidate sequence exceeds twice the cited paper RTE.']
    result+=['','These blocks locate error growth, including gradual drift without a local label crossing. They do not establish a solver-level physical cause. Block errors are not official distance-segment RTE; paper values contextualize an outside-paper extension. All steps, boundary observations and error blocks remain in the complete report.','',
      '![All official RTE outcomes](report/official_rte.png)']
    for seq in report['over_twice_paper_sequences']:result+=['',f'![KITTI {seq} error growth](report/seq_{seq}_error_growth.png)']
    tail='''

## Reproduction and preservation

Sources/rules/manifests and binary/library hashes were frozen before full runs.
Every unchanged full baseline reproduces the historical accepted reference's
metrics, pose/GT bytes and full numerical trace after the declared scan-root-only
substitution. Cached controls reproduce every original full-point profile and
estimated solver trace at both worker counts, including the prior adverse
endpoints and declared majority/boundary fallback. No GT is read by the estimator.

The query implementation preserves the original float coordinates, exact k=1
nearest-neighbor definition, squared distances, point order, sort order and serial
sum order. It reuses the native exact FeatureTree and balances independent
(cluster, shift) cells; each cell still reduces in the original serial order.
The earlier implementation and both distinct accelerated builds/binaries remain
preserved. Both cached acceleration stages reproduce the original costs, native
poses and complete solver traces at the fixed worker counts; their receipts are
in `checks/cached_parity/` and `checks/cached_parity_balanced/`.

The original full 01 negative result is retained under
`../inten_recursive_consensus/sequences/01/`. Its complete numerical traces,
pose/GT bytes and accuracy metrics must match both new full 01 arms exactly.
The original slow 07 candidate was deliberately interrupted after the exact cached
controls passed. Its actual signal exit, wrapper failure, all completed baseline
bytes and partial candidate bytes were archived, roundtripped, committed and
pushed under `../inten_recursive_consensus/collection_interruption/`; it is not
reported as a complete run. The new candidate starts at frame zero and must match
every complete old candidate prefix record. The owned official 07 scans are reused
without another acquisition, then removed only after the new complete pair is
archived and pushed. Complete-sequence equivalence is recorded by the actual
`implementation_equivalence.py` command and receipt for every sequence.

Every full frame retains the actual admission bands, votes, excluded clusters,
seed/original estimate and refinement equations. Independent validation reconstructs
all votes/masks/fallbacks, verifies bitwise original rotation at every projected
step, additive translation, damped Htt backward error and accepted-pose flow.
Full original joint/S2M equations, adaptive prior and mapped prediction are also
checked. The completed CLI trace-on/off control preserves metrics and pose bytes.

The initial synthetic test confused the minimum-cost shift with the nearest-zero
admissible consensus; its failed actual CTest and build are retained. Only its
expectation changed; the helper/rules were preserved. The corrected native build
uses a distinct directory and every native check passes. Original log bytes are
losslessly archived, including CMake whitespace. A clean-tree preflight rejected
an unstaged artifact set before any replay; that actual rejected start is retained.
The first accelerated link driver used a stale library path after successful
configure/build/CTest; its failure and a link-only continuation are retained.
The first cached-control setup failed before native execution because of a stale
module name; the failure and corrected distinct staging paths remain preserved.
The balanced build and both cached equivalence gates have actual successful
command exits. These engineering failures did not trigger accuracy retuning.
No failed attempt or negative outcome is silently dropped.

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/build_balanced.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/parity_balanced.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/run_collection.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/run_postprocessing.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/finalize_docs.py
```

Use the matching frozen source commits and fresh private build/staging paths;
existing evidence and binaries must not be overwritten. Exact actual commands,
cwd, source/environment, input/binary/static-library hashes and exit codes are in
`checks/`, `sequences/`, `collection_execution/` and postprocessing receipts.
The original external tool session became unavailable after a turn interruption;
its OS exit is explicitly unknown. The durable `subprocess.run` receipt records
the actual collector exit as zero, alongside every native/validator exit; the
completed wrapper archive commit and recovery observations are preserved in
`checks/outer_execution_completion.json`. No success is inferred from log text.
Official KITTI acquisition uses `fetch_kitti_odometry_sequences.py`, one owned
sequence at a time. Every scan/GT hash matches committed official inputs. Each
sequence was losslessly archived, roundtrip-checked, committed and normally pushed
before scan deletion; cleanup was committed/pushed before the next acquisition.

Disk space is conserved only by byte/hash/mode/owner/device-identical sharing
of immutable evidence and feature caches. Every logical path and file byte is
preserved. Closed-worktree coalescing excludes both producer worktrees and every
raw/source/build/object/library/binary/index/top-level aggregate path; the exact
source, command and observed successful OS exit remain in
`checks/closed_evidence_storage/`. Earlier cache-sharing receipts are retained too.

All historical aggregates/results/defaults and primary paper publications are
preserved. New complete rows are experimental, nondefault and unscored for paired
timing. No per-sequence winner is adopted. Scientific checks use actual exit codes;
full Native and ROS2 CI on the final commit are required before review.
'''
    (E/'README.md').write_text(head+'\n'.join(result)+tail)
    method=W/'papers/inten_loam/README.md';old=method.read_text();assert '<!-- INTEN_RECURSIVE_CONSENSUS -->' not in old
    intro='\n\n<!-- INTEN_RECURSIVE_CONSENSUS -->\n## Recursive geometry consensus at the original estimated rotation\n\n'
    intro+='`s2s_geometry_consensus=1` is outside the paper and default off. The frozen consensus seed/support rule re-solves translation at the original joint estimated rotation, with native re-association and adaptive prior. It keeps the original on unavailable consensus/failed refinement. S2M/map feedback can change global rotation. The original 07/00/DiLO 01 provenance and every prior KITTI observation are development; no GT seed, hand region, sweep, retuning or per-sequence adoption.\n\n'
    intro+='[Complete commands, controls, failures, every step and all outcomes](../../experiments/results/inten_consensus_query_acceleration/README.md).\n\n'
    body='\n'.join(result).replace('(report/','(../../experiments/results/inten_consensus_query_acceleration/report/')
    method.write_text(old+intro+body+'\n<!-- /INTEN_RECURSIVE_CONSENSUS -->\n')
    print('All full outcomes and adverse metrics written from committed numerical evidence')

if __name__=='__main__':main()
