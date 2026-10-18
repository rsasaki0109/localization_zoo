from pathlib import Path
import json,subprocess
repo=Path('/workspace/localization_zoo');E=repo/'experiments/results/shared_near_range'
source='61c1f26e1019aa3f0b05b100404c8bb117448b41'
path=E/'diagnostics/00/summary.json'
raw=subprocess.check_output(['git','show',source+':'+path.relative_to(repo).as_posix()],cwd=repo)
assert raw==path.read_bytes();d=json.loads(raw)
modules={'A-LOAM':'aloam','F-LOAM':'floam','LeGO-LOAM':'lego_loam','Mesh-LOAM':'mesh_loam','VLOM':'vlom'}
marker='SHARED_NEAR_RANGE_00_DIAGNOSTICS'
for r in d['rows']:
    p=repo/'papers'/modules[r['method']]/'README.md';text=p.read_text();assert marker not in text
    b=r['largest_position_error_growth_block'];growth=b['position_error_end_m']-b['position_error_start_m']
    block=(f'\n\n<!-- {marker} -->\n## KITTI 00 current-replay error growth\n\n'
        f'The committed current replay has {r["steps_length_off_over_threshold"]}/{r["moving_step_count"]} moving steps with length error above 20%, '
        f'and {r["steps_vector_off_over_threshold"]}/{r["moving_step_count"]} with local translation-vector error above 20%. '
        f'The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.\n\n'
        f'The largest increase in pointwise position error among those blocks is poses '
        f'{b["first_pose_frame"]}–{b["last_pose_frame"]}: GT travels {b["gt_distance_m"]:.6f} m, '
        f'while error rises from {b["position_error_start_m"]:.6f} m to {b["position_error_end_m"]:.6f} m '
        f'(increase {growth:.6f} m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. '
        f'The tool uses first-pose normalization without fitting or rescaling.\n\n'
        f'The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout\'s reachable history. '
        f'Without its historical saved path, this does not locate the cause of the historical metric difference. '
        f'[Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) '
        f'use the committed current poses, with diagnostic evidence committed at `{source}`. '
        f'Native source remains `{d["implementation_commit"]}`.\n<!-- /{marker} -->\n')
    p.write_text(text.rstrip()+block)
lines=['# KITTI 00: current replay error growth','',
       f'Diagnostics were committed at `{source}` before these numeric notes. Inputs are the completed candidate replay poses at `{d["input_commit"]}`; native source is `{d["implementation_commit"]}`. No estimator is rerun and no setting is selected.','',
       'The five cells below differ from historical stored metrics. This report locates error growth in their **current replay trajectories**. The referenced historical per-run summaries are absent at the inspected revision and were not found in this checkout\'s reachable history. The search is limited to this checkout and revision; no paired old-path comparison can be made from these artifacts. Their historical causes remain unresolved.','',
       'The existing `diagnose_kitti_steps.py` definitions are used unchanged for every method: 100-frame blocks (including the final shorter block), moving GT steps at least 0.1 m, and a 20% length/vector error threshold. The displayed block maximizes position-error end minus start over all blocks. It is a diagnostic choice, not parameter tuning or official KITTI RTE. The tool normalizes the first pose without fitting or rescaling.','',
       '| Method | Length-error steps >20% | Vector-error steps >20% | Moving steps | Largest-growth pose interval | GT travel [m] | Position error start [m] | End [m] | Increase [m] |',
       '|---|---:|---:|---:|---|---:|---:|---:|---:|']
for r in d['rows']:
    b=r['largest_position_error_growth_block']
    lines.append(f'| {r["method"]} | {r["steps_length_off_over_threshold"]} | {r["steps_vector_off_over_threshold"]} | {r["moving_step_count"]} | {b["first_pose_frame"]}–{b["last_pose_frame"]} | {b["gt_distance_m"]:.6f} | {b["position_error_start_m"]:.6f} | {b["position_error_end_m"]:.6f} | {b["position_error_end_m"]-b["position_error_start_m"]:.6f} |')
lines += ['', '![Current replay position errors](trace.png)', '',
          '[Numeric per-frame trace](trace.csv), [SVG](trace.svg), [all blocks, source hashes, legacy-path probes and definitions](summary.json). A-LOAM and VLOM curves overlap; the input manifests record identical saved-estimate hashes for these two current runs.', '',
          'Each method folder retains the exact `diagnose_kitti_steps.py --estimate ... --gt ... --summary ... --method ... --output ...` command, actual exit, full block diagnostics and saved-pose metric checks. Recomputed ATE, 100 m RPE and official translation/rotation RTE agree with their committed summaries within the recorded absolute tolerance. These checks do not replace the original historical audit exit 1 or establish full-study completion.', '',
          'Run `diagnose_current00.py --output-dir <fresh directory>` using the recorded command in `../../checks/diagnose00/diagnose00_command.json`. The SVG line-end padding initially fails the branch whitespace check with exit 2; original output and SVG are preserved as gzip archives in `../../checks/diagnose00/`. The recorded `normalize_svg.py` command removes only line-end padding, preserves every non-whitespace token and parses both XML forms; the corrected check exits 0. No new whitespace exception is added.', '',
          'Four frozen 00 controls are documented in the study README; Mesh-LOAM comparison and input cleanup are tracked there separately. These current-path diagnostics do not close those gates or fresh NCLT validation.']
(E/'diagnostics/00/README.md').write_text('\n'.join(lines)+'\n')
p=E/'README.md';text=p.read_text();anchor='## Scope and status'
assert text.count(anchor)==1
intro=('The [current KITTI 00 saved-pose diagnostics](diagnostics/00/README.md) locate\n'
       'error growth in the five replay paths that differ from stored metrics.\n'
       'Referenced historical per-run summaries are missing at the inspected revision,\n'
       'so these diagnostics do not identify the cause of the historical differences.\n\n')
p.write_text(text.replace(anchor,intro+anchor))
p=repo/'docs/roadmap.md';text=p.read_text();anchor='     later audit results are follow-up work.\n';assert text.count(anchor)==1
p.write_text(text.replace(anchor,anchor+'     [Current 00 trajectory diagnostics](../experiments/results/shared_near_range/diagnostics/00/README.md)\n     locate error growth; missing referenced historical paths prevent a paired\n     diagnosis of the historical metric differences.\n'))
print('Rendered numeric diagnostic notes from committed61c1f26e only.')
