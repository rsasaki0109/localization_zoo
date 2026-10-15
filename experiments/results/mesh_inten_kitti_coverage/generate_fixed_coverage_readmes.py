#!/usr/bin/env python3
"""Render fixed-setting coverage tables from committed execution evidence."""
import importlib.util
from collections import Counter
import json
import math
from pathlib import Path
import shlex
import subprocess
import sys

REPO = Path(__file__).resolve().parents[3]
RESULTS = REPO / 'experiments/results'
EVIDENCE = RESULTS / 'mesh_inten_kitti_coverage'
METHODS = [('mesh_loam', 'Mesh-LOAM', 'paper_odometry_elevation'),
           ('inten_loam', 'InTEn-LOAM', 'paper_odometry_plane_check')]


def committed(path):
    relative = str(Path(path).relative_to(REPO))
    return json.loads(subprocess.check_output(['git', 'show', f'HEAD:{relative}'], cwd=REPO, text=True))


def mean_ratio(values):
    return math.exp(sum(math.log(value) for value in values) / len(values))


def main():
    paper_path = REPO / 'evaluation/data/paper_reported_numbers.json'
    paper = committed(paper_path)['methods']
    comparison = json.loads(paper_path.read_text())['methods']
    for method, _, variant in METHODS:
        assert comparison[method]['repo_variants'] == [variant]
    spec = importlib.util.spec_from_file_location('coverage_paper_ratio', REPO / 'evaluation/scripts/generate_paper_ratio_table.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    best_rows = module.collect_rows(paper_path, RESULTS)
    summary = {'schema_version': 1,
               'input_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip(),
               'command': shlex.join(['python3', *sys.argv]), 'methods': {}}
    index = committed(RESULTS / 'index.json')
    summary['index_summary'] = {
        'status_counts': dict(Counter(row['status'] for row in index['problems'])),
        'active_selectors': len({committed(REPO / row['aggregate_path'])['stable_interface']['methods']
                                 for row in index['problems']}),
    }
    for method, label, variant in METHODS:
        rows = []
        frozen = None
        for sequence in range(11):
            seq = f'{sequence:02d}'
            path = RESULTS / f'{method}_kitti_seq_{seq}_full_paper_comp_matrix.json'
            aggregate = committed(path)
            row = next(row for row in aggregate['variants'] if row['id'] == variant)
            assert row['status'] == 'ok' and row['frames'] > 0
            frozen = row['args'] if frozen is None else frozen
            assert row['args'] == frozen, f'Configuration changed on {method} {seq}'
            assert '--no-gt-seed' in frozen
            value = row['kitti_rte_trans_pct']
            reference = paper[method]['reported_values'][f'kitti_{seq}']
            assert math.isfinite(value) and value > 0
            result = {'sequence': seq, 'variant': variant, 'paper_rte_pct': reference,
                      'fixed_rte_pct': value, 'fixed_ratio': value / reference,
                      'frames': row['frames'], 'ate_m': row['ate_m'], 'fps': row['fps'],
                      'aggregate': str(path.relative_to(REPO)), 'above_twice_paper': value > 2 * reference}
            if result['above_twice_paper']:
                diag_path = RESULTS / 'runs' / path.stem / variant / 'step_diagnostics.json'
                result['diagnostics_path'] = str(diag_path.relative_to(REPO))
                result['diagnostics'] = committed(diag_path)
            rows.append(result)
        best_ratios = []
        for best in best_rows:
            if best['method'] != method:
                continue
            assert best['repo_best_variant'] == variant
            assert best['repo_pool_size'] == '1' and best['selected_from_n'] == '1'
            aggregate = committed(REPO / best['repo_best_aggregate'])
            row = next(row for row in aggregate['variants'] if row['id'] == best['repo_best_variant'])
            best_ratios.append(row['kitti_rte_trans_pct'] / paper[method]['reported_values'][f"kitti_{best['sequence']}"])
        assert len(best_ratios) == 11
        fixed_mean = mean_ratio([row['fixed_ratio'] for row in rows])
        best_mean = mean_ratio(best_ratios)
        info = {'label': label, 'variant': variant, 'args': frozen,
                'fixed_geometric_mean_ratio': fixed_mean,
                'table6_best_geometric_mean_ratio': best_mean,
                'table6_selects_fixed_variant_everywhere': all(
                    row['repo_best_variant'] == variant for row in best_rows if row['method'] == method),
                'above_twice_paper': [row['sequence'] for row in rows if row['above_twice_paper']],
                'rows': rows}
        summary['methods'][method] = info
        lines = ['<!-- FIXED-KITTI-RESULTS:START -->', '',
                 'Full sequences, official KITTI translational RTE (100-800 m), no GT seed.',
                 'The 00/07 rows retain their existing committed results; the other rows are',
                 'the added coverage runs. Every row below uses the same fixed variant.', '',
                 '| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |',
                 '|---|---:|---:|---:|---:|']
        for row in rows:
            seq = row['sequence'] + (' (development)' if row['sequence'] in ['00', '07'] else '')
            lines.append(f"| {seq} | {row['paper_rte_pct']:.2f} | {row['fixed_rte_pct']:.3f} | {row['fixed_ratio']:.2f}x | {row['fps']:.3f} |")
        lines.extend(['', f'Geometric mean over all sequences with paper values: **{fixed_mean:.2f}x**',
                      'with the fixed setting. Table 6 is restricted to this same variant by',
                      '`repo_variants` in `evaluation/data/paper_reported_numbers.json`.',
                      'The historical 00/07 ablations above remain available in their original',
                      'result aggregates.', '',
                      'FPS describes the recorded execution: the new runs use two OpenMP',
                      'threads per method and may overlap on the same sequence. The 00/07 FPS',
                      'comes from the historical runs.', '',
                      'Sources: `experiments/results/' + method + '_kitti_seq_NN_full_paper_comp_matrix.json`.',
                      'New per-run summaries, host records, logs, estimated poses, evaluated GT',
                      'and step diagnostics are committed under the corresponding',
                      '`experiments/results/runs/` directory. Exact execution commands and the',
                      'isolated-output launcher used for overlapping the two methods are recorded',
                      'in `experiments/results/mesh_inten_kitti_coverage/`.', '',
                      '#### Sequences above twice the paper value', ''])
        near_threshold = [row for row in rows
                          if not row['above_twice_paper'] and round(row['fixed_ratio'], 2) == 2.0]
        if near_threshold:
            lines.extend(['The strict >2x check uses the unrounded RTE. Ratios displayed as',
                          '2.00x can remain at or below that threshold:', ''])
            for row in near_threshold:
                lines.append(f"- Seq {row['sequence']}: {row['fixed_rte_pct']:.6f} % <= {2 * row['paper_rte_pct']:.6f} % (twice the paper value).")
            lines.append('')
        failures = [row for row in rows if row['above_twice_paper']]
        if not failures:
            lines.append('No sequence exceeds twice its paper value with this fixed setting.')
        for row in failures:
            d = row['diagnostics']
            longest = sorted(d['bad_length_step_frame_ranges'], key=lambda pair: pair[1] - pair[0], reverse=True)[:3]
            ranges = ', '.join(f'{first}-{last}' for first, last in sorted(longest)) or 'none'
            lines.extend([f"**{row['sequence']}: {row['fixed_rte_pct']:.3f} % vs {row['paper_rte_pct']:.2f} % ({row['fixed_ratio']:.2f}x).**",
                          f"The estimated path length is {d['path_length_ratio']:.3f} of GT.",
                          f"For GT steps of at least {d['parameters']['min_step_m']:g} m,",
                          f"{d['steps_length_off_over_threshold']} of {d['moving_step_count']} steps differ in length by more than 20 %;",
                          f"the local translation-vector error exceeds 20 % on {d['steps_vector_off_over_threshold']} steps.",
                          (f'The longest runs of step length errors have destination-frame indices {ranges} '
                           '(zero-based; inclusive).' if longest else
                           'No step length error exceeds that threshold.'),
                          'All intervals are retained in the diagnostic JSON.', '',
                          'The largest relative-translation-error blocks locate where drift grows.',
                          'Each block contains up to 100 consecutive steps; these endpoint errors',
                          'are diagnostics, not official KITTI RTE.', '',
                          'Blocks spanning at least 1 m of GT travel are ranked below; the JSON',
                          'also retains blocks with less travel.', '',
                          '| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |',
                          '|---|---:|---:|---:|---:|'])
            blocks = sorted((b for b in d['blocks'] if b['gt_distance_m'] >= 1),
                            key=lambda b: b['block_relative_translation_error_pct'] or 0, reverse=True)[:3]
            for b in blocks:
                lines.append(f"| {b['first_pose_frame']}-{b['last_pose_frame']} | {b['gt_distance_m']:.1f} | {b['path_length_ratio']:.3f} | {b['position_error_start_m']:.1f} / {b['position_error_end_m']:.1f} | {b['block_relative_translation_error_pct']:.2f} |")
            lines.extend(['', 'Diagnostic source: `' + row['diagnostics_path'] + '`.', ''])
        lines.extend(['', 'Saved-pose diagnostics are generated with',
                      '`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>',
                      '--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "' + label + '"',
                      '--output <step_diagnostics.json>`; each JSON records the exact command.',
                      'The script checks its full-sequence RTE against the original run summary.',
                      '', '<!-- FIXED-KITTI-RESULTS:END -->'])
        readme = REPO / 'papers' / method / 'README.md'
        text = readme.read_text()
        block = '\n'.join(lines)
        start, end = '<!-- FIXED-KITTI-RESULTS:START -->', '<!-- FIXED-KITTI-RESULTS:END -->'
        if start in text:
            first = text.index(start); last = text.index(end, first) + len(end)
            text = text[:first] + block + text[last:]
        else:
            anchor = 'fetching, result import and regeneration of the derived tables.\n'
            assert text.count(anchor) == 1
            text = text.replace(anchor, anchor + '\n' + block + '\n', 1)
        text = text.replace('### Fixed configuration for the coverage run', '### KITTI 00-10 with one configuration')
        readme.write_text(text)
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    (EVIDENCE / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps({method: {key: value for key, value in info.items() if key != 'rows'}
                      for method, info in summary['methods'].items()}, indent=2))


if __name__ == '__main__':
    main()
