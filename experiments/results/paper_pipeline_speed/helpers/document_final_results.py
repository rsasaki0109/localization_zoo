"""Render numerical prose only from committed paired execution evidence."""
from pathlib import Path
import json
import subprocess

repo = Path('/workspace/localization_zoo')
e = repo / 'experiments/results/paper_pipeline_speed'
comparison = e / 'comparisons/f4755573.json'
assert subprocess.check_output(['git', 'show', 'HEAD:' + str(comparison.relative_to(repo))], cwd=repo) == comparison.read_bytes()
data = json.loads(comparison.read_text())
assert data['complete_method_set'] and data['all_rte_equal_to_three_decimals']
assert all(v['pose_matrix_max_abs_delta'] == 0 for v in data['methods'].values())
names = {'mesh_loam': 'Mesh-LOAM', 'imls_slam': 'IMLS-SLAM', 'inten_loam': 'InTEn-LOAM'}
rows = []
for method, v in data['methods'].items():
    b, a = v['baseline'], v['accelerated']
    disposition = 'Rejected; original code retained' if method == 'mesh_loam' else 'Retained, opt-in/default off'
    rows.append(f"| {names[method]} | {b['time_ms']/1000:.3f} | {a['time_ms']/1000:.3f} | {v['speedup']:.2f}x | {b['kitti_rte_trans_pct']:.6f} | {a['kitti_rte_trans_pct']:.6f} | {disposition} |")
summary = '''# Paper pipeline speed experiment

The full KITTI 07 comparison retains optional IMLS-SLAM and InTEn-LOAM
optimizations. Mesh-LOAM's candidate was slower and is excluded from the
implementation. **The roadmap's speed target remains open.**

## Full paired comparison

[Committed comparison](comparisons/f4755573.json), measured implementation
`f47555735c94d43d6f8e1554ee7bf50be77ac159`; raw paired executions were committed
in `efec7cac` before deriving the comparison in `f8abcb1b`.
Each method runs the complete sequence with its selected registration arguments,
four OpenMP workers, one OpenBLAS worker and the profiler disabled. A fresh
frozen-source control immediately precedes each candidate run. Times are the
harness `time_ms`, including scan loading and preprocessing.

| Method | Fresh control [s] | Candidate [s] | Speedup | Control RTE [%] | Candidate RTE [%] | Disposition |
|---|---:|---:|---:|---:|---:|---|
''' + '\n'.join(rows) + '''

None of these attempts reaches the speed target. Official RTE is unchanged to
the third decimal for every method; all saved KITTI matrices match at their
recorded precision (nine significant digits). This does not measure differences
below that serialization precision. These are single pairs on a shared host,
not confidence intervals; the repeated development windows below show timing
variation. The earlier full baselines are preserved separately and do not define
the primary speed ratios.

## Retained code and fixed settings

The measured candidate above includes the rejected Mesh implementation. The
retained source is `1b51db694a137379151366b889087f1186bb08b5`: Mesh source,
header and original mechanism tests are restored to the frozen baseline, and
`--paper-pipeline-accelerated` applies only to the IMLS-SLAM and InTEn-LOAM paper
paths. The option is off by default and describes repository implementation
optimizations, not a processing step specified by either paper. Their kernels
and mechanism tests are unchanged from the measured candidate. The harness
also fixes IMLS's reporting of effective paper-path settings.

The selected argument lists are copied unchanged from the existing fixed
variants in [the frozen plan](../../paper_pipeline_speed_kitti07_plan.json)
and `protocol.json`. Mesh's correction was selected using KITTI 00/07;
IMLS's coarse first registration was selected after the KITTI 04 failure;
InTEn's plane check/correction was selected on 07 windows with 00 influencing
the diagnosis. No registration setting was retuned for this speed experiment.
Implementation development and profiling use KITTI 07, so this full run is
not held out. Historical cross-sequence results continue to use their original
default implementations and evidence.

Old IMLS receipts round the actual bandwidth to `h=0.1` and print unused dense
startup defaults. The configured paper bandwidth is `0.06` and radius `0.20`;
the retained harness prints these effective settings accurately. This is a
reporting correction, not a registration-parameter change. The original logs
are preserved.

## Profiles and development history

'''
p = e / 'README.md'
old = p.read_text()
body = old[old.index('The frozen baseline is source'):]
body = body.replace('The Mesh tree candidate is a negative\nresult and is replaced with cached packet filters. IMLS cell-topology caching is\nbeing evaluated next. The target remains open until full-sequence acceleration\nand accuracy have been measured.', 'The Mesh tree candidate is a negative\nresult; subsequent packet and normal-bound candidates are recorded below.\nThe full paired comparison above determines the target status.')
body = body.replace('Final source `f4755573` also removes InTEn soft-pause scheduling. Final build has passed; its full CTest, screening and paired full-sequence\nvalidation are in progress.', 'Measured source `f4755573` removes InTEn soft-pause scheduling. Its build,\nfull CTest, default/accelerated screening and paired full-sequence validation\nall have successful command receipts under `candidates/f4755573/`.\nThe normal-bound Mesh candidate is excluded from the retained implementation\nafter its full-sequence slowdown.')
body = body.replace('CLI changes\nonly parse the option and forward method parameters;', 'CLI changes\nparse/forward the option and report effective settings;')
p.write_text(summary + body)

p = repo / 'docs/roadmap.md'
text = p.read_text()
assert '<!-- PAPER-PIPELINE-SPEED:START -->' not in text
factors = ', '.join(f"{names[m]} {v['speedup']:.2f}x" for m, v in data['methods'].items())
block = f'''   <!-- PAPER-PIPELINE-SPEED:START -->
   - Full KITTI 07 experiment: {factors}.
     Official RTE agrees to the third decimal; saved matrices match at their
     recorded precision. **The 3x speed target is not complete.**
   - Mesh's slower candidate is excluded; its original implementation is
     retained. IMLS-SLAM and InTEn-LOAM acceleration remains opt-in and default
     off. Their selected registration arguments are unchanged; 07 is
     development data. Negative tree, packet-cache and scheduling trials
     remain in the method READMEs.
   - Primary ratios use fresh frozen-source full controls immediately before
     each candidate run, with the profiler disabled and fixed thread counts.
     [Commands, profiles, comparisons and limits](../experiments/results/paper_pipeline_speed/README.md).
   <!-- PAPER-PIPELINE-SPEED:END -->

'''
assert '6. **Shared robustness components.**' in text
p.write_text(text.replace('6. **Shared robustness components.**', block + '6. **Shared robustness components.**', 1))
print('Rendered full comparison prose and roadmap status from committed evidence.')
