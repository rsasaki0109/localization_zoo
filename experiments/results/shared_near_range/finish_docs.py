"""Render final study documentation only from a committed, complete replay report.

Partial reports are rejected before any repository file is written. Numerical
solvers, flags, stored aggregates and the published leaderboard are untouched.
"""
import argparse
import json
from pathlib import Path
import re
import subprocess

from support import E, REPO, SOURCE, PROTOCOL

MODULES = {
    'A-LOAM': 'aloam', 'CT-ICP': 'ct_icp', 'DiLO': 'dilo', 'ELO': 'elo',
    'F-LOAM': 'floam', 'IMLS-SLAM': 'imls_slam', 'InTEn-LOAM': 'inten_loam',
    'KISS-ICP': 'kiss_icp', 'L-LO': 'l_lo', 'LF-GICP': 'lf_gicp',
    'LeGO-LOAM': 'lego_loam', 'LiTAMIN2': 'litamin2', 'MULLS': 'mulls',
    'Mesh-LOAM': 'mesh_loam', 'NDT': 'ndt', 'Quadric-LO': 'quadric_lo',
    'Small-GICP': 'small_gicp', 'SuMa': 'suma', 'VLOM': 'vlom',
    'Voxel-GICP': 'voxel_gicp',
}
OLD_MARKERS = ('SHARED_NEAR_RANGE', 'SHARED_NEAR_RANGE_AUDIT',
               'SHARED_NEAR_RANGE_00_PRIMARY', 'SHARED_NEAR_RANGE_REPLAY')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate_complete(report):
    selection = json.loads((E / 'selection.json').read_text())['rows']
    require(report.get('implementation_commit') == SOURCE, 'Wrong native source.')
    require(report.get('exit_code') == 0, 'Report did not finish successfully.')
    require(report.get('completed_cell_count') == len(selection), 'Incomplete cell count.')
    require(report.get('selected_cell_count') == len(selection), 'Wrong selection size.')
    require(report.get('completed_sequences') == PROTOCOL['order'], 'Incomplete sequences.')
    require(report.get('missing') == [], 'Missing cells.')
    rows = report.get('rows', [])
    expected = {(r['method'], r['sequence'], r['repo_best_aggregate'], r['repo_best_variant'])
                for r in selection}
    actual = {(r['method'], r['sequence'], r['aggregate_path'], r['variant_id']) for r in rows}
    require(actual == expected and len(rows) == len(selection), 'Cell identity mismatch.')
    require(all(r['audit_status'] in ('ok', 'drift') and r['process_exit_code'] == 0
                for r in rows), 'A replay failed.')
    controls = {(r['method'], r['sequence']): r for r in report['frozen_comparisons']}
    require(len(controls) == len(report['frozen_comparisons']), 'Duplicate frozen controls.')
    require(set(controls) <= {(r['method'], r['sequence']) for r in rows}, 'Unknown frozen control.')
    for check in controls.values():
        require(check['saved_pose_bytes_equal'] and all(check['saved_pose_bytes_equal'].values()),
                'Saved poses changed against the frozen executable.')
        require(all(value == 0 for value in check['metric_abs_delta'].values()),
                'A recorded metric changed against the frozen executable.')
    drift = [r for r in rows if r['audit_status'] == 'drift']
    require(report['historical_metric_drift_cells'] == drift, 'Drift list mismatch.')
    require(all((r['method'], r['sequence']) in controls for r in drift),
            'A historical difference lacks its frozen comparison.')
    require({r['method'] for r in rows} == set(MODULES), 'Unknown method mapping.')
    return rows, controls


def outcome_table(rows):
    lines = ['| Seq | Historical variant | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |',
             '|---|---|---:|---:|---:|---:|']
    for row in sorted(rows, key=lambda r: (r['sequence'], r['method'])):
        lines.append(f"| {row['sequence']} | `{row['variant_id']}` | "
                     f"{row['stored_rpe_trans_pct']:.6f} | {row['rerun_rpe_trans_pct']:.6f} | "
                     f"{row['stored_ate_m']:.6f} | {row['rerun_ate_m']:.6f} |")
    return '\n'.join(lines)


def strip_old_blocks(text):
    for marker in OLD_MARKERS:
        text = re.sub(r'\n?<!-- ' + re.escape(marker) + r' -->.*?<!-- /' +
                      re.escape(marker) + r' -->\n?', '\n', text, flags=re.S)
    return text.rstrip()


def historic_nclt(method):
    if method not in ('ELO', 'DiLO'):
        return ''
    stem = 'elo' if method == 'ELO' else 'dilo'
    summaries = []
    for suffix in ('', '_m5'):
        path = f'docs/benchmarks/nclt_paper_paths/{stem}{suffix}.json'
        raw = subprocess.check_output(['git', 'show', PROTOCOL['base_commit'] + ':' + path], cwd=REPO)
        summaries.append(json.loads(raw)['methods'][0])
    a, b = summaries
    caveat = ('DiLO also changes range-image geometry (192 versus 240 rows, top elevation and fill gap); '
              'this is not a controlled cutoff-only pair. ATE worsens.' if method == 'DiLO' else
              'The cut still leaves substantial absolute position error.')
    return (f"\nThe runner's sensor-frame near-range cut is **non-paper preprocessing**, still default zero/off. "
            f"It is shared by ELO/DiLO and intensity-preserving loaders, after sensor-frame conversion and "
            f"before elevation correction/downsampling; standalone cores do not apply it automatically.\n\n"
            f"Historical NCLT full-sequence RTE without/with the existing 5 m cut is "
            f"{a['kitti_rte_trans_pct']:.6f}% / {b['kitti_rte_trans_pct']:.6f}%; "
            f"ATE is {a['ate_m']:.6f} m / {b['ate_m']:.6f} m. {caveat} "
            f"The cutoff was selected on that NCLT sequence and restart window, with no new held-out claim. "
            f"These are historical numbers, not a fresh NCLT rerun. "
            f"[Committed summaries](../../docs/benchmarks/nclt_paper_paths/) and "
            f"[recorded commands and limits](../../docs/nclt_paper_paths.md) retain their original "
            f"missing evaluator-commit/argv provenance. Fresh NCLT validation remains open.\n")


def method_block(method, rows, report_link):
    drift = [r for r in rows if r['audit_status'] == 'drift']
    block = (f"\n\n<!-- SHARED_NEAR_RANGE_REPLAY -->\n"
             f"## Shared-loader full KITTI replay\n\n"
             f"All {len(rows)} selected stored {method} full-sequence cells complete with native exits 0. "
             f"{len(rows) - len(drift)} match historical 100 m RPE, ATE and frame count; "
             f"{len(drift)} differ. Each cell uses its original recorded arguments, which may differ "
             f"by sequence. This audit does not select a new fixed setting or paper value. "
             f"The optional near-range cut still defaults to zero/off.\n")
    if drift:
        block += ('\nEvery historical difference is retained below, including improvements. '
                  'The corresponding historical audits exit **1**.\n\n' + outcome_table(drift) +
                  '\n\nFor every differing cell, the frozen pre-extraction executable reproduces '
                  'the replayed metrics and byte-identical saved poses. Thus these differences '
                  'predate this extraction; their historical causes remain unresolved. '
                  'Official translation/rotation RTE also matches for those frozen controls. '
                  'This is not credited as a filter improvement or used to replace stored results.\n')
    block += historic_nclt(method)
    block += (f"\n[Complete report and commands]({report_link}) retain all original audit exits, "
              f"summaries, logs, saved poses, frozen comparisons and verified input cleanup. "
              f"KITTI replay does not establish fresh NCLT accuracy.\n"
              f"<!-- /SHARED_NEAR_RANGE_REPLAY -->\n")
    return block


def build_documents(report, report_path):
    rows, controls = validate_complete(report)
    relative = report_path.parent.relative_to(E).as_posix()
    files = {}
    for method, module in MODULES.items():
        path = REPO / 'papers' / module / 'README.md'
        own = [r for r in rows if r['method'] == method]
        link = '../../experiments/results/shared_near_range/' + relative + '/summary.json'
        files[path] = strip_old_blocks(path.read_text()) + method_block(method, own, link)
    current = (E / 'README.md').read_text()
    require('## Scope and status' in current and '## Historical NCLT evidence and limits' in current
            and '## Commands and evidence' in current, 'Study README template changed; review before writing.')
    intro = current.split('## Scope and status')[0]
    nclt = current.split('## Historical NCLT evidence and limits', 1)[1].split('## Commands and evidence', 1)[0]
    nclt = nclt.split('Real NCLT validation remains blocked:', 1)[0].rstrip()
    drift = report['historical_metric_drift_cells']
    table = ['| Method | Replayed cells | Historical matches | Historical differences | RPE worse | ATE worse |',
             '|---|---:|---:|---:|---:|---:|']
    for method in sorted(MODULES):
        own = [r for r in rows if r['method'] == method]
        differences = [r for r in own if r['audit_status'] == 'drift']
        table.append(f"| {method} | {len(own)} | {len(own)-len(differences)} | {len(differences)} | "
                     f"{sum(r['rerun_rpe_trans_pct'] > r['stored_rpe_trans_pct'] for r in differences)} | "
                     f"{sum(r['rerun_ate_m'] > r['stored_ate_m'] for r in differences)} |")
    files[E / 'README.md'] = (intro + '## Complete KITTI replay\n\n' +
        f"All {len(rows)} frozen selected cells across full KITTI 00-10 have completed, including "
        f"{len(controls)} frozen-executable controls. All owned scans/aliases were verified and removed "
        f"one sequence at a time. {len(rows)-len(drift)} cells match historical RPE/ATE/frame values; "
        f"{len(drift)} differ. Every differing cell matches the pre-extraction executable's recorded "
        f"errors/frame count, official translation/rotation RTE and saved-pose bytes. Historical causes "
        f"remain unresolved. These are historical per-cell settings, not a new fixed-setting accuracy study.\n\n" +
        '\n'.join(table) + '\n\n' +
        f"[Full committed report]({relative}/summary.json) and [all numeric pairs]({relative}/comparison.csv) "
        f"retain every outcome. Each historical audit's actual nonzero exit remains in its sequence folder; "
        f"successful completion of the orchestration/report is not a claim that historical audits all pass. "
        f"Every affected method README retains its numeric differences. No published value is replaced.\n\n" +
        '## Historical NCLT evidence and limits\n' + nclt + '\n\n' +
        'Fresh real-NCLT validation remains open. The launch-time enforced policy lacked the official '
        'host despite prior approval; the policy observations are retained in `checks/`. This KITTI '
        'evidence does not establish current NCLT access or accuracy.\n\n' +
        '## Commands and validation\n\n'
        '`protocol.json`, `selection.json`, binary pins and `sequences/` retain the frozen source, '
        'arguments, actual process exits, logs/poses, official input hashes, fetch failures and cleanup. '
        'Defaults stay off. Concurrent processes do not support a speed claim.\n\n'
        f"Finalization commands, report generation, document changes and required checks are retained "
        f"under `{relative.rsplit('/report', 1)[0]}/`. Initial Release/125 CTest and 290 Python checks "
        'are in `checks/preflight/` and `checks/python/`; final actual check exits are recorded separately. '
        'Raw native log padding is preserved through narrowly scoped Git attributes.\n')
    roadmap = REPO / 'docs/roadmap.md'
    text = roadmap.read_text()
    start = text.index('   - Third component implemented:')
    stop = text.index('7. **Bindings.**', start)
    replacement = (f"   - Third component implemented and audited in [PR #137](https://github.com/rsasaki0109/localization_zoo/pull/137):\n"
        f"     the optional sensor-frame near-range cut is shared in `common/range_filter.h`,\n"
        f"     used by ELO/DiLO and intensity-preserving loaders. Float arithmetic, strict\n"
        f"     boundary, processing order and zero/off default are preserved; standalone\n"
        f"     cores do not apply it automatically. All {len(rows)} selected full KITTI cells\n"
        f"     complete with verified input cleanup. {len(rows)-len(drift)} match historical\n"
        f"     RPE/ATE/frame values; {len(drift)} differ, with every difference reproduced by\n"
        f"     the frozen pre-extraction executable and retained numerically in method READMEs.\n"
        f"     Historical audits with differences retain exit 1; their causes remain unresolved.\n"
        f"     No setting is reselected or published result replaced. Fresh real-NCLT evaluation\n"
        f"     remains open; its historical DiLO cut/uncut pair also changes range-image geometry\n"
        f"     and cannot isolate the cut's effect.\n"
        f"     [Full report, scope, commands and negative results](../experiments/results/shared_near_range/README.md).\n")
    files[roadmap] = text[:start] + replacement + text[stop:]
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--apply', action='store_true', help='Write verified final docs; otherwise validate only.')
    parser.add_argument('--manifest', type=Path)
    args = parser.parse_args()
    path = args.report.resolve()
    try:
        raw = subprocess.check_output(['git', 'show', 'HEAD:' + path.relative_to(REPO).as_posix()], cwd=REPO)
        require(raw == path.read_bytes(), 'Report is not committed unchanged.')
        report = json.loads(raw)
        documents = build_documents(report, path)
        if args.apply:
            for target, text in documents.items():
                target.write_text(text)
        if args.manifest:
            args.manifest.parent.mkdir(parents=True, exist_ok=True)
            args.manifest.write_text(json.dumps({'applied': args.apply, 'report': str(path),
                'paths': [p.relative_to(REPO).as_posix() for p in documents], 'exit_code': 0}, indent=2) + '\n')
        print(json.dumps({'applied': args.apply, 'document_count': len(documents), 'exit_code': 0}))
        return 0
    except (ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(f'Refusing final documentation: {error}')
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
