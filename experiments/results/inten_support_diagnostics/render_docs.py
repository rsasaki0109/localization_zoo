#!/usr/bin/env python3
"""Render diagnostic prose only after its numerical evidence is committed."""
import json
from pathlib import Path
import subprocess

REPO = Path(__file__).resolve().parents[3]
STUDY = Path(__file__).resolve().parent
path = STUDY / 'analysis/summary.json'
relative = path.relative_to(REPO).as_posix()
assert path.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + relative], cwd=REPO)
evidence = subprocess.check_output(['git', 'log', '-1', '--format=%H', '--', relative], cwd=REPO, text=True).strip()
data = json.loads(path.read_text())
assert data['exit_code'] == 0
rows = {r['sequence']: r for r in data['sequences']}


def value(x):
    return '-' if x is None else str(x)


def table():
    text = ['| Sequence | All bad-length steps | Logged bad steps | Unlogged bad steps | Min S2S pairs on logged bad steps | Min S2M pairs on logged bad steps |',
            '|---|---:|---:|---:|---:|---:|']
    for row in data['sequences']:
        s = row['logged_bad_length']
        text.append(f"| {row['sequence']} | {row['all_bad_length_steps']} | {s['logged_steps']} | "
                    f"{row['unlogged_bad_length_steps']} | {value(s['s2s_min'])} | {value(s['s2m_min'])} |")
    return '\n'.join(text)


moving = sum(r['logged_moving']['logged_steps'] for r in data['sequences'])
bad = sum(r['logged_bad_length']['logged_steps'] for r in data['sequences'])
assert all(r['logged_moving'][k] == 0 for r in rows.values()
           for k in ('s2s_below_cutoff', 's2m_below_cutoff'))

readme = f'''# InTEn-LOAM: correspondence support at saved failure steps

The committed native logs show substantial correspondence support at the sampled
steps with large length errors. This excludes the **low-count
branch at those recorded frames**. It does not explain the motion deficit or
exclude an earlier fallback influencing later estimates.

This is an offline join of the existing fixed-setting runs, not an estimator
experiment. No scans are read or downloaded and no native code, parameter,
stored aggregate, paper value or published accuracy table is changed.

## Scope and reproducibility

[`protocol.json`](protocol.json) records the new fixed-setting coverage runs
01-06 and 08-10 included in this analysis. The low-count hypothesis was formed
while inspecting these saved failures; this is an exploratory diagnostic.
Historical 00/07 runs are outside this log
analysis. Every included manifest uses the unchanged `paper_odometry_plane_check`
arguments: the plane distance and elevation correction were selected on 07,
with 00 influencing the diagnosis. No new setting is selected here. The focus
01/02/10 failures were already identified by their paper comparisons.

Run from the repository root, with NumPy available:

```bash
python3 experiments/results/inten_support_diagnostics/analyze.py --output-dir <fresh-directory>
python3 experiments/results/inten_support_diagnostics/render_docs.py
```

The exact executed interpreter, command, working directory, analysis revision,
input hashes and recorded native build/binary pins are in
[`analysis/summary.json`](analysis/summary.json). Native execution commands and
successful exits come from the existing coverage study's `parallel_execution.json`.
The original code at `{data['recorded_native_commit']}` is read directly from Git;
the InTEn paper source/header match this branch byte for byte. The recorded
build metadata is distinguished from the analysis commit.

The analysis and raw check receipts were committed at `{evidence}` before this
numeric prose. [`checks/preflight/`](checks/preflight/) preserves actual exits
and losslessly compressed logs. Existing step ranges/counts are independently
recomputed from saved poses and matched, and the saved-pose accuracy metrics
agree with their recorded summaries within the declared analysis tolerance.

## Observed support and missing observations

The native runner prints progress every 10 frames. Frame 0 is initialization
and is excluded from step statistics. A bad-length step has GT travel at least
0.1 m and an absolute length-ratio error above 20%, exactly as in the existing
saved-pose diagnostics. Its index is the destination frame.

{table()}

Across these sequences, all {moving} logged moving steps, including {bad} logged
bad-length steps, have S2S and S2M returned counts at least 30. The recorded
code replaces an S2S solution with the previous relative motion only below 30
pairs; S2M accepts its correction at or above 30 pairs. The selected pipeline
has mapping enabled at every frame. Thus neither low-count condition holds
at the sampled failure frames. A dash means no logged bad step, not a measured
zero minimum.

The [per-logged-step CSV](analysis/logged_steps.csv) also retains vector error,
feature counts and S2M class counts. The
[complete bad-length interval CSV](analysis/bad_length_intervals.csv) includes
the singleton and unobserved intervals, without filling missing counts.

## Limits and next diagnostic

The large counts rule out correspondence-count shortage at the observed steps
only. The unlogged bad steps in the table remain unobserved, including earlier
frames that might influence prediction or the map. No claim is made about
their fallback branches. Sequence 10 has no step beyond the existing 20%
threshold despite its paper gap; this threshold does not describe all drift.

Many correspondences can still be ambiguous, poorly distributed or associated
incorrectly. These progress logs contain neither per-iteration residuals nor
Jacobian conditioning, so they do not establish observability, convergence or
the solver-level cause. Ground truth here labels saved steps only; it never
initializes an estimator.

The merged [passive trace API and streaming workflow](../../../docs/inten_loam_passive_trace.md)
now provide stage motions, fallback/acceptance decisions, normal equations and
iteration residuals for continuous observation. Tracing stays opt-in and default
off. The legacy forced-pose diagnostics change placement and prediction and
cannot serve as passive observation. The streaming checker validates record/run
consistency; it does not establish solver quality or a failure cause. Conditioning
analysis also requires an explicit convention for mixed translation/rotation units.

Continuous failure-sequence collection remains pending. Use the unchanged
selected registration setting after the ongoing frozen replay releases native
and input ownership; do not borrow its live scans. Existing progress records
still leave earlier fallback and unlogged frames unobserved. Short-fixture
trace compatibility is separate from this full failure diagnosis.

## Reproduction after main integration

Source `3d7e3f91d4e0c28d8c286fef68e72a49ca5c14fe` follows integration of main
`a597e4c95614e733fc20977321fe4d6aea0fba46`. The analyzer reads native source/header
blobs from the recorded evaluator commit and the protocol's pinned input commit,
and requires those historical blobs to match. Current native files can evolve
without changing the interpretation of recorded progress. Stored logs, poses,
manifests and analysis inputs retain their committed-byte checks.

The pre-fix command exits 1 because the current native source differs from the
historical execution; [its receipt and log](checks/main_refresh/before_fix/command.json)
are retained. Raw fix evidence was committed at `70bac660` before this prose.
The [reproduction commands](checks/main_refresh/analysis/commands.json) succeed;
[numerical identity](checks/main_refresh/analysis/numerical_identity.json) records
byte-identical CSVs and unchanged protocol, selected arguments, sequence counts,
step intervals and metric comparisons. The original analysis above remains
attached to its original evidence commit. Re-analysis adds explicit historical
source provenance without a new estimator execution or accuracy measurement.

The [new metadata and outputs](checks/main_refresh/analysis/analysis/summary.json)
record the actual analysis/source/input pins. Core algorithms and evaluation
sources match integrated main; original numerical artifacts remain unchanged.
'''
(STUDY / 'README.md').write_text(readme)

a, b = rows['01'], rows['02']
block = f'''<!-- INTEN_SUPPORT_DIAGNOSTICS -->
#### Pair support at the failure steps

An offline join of the committed native progress logs and saved poses covers
the fixed-setting runs 01-06 and 08-10. In 01, {a['logged_bad_length']['logged_steps']}
of the {a['all_bad_length_steps']} bad-length steps have progress records; their
minimum returned S2S / S2M pair counts are
**{a['logged_bad_length']['s2s_min']} / {a['logged_bad_length']['s2m_min']}**.
In 02, {b['logged_bad_length']['logged_steps']} of {b['all_bad_length_steps']} have
records, with minima **{b['logged_bad_length']['s2s_min']} / {b['logged_bad_length']['s2m_min']}**.
None of these sampled bad steps has a count below 30. At those frames the
code keeps the returned S2S transform without low-count replacement and the
S2M pose passes its count-based acceptance test. The low-count branches do
not explain those individual steps; this does not establish solver convergence.

This is a negative check of the low-count hypothesis, not a cause or fix for
the paper gap. There are still {a['unlogged_bad_length_steps']} unlogged bad steps
in 01 and {b['unlogged_bad_length_steps']} in 02. Their branch decisions and the
effect of earlier fallback remain unknown. High pair counts also do not
establish geometric conditioning or correct associations. The solver-level
cause remains open; settings and stored results are unchanged.

[Protocol, all sequence counts, CSVs, commands and limits](../../experiments/results/inten_support_diagnostics/README.md)
derive from evidence committed at `{evidence}`. No native replay or new
accuracy measurement is performed.
For continuous solver observations, use the opt-in
[passive trace workflow](../../docs/inten_loam_passive_trace.md). Its availability
does not fill the unlogged frames in this historical analysis; full failure
tracing and cause attribution remain pending.
<!-- /INTEN_SUPPORT_DIAGNOSTICS -->'''
p = REPO / 'papers/inten_loam/README.md'
text = p.read_text()
if '<!-- INTEN_SUPPORT_DIAGNOSTICS -->' in text:
    start = text.index('<!-- INTEN_SUPPORT_DIAGNOSTICS -->')
    end = text.index('<!-- /INTEN_SUPPORT_DIAGNOSTICS -->') + len('<!-- /INTEN_SUPPORT_DIAGNOSTICS -->')
    text = text[:start] + block + text[end:]
else:
    anchor = '## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)'
    assert anchor in text
    text = text.replace(anchor, block + '\n\n' + anchor)
p.write_text(text)

p = REPO / 'docs/roadmap.md'
text = p.read_text()
old = '''    These saved-pose diagnostics locate the failures; the solver-level cause
    remains open. The plane check and correction were chosen on 07 windows'''
new = '''    These saved-pose diagnostics locate the failures; the solver-level cause
    remains open. An [offline log/pose join](../experiments/results/inten_support_diagnostics/README.md)
    finds 14 logged bad-length steps each in 01 and 02; all have S2S/S2M
    counts above the low-count cutoff. That excludes low-count fallback at
    those frames only: 122 / 119 other bad steps are unlogged, and high counts
    do not establish conditioning or correct associations. No setting is changed.
    The plane check and correction were chosen on 07 windows'''
if new not in text:
    assert old in text
    # Values in the roadmap must also agree with the committed table.
    assert (a['logged_bad_length']['logged_steps'], b['logged_bad_length']['logged_steps'],
            a['unlogged_bad_length_steps'], b['unlogged_bad_length_steps']) == (14, 14, 122, 119)
    text = text.replace(old, new)
p.write_text(text)
print('Rendered documentation from committed offline diagnostic evidence.')
