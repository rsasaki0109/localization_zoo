from pathlib import Path
import json
import subprocess

repo = Path('/workspace/localization_zoo')
e = repo / 'experiments/results/shared_near_range'
primary_commit = '9555f01fda68a0294fc7dd5a828a85fac207f68f'
diagnostic_commit = '1426a851c9f021cbffae239e814b6379a5788b25'


def committed(path, revision):
    raw = subprocess.check_output(['git', 'show', revision + ':' + path.relative_to(repo).as_posix()], cwd=repo)
    assert raw == path.read_bytes()
    return json.loads(raw)


audit = committed(e / 'sequences/01/audit/report.json', primary_commit)
verification = committed(e / 'checks/01_primary/verification.json', primary_commit)
diagnostic = committed(e / 'diagnostics/01/summary.json', diagnostic_commit)
assert verification['completed_primary_cells'] == len(audit['rows'])
assert not verification['cleanup_complete'] and not verification['frozen_comparison_complete']
assert audit['exit_code'] == verification['historical_audit_exit_code'] == 1
drifts = [r for r in audit['rows'] if r['audit_status'] == 'drift']
assert {r['method'] for r in drifts} == {'CT-ICP', 'Mesh-LOAM'}
diagnoses = {r['method']: r for r in diagnostic['rows']}
mesh = diagnoses['Mesh-LOAM']
gap = mesh['largest_path_gap_growth_block']
assert mesh['paired_historical_comparison_available']
assert not diagnoses['CT-ICP']['paired_historical_comparison_available']

table = ['| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |',
         '|---|---:|---:|---:|---:|']
for r in drifts:
    table.append(f"| {r['method']} | {r['stored_rpe_trans_pct']:.6f} | {r['rerun_rpe_trans_pct']:.6f} | {r['stored_ate_m']:.6f} | {r['rerun_ate_m']:.6f} |")

p = e / 'README.md'
s = p.read_text()
intro_anchor = '## Scope and status'
assert intro_anchor in s
intro = '''The [KITTI 01 saved-path diagnosis](diagnostics/01/README.md) compares the
available historical/current Mesh-LOAM paths and locates current CT-ICP error
growth. Its runtime and missing-path limits are explicit; it does not identify
the cause of historical metric differences.

'''
assert 'diagnostics/01/README.md' not in s
s = s.replace(intro_anchor, intro + intro_anchor, 1)
section = f'''## Completed candidate runs: 01; full frozen comparison and cleanup pending

All {len(audit['rows'])} selected candidate cells finish, with native exits 0 and
{audit['rows'][0]['frames']} evaluated frames each. {verification['historical_matching_cells']} match stored RPE, ATE and frame count;
the following {len(drifts)} differ, so the historical audit exits **1**.

{chr(10).join(table)}

RPE worsens for both. CT-ICP ATE improves; Mesh-LOAM ATE worsens. The original
arguments are replayed without selecting a setting. The completed candidate
results, commands, native hashes, input hashes and verification were committed
at `{primary_commit}`:
[candidate evidence](sequences/01/audit/), [inputs](sequences/01/inputs.json),
[verification](checks/01_primary/verification.json).

The controller is comparing both historical differences with the frozen
pre-extraction executable. The complete comparison and owned-input cleanup
remain pending, so these candidate runs are **not** added to the cleaned
checkpoint count. The cleaned 04/00 checkpoint remains **34/177** cells.
Attribution of either 01 difference to extraction remains unestablished here.
Stored aggregates, Table 6, paper metadata and defaults are unchanged.

[Committed saved-path diagnostics](diagnostics/01/README.md) pair Mesh-LOAM's
historical/current paths against byte-identical evaluated GT, and locate current
CT-ICP error growth. The Mesh source/header match its recorded original source,
but its original/current OpenMP settings differ; this is not a controlled
thread-count experiment or a cause attribution. CT-ICP's referenced legacy
summary/path is unavailable in the inspected checkout. Diagnostic evidence was
committed at `{diagnostic_commit}` before numeric prose.

'''
anchor = 'Native stdout contains padded tables.'
assert anchor in s and '## Completed candidate runs: 01;' not in s
p.write_text(s.replace(anchor, section + anchor, 1))

for r in drifts:
    folder = 'ct_icp' if r['method'] == 'CT-ICP' else 'mesh_loam'
    p = repo / 'papers' / folder / 'README.md'
    s = p.read_text()
    assert '<!-- SHARED_NEAR_RANGE_REPLAY -->' not in s
    block = f'''
<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader replay: KITTI 01 comparison in progress

The full stored `{r['variant_id']}` replay completes all {r['frames']} frames with
native exit 0. Historical 100 m RPE is **{r['stored_rpe_trans_pct']:.6f}%**, versus
**{r['rerun_rpe_trans_pct']:.6f}%** in this replay; ATE is **{r['stored_ate_m']:.6f} m**
versus **{r['rerun_ate_m']:.6f} m**. RPE worsens; ATE
{'improves' if r['rerun_ate_m'] < r['stored_ate_m'] else 'worsens'}. The historical audit exits **1**, preserving the difference.

The complete frozen pre-extraction comparison and verified input cleanup are
pending. Whether this cell changes because of extraction is not established
by the candidate result. Its historical cause remains unresolved. No setting
is reselected, and the near-range cut remains off.

[Commands, exits, hashes and candidate results](../../experiments/results/shared_near_range/sequences/01/audit/)
were committed at `{primary_commit}`. The
[study and cleaned checkpoint scope](../../experiments/results/shared_near_range/README.md)
keep this candidate-only evidence separate from completed sequence checkpoints.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
'''
    s = s.rstrip() + '\n' + block
    current = diagnoses[r['method']]['largest_current_error_growth_block']
    detail = f'''The current path's largest position-error increase in the existing fixed
blocks is frames **{current['first_pose_frame']}-{current['last_pose_frame']}**:
**{current['position_error_start_m']:.6f} to {current['position_error_end_m']:.6f} m**
against evaluated GT. This locates current error growth, not the historical
metric difference.
'''
    if folder == 'mesh_loam':
        detail += f'''
The historical/current saved paths use byte-identical evaluated GT. Their
position gap reaches **{mesh['max_path_gap_m']:.6f} m** at frame
**{mesh['max_path_gap_frame']}** and ends at **{mesh['final_path_gap_m']:.6f} m**.
Its largest fixed-block increase is frames **{gap['first_pose_frame']}-{gap['last_pose_frame']}**,
from **{gap['gap_start_m']:.6f} to {gap['gap_end_m']:.6f} m**.
The recorded original and current OpenMP thread settings are **2 and 1**;
the method source/header match, but no controlled thread-count experiment
is performed. These observations do not establish the cause of the difference.
'''
    else:
        detail += '''
The referenced historical per-run summary is absent in the inspected tree
and was not found in the reachable history of this checkout. There is no
paired historical path here, so this cannot locate where that path differs.
'''
    s += f'''
<!-- SHARED_NEAR_RANGE_01_DIAGNOSTICS -->
### Saved KITTI 01 path diagnosis

{detail}
The [diagnostic table, plot, CSVs, source pins and exact command](../../experiments/results/shared_near_range/diagnostics/01/README.md)
come from committed evidence `{diagnostic_commit}`. First-pose normalization
is used without fitted alignment or rescaling. No estimator is rerun and no
published accuracy result is replaced.
<!-- /SHARED_NEAR_RANGE_01_DIAGNOSTICS -->
'''
    p.write_text(s)

lines = ['| Method | Current error-growth block | Error at block start [m] | Error at block end [m] | Historical path paired? |',
         '|---|---|---:|---:|---|']
for name, d in diagnoses.items():
    b = d['largest_current_error_growth_block']
    lines.append(f"| {name} | {b['first_pose_frame']}-{b['last_pose_frame']} | {b['position_error_start_m']:.6f} | {b['position_error_end_m']:.6f} | {'yes' if d['paired_historical_comparison_available'] else 'no'} |")
(e / 'diagnostics/01/README.md').write_text(f'''# KITTI 01: saved paths and historical differences

These offline diagnostics read the committed candidate results and available
historical poses. They do not run an estimator, select a setting, establish
an extraction effect or replace a published result. The historical audit retains
its exit 1; full frozen comparison and scan cleanup are tracked in the
[current study](../../README.md).

## Current error growth

The [protocol](../01_protocol.json) inherits the existing 100-step blocks,
0.1 m moving-step threshold and 20% step-error threshold. The displayed block
maximizes position-error end minus start; all blocks remain in the step JSONs.
Estimates and GT are normalized at their first pose, without fitted alignment
or rescaling. These endpoint diagnostics are not official KITTI RTE.

{chr(10).join(lines)}

CT-ICP's referenced historical per-run summary is absent in the inspected
revision and was not found by the recorded reachable-history query. Its current
error growth cannot locate a difference from that unavailable historical path.
This is a statement about this checkout, not global absence.

## Paired Mesh-LOAM paths

Historical/current evaluated GT files are byte-identical. Their path gap is
the Euclidean position difference after the same first-pose normalization.
It reaches **{mesh['max_path_gap_m']:.6f} m** at frame **{mesh['max_path_gap_frame']}**,
ending at **{mesh['final_path_gap_m']:.6f} m**. The largest gap increase among
the fixed blocks is **{gap['first_pose_frame']}-{gap['last_pose_frame']}**:
**{gap['gap_start_m']:.6f} to {gap['gap_end_m']:.6f} m**
(increase **{gap['gap_growth_m']:.6f} m**).

![Historical/current error and gap](mesh_loam/paired_trace.png)

[Full Mesh trace CSV](mesh_loam/trace.csv), [historical steps](mesh_loam/historical_steps.json),
[current steps](mesh_loam/current_steps.json), [CT current trace](ct_icp/trace.csv)
and [complete summary](summary.json) retain all frames and blocks.

The source/header match the original recorded source
`d1b7288ff46964bde86ef15ab34b2ae8b4f389da`. Original/current recorded OpenMP
thread counts are 2 / 1 (OpenBLAS remains 1). This runtime difference is a
confounding factor, not a demonstrated cause. No controlled rerun with changed
thread count is performed, and the compiled binaries/loaders are not claimed
to be identical to the original execution. Frozen pre-extraction controls
are separate comparisons with matched current runtime settings.

## Commands and evidence

```bash
python3 experiments/results/shared_near_range/diagnose_saved01.py --output-dir <fresh-directory>
```

The actual command, analysis/input/source commits, executable and file hashes,
legacy availability probes, paired blocks and saved-pose metric checks are in
[summary.json](summary.json) and [analysis_command.json](analysis_command.json).
The exact analysis output is losslessly retained in [analysis.log.gz](analysis.log.gz).
All current/historical metric checks pass the recorded 0.001 absolute tolerance.
Raw diagnostic evidence was committed at `{diagnostic_commit}` before this
numeric prose; source and protocol were committed before execution.
''')

p = repo / 'docs/roadmap.md'
s = p.read_text()
anchor = '     The historical NCLT cutoff is retained'
addition = f'''     Full 01 candidate replay also finishes all {len(audit['rows'])} cells: {verification['historical_matching_cells']} match;
     CT-ICP and Mesh-LOAM differ, retaining audit exit 1. Their numeric pairs
     are in the method READMEs. Full frozen comparison and input cleanup are
     pending, so the cleaned checkpoint count remains 34/177.
     [Candidate evidence](../experiments/results/shared_near_range/sequences/01/audit/)
     and [saved-path diagnosis](../experiments/results/shared_near_range/diagnostics/01/README.md)
     retain Mesh's paired path gap and CT's current error-growth block, with
     explicit runtime/missing-path limits and no cause attribution.
'''
assert anchor in s
p.write_text(s.replace(anchor, addition + anchor, 1))
print('Documented committed01 candidates and offline diagnostics; cleaned checkpoint remains04/00.')
