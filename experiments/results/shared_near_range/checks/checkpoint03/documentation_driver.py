"""Render the committed cleaned03 checkpoint and saved02 diagnosis."""
from pathlib import Path
import json
import subprocess

REPO = Path('/workspace/localization_zoo')
E = REPO / 'experiments/results/shared_near_range'
RAW = '8f5841e3946995cedfa9c0e2fc484dbd2c02373e'

def read(name):
    p = E / name
    raw = subprocess.check_output(['git','show',RAW+':'+p.relative_to(REPO).as_posix()],cwd=REPO)
    if raw != p.read_bytes(): raise RuntimeError('Changed committed input: '+name)
    return json.loads(raw)

x = read('checkpoints/03/summary.json')
d = read('diagnostics/02/summary.json')
if x['completed_sequences'] != ['04','00','01','02','03'] or x['whole_zoo_audit_complete']:
    raise RuntimeError('Checkpoint scope differs')
rows = [r for r in x['historical_drift_cells'] if r['sequence'] in ('02','03')]
diags = {r['method']:r for r in d['rows']}
mesh = diags['Mesh-LOAM']
gap = mesh['largest_path_gap_growth_block']

def table(values):
    lines=['| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |',
           '|---|---|---:|---:|---:|---:|']
    for r in values:
        lines.append(f"| {r['method']} | {r['sequence']} | {r['stored_rpe_trans_pct']:.6f} | {r['rerun_rpe_trans_pct']:.6f} | {r['stored_ate_m']:.6f} | {r['rerun_ate_m']:.6f} |")
    return '\n'.join(lines)

p=E/'README.md'
s=p.read_text()
s=s.replace('The current [\u0060checkpoints/01/\u0060](checkpoints/01/) adds','The earlier [\u0060checkpoints/01/\u0060](checkpoints/01/) adds')
s=s.replace('## Scope and status','The [KITTI02 saved-path diagnosis](diagnostics/02/README.md) now includes all\nhistorical differences on that sequence and the available Mesh paired paths.\n\n## Scope and status',1)
s+=f'''
## Current cleaned checkpoint: 02 and 03 added

The [checkpoint](checkpoints/03/summary.json) and [comparison CSV](checkpoints/03/comparison.csv),
committed at \u0060{RAW}\u0060, cover **{x['completed_cell_count']}/{x['planned_cell_count']} cells**
across 04, 00, 01, 02 and 03: **{x['historical_matching_cell_count']}** historical matches and
**{x['historical_drift_cell_count']}** differences. All **{len(x['frozen_comparisons'])}** frozen controls have zero
recorded metric differences and byte-identical saved poses. All five historical
audits retain exit **1**; checkpoint verification exits **0**. Coverage is incomplete.

New historical differences, retained in each method README:

{table(rows)}

Candidate/frozen reports and native receipts are committed. Scan hashes were
verified before deleting owned inputs and aliases. The sequence commits are
\u0060a821eaee\u0060 (02) and \u0060eebaf3d7\u0060 (03). These differences predate extraction;
their original causes remain unresolved. This replays historical per-cell
arguments rather than defining a new single-setting accuracy study.
[Saved02 diagnostics](diagnostics/02/README.md) locate current error growth and
the available Mesh path gap, with missing-path/runtime limits and no cause attribution.

## Environment interruption and remaining-sequence recovery

The managed environment restarted during05; its original controller, native
processes and completion worker were absent afterward. The interrupted audit is
preserved under \u0060sequences/05/interrupted_20261018/\u0060. Unknown exits are null,
and that attempt contributes no completed cells.

The [recovery protocol](recovery05_protocol.json) and [source](recover_interrupted05.py)
verify frozen source/binary/sealed pins and owned official05 scan/GT hashes, then
rerun every selected05 cell from fresh native state. Original audit/comparison
helpers, arguments, one-thread settings and cleanup policy are retained.
After a complete05 commit, the sealed controller continues at the next unstarted
sequence. This is an explicit environment-interruption addendum to the original
fetch-failure-only restart procedure.

[Launch receipts](checks/recovery05_launch/) identify the recovery and replacement
completion worker. Unfinished work is excluded from the checkpoint. Historical
aggregates, paper numbers, Table 6 and defaults are preserved. This replay retains
its original frozen implementation; current-main acceleration/passive tracing
and real NCLT validation keep their previously documented scope.
'''
p.write_text(s)

growth=['| Method | Current error-growth block | Error at start [m] | Error at end [m] | Historical path paired? |',
        '|---|---|---:|---:|---|']
for method,row in diags.items():
    b=row['largest_current_error_growth_block']
    growth.append(f"| {method} | {b['first_pose_frame']}-{b['last_pose_frame']} | {b['position_error_start_m']:.6f} | {b['position_error_end_m']:.6f} | {'yes' if row['paired_historical_comparison_available'] else 'no'} |")
(E/'diagnostics/02/README.md').write_text(f'''# KITTI02: committed saved-path diagnosis

Source/protocol precede execution. Numerical evidence was committed at \u0060{RAW}\u0060
before this prose. Candidate/frozen reports, actual native receipts, matching
paths and hash-verified cleanup are committed. Owned scans and alias are absent;
the historical audit retains exit **1**.

## Current error growth

The [inherited protocol](../02_protocol.json) fixes 100-step blocks, a 0.1 m
moving-step threshold and 20% relative-step threshold. The displayed block
maximizes position-error end minus start; every block and frame is retained.
First-pose normalization uses no fitting or rescaling. These endpoint errors are
distinct from official KITTI RTE.

{chr(10).join(growth)}

The A-LOAM, CT-ICP, F-LOAM and LeGO-LOAM referenced historical per-run paths are
unavailable in the inspected tree/reachable-history probes. Current error growth
cannot locate where those unavailable paths differ. This describes the checkout,
rather than global absence.

## Paired Mesh-LOAM paths

Evaluated GT bytes match. The Euclidean path gap reaches **{mesh['max_path_gap_m']:.6f} m**
at frame **{mesh['max_path_gap_frame']}** and ends at **{mesh['final_path_gap_m']:.6f} m**.
Its largest fixed-block increase is frames **{gap['first_pose_frame']}-{gap['last_pose_frame']}**,
from **{gap['gap_start_m']:.6f} to {gap['gap_end_m']:.6f} m**
(increase **{gap['gap_growth_m']:.6f} m**).

![Saved error and path gap](mesh_loam/paired_trace.png)

The method source/header match the recorded original source. Original/current
OpenMP counts are **2 / 1**, with OpenBLAS **1**. This is a runtime confound, not
a controlled thread-count experiment or a cause attribution. The separate,
matched-runtime frozen pre-extraction controls reproduce the current paths exactly.

## Commands and retained checks

\u0060\u0060\u0060bash
python3 experiments/results/shared_near_range/diagnose_saved02.py --output-dir <fresh-directory>
\u0060\u0060\u0060

[Summary and input/source hashes](summary.json), [actual command/exit](analysis_command.json),
[full lossless output](analysis.log.gz), step JSONs and trace CSVs retain all
blocks/frames. Saved-pose metrics match summaries within the inherited **0.001**
absolute tolerance.

The initial diagnosis rejected the real checkpoint with exit **2**: it confused
the audit-wrapper hash with the native executable hash. The [failure and gate
regressions](../../checks/diagnose02_wrapper_fix/) precede correction \u0060869dd694\u0060.
The reports now pin the committed wrapper; each native receipt independently pins
the candidate/frozen executable. The successful analysis exits **0**.
Registration settings, saved results and diagnostic definitions are unchanged.
''')

folders={'A-LOAM':'aloam','CT-ICP':'ct_icp','F-LOAM':'floam','LeGO-LOAM':'lego_loam','Mesh-LOAM':'mesh_loam'}
for method,folder in folders.items():
    p=REPO/'papers'/folder/'README.md'
    s=p.read_text()
    marker='<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->'
    if marker in s: raise RuntimeError('Already documented')
    value=next(r for r in rows if r['sequence']=='02' and r['method']==method)
    b=diags[method]['largest_current_error_growth_block']
    extra=f"""The largest current position-error increase among the inherited fixed blocks
is frames **{b['first_pose_frame']}-{b['last_pose_frame']}**:
**{b['position_error_start_m']:.6f} to {b['position_error_end_m']:.6f} m**.
"""
    if method=='Mesh-LOAM':
        extra+=f"""
Historical/current GT bytes match. The path gap reaches **{mesh['max_path_gap_m']:.6f} m**
at frame **{mesh['max_path_gap_frame']}**, ending at **{mesh['final_path_gap_m']:.6f} m**. Its largest block
increase is **{gap['first_pose_frame']}-{gap['last_pose_frame']}**:
**{gap['gap_start_m']:.6f} to {gap['gap_end_m']:.6f} m**.
Identical method sources and differing OpenMP settings (**2 / 1**) do not isolate
runtime as the cause.
"""
    else:
        extra+='\nThe referenced historical per-run path is unavailable in the inspected tree/history.\nCurrent error growth cannot locate a gap from that unavailable trajectory.\n'
    s+=f'''
{marker}
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

{table([value])}

The selected \u0060{value['variant_id']}\u0060 arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

{extra}
The [protocol, all steps/CSVs and plot](../../experiments/results/shared_near_range/diagnostics/02/README.md)
come from committed evidence \u0060{RAW}\u0060. First-pose normalization uses no fitted
alignment or rescaling. These observations locate errors without assigning a
cause or replacing published accuracy. The
[cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/03/summary.json)
retains all historical differences and audit exits.
<!-- /SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
'''
    if method=='Mesh-LOAM':
        value=next(r for r in rows if r['sequence']=='03')
        s+=f'''
<!-- SHARED_NEAR_RANGE_03_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI03 historical difference

{table([value])}

RPE and ATE worsen, retaining historical audit exit **1**. The frozen control
has native exit **0**, zero recorded metric differences and byte-identical
poses; this difference also predates extraction. Its cause remains unresolved.
Owned-scan hashes were verified before scan/alias removal. Settings and disabled
defaults are unchanged. [Audit/control/cleanup and actual commands](../../experiments/results/shared_near_range/sequences/03/)
were committed at \u0060eebaf3d7\u0060 before this prose.
<!-- /SHARED_NEAR_RANGE_03_DIAGNOSTICS -->
'''
    p.write_text(s)

p=REPO/'docs/roadmap.md'; s=p.read_text()
before='''     The committed, cleaned 04/00/01 checkpoint now covers 47/177 cells:
     38 historical matches and 9 differences, all reproduced by frozen controls.
     All three historical audits retain exit 1; checkpoint verification exits 0.
     [Checkpoint, CSV and commands](../experiments/results/shared_near_range/checkpoints/01/summary.json).'''
after=f'''     The committed, cleaned 04/00/01/02/03 checkpoint now covers {x['completed_cell_count']}/{x['planned_cell_count']} cells:
     {x['historical_matching_cell_count']} historical matches and {x['historical_drift_cell_count']} differences, all reproduced by frozen controls.
     All {len(x['frozen_comparisons'])} controls have zero metric differences and byte-identical poses.
     All five historical audits retain exit 1; checkpoint verification exits 0.
     [Checkpoint, CSV and commands](../experiments/results/shared_near_range/checkpoints/03/summary.json).
     [Saved02 diagnosis](../experiments/results/shared_near_range/diagnostics/02/README.md)
     retains current error-growth blocks and the available Mesh paired path,
     with explicit missing-path/runtime limits. Full03 adds a Mesh historical
     difference reproduced by its frozen control; method READMEs retain all
     numerical pairs. Inputs were hash-verified before deletion.
     The environment restarted during05. Interrupted outputs are preserved and
     excluded from coverage; a source-pinned recovery reruns all selected05
     cells with the same frozen executables/settings before continuing.
     [Recovery protocol](../experiments/results/shared_near_range/recovery05_protocol.json).'''
if before not in s: raise RuntimeError('Roadmap anchor differs')
p.write_text(s.replace(before,after,1))
print('Documented committed cleaned03 checkpoint, saved02 paths and recovery scope.')
