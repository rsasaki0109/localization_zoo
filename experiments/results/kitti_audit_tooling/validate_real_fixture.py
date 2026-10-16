"""Exercise audit exit codes with a native replay in an isolated MCD workspace.

The KITTI-shaped directory names are required only by the audit selector. These
inputs are the committed three-frame MCD fixture, never full KITTI sequences.
Reference perturbations are deliberate control-flow tests, not method results.
Run this driver inside localization-zoo-dev after the Release build.
"""
from pathlib import Path
import hashlib, json, os, shutil, subprocess, sys, time, traceback

repo = Path('/workspace/localization_zoo')
out = Path('/workspace/localization-env/kitti-audit/real_fixture')
out.mkdir()
binary = repo / 'build/evaluation/pcd_dogfooding'
fixture = repo / 'evaluation/fixtures/mcd_kth_smoke'
gt = repo / 'evaluation/fixtures/mcd_kth_smoke_gt.csv'
head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()
records = []

def run(label, command, cwd, expected):
    start = time.time()
    with (out / (label + '.log')).open('w') as log:
        p = subprocess.run(command, cwd=cwd, stdout=log, stderr=subprocess.STDOUT)
    records.append({'label': label, 'command': command, 'cwd': str(cwd), 'source_commit': head,
                    'started_unix': start, 'finished_unix': time.time(), 'exit_code': p.returncode,
                    'expected_exit_code': expected})
    (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
    assert p.returncode == expected, (label, p.returncode, expected)

try:
    baseline = out / 'baseline_summary.json'
    run('baseline', [str(binary), str(fixture), str(gt), '--methods', 'kiss_icp', '--no-gt-seed',
                     '--summary-json', str(baseline)], repo, 0)
    result = next(x for x in json.loads(baseline.read_text())['methods'] if x['name'] == 'KISS-ICP')
    (out / 'inputs.json').write_text(json.dumps({
        'scope': 'isolated three-frame MCD fixture; not KITTI accuracy or a full audit',
        'source_commit': head, 'binary': str(binary), 'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
        'fixture_sha256': {str(p.relative_to(repo)): hashlib.sha256(p.read_bytes()).hexdigest()
                           for p in sorted(fixture.rglob('*')) if p.is_file()},
        'gt_sha256': hashlib.sha256(gt.read_bytes()).hexdigest(),
    }, indent=2) + '\n')
    reports = []
    for name, metric, expected in [('same', None, 0), ('ate_drift', 'ate_m', 1),
                                    ('frame_drift', 'frames', 1), ('rpe_drift', 'rpe_trans_pct', 1),
                                    ('missing_input', None, 2)]:
        case = out / name
        scripts = case / 'evaluation/scripts'
        scripts.mkdir(parents=True)
        for script in ('audit_kitti_drift.py', 'rescore_kitti_rte.py', 'capture_benchmark_environment.py'):
            shutil.copyfile(repo / 'evaluation/scripts' / script, scripts / script)
        dataset = case / 'dogfooding_results/kitti_seq_00_full'
        dataset.parent.mkdir()
        if name != 'missing_input':
            dataset.symlink_to(fixture, target_is_directory=True)
        results = case / 'experiments/results'
        results.mkdir(parents=True)
        variant = {key: result[key] for key in ('frames', 'ate_m', 'rpe_trans_pct')}
        variant.update(id='fixture_reference', status='ok', args=[])
        if metric:
            variant[metric] += 1
        aggregate = {'stable_interface': {'methods': 'kiss_icp', 'primary_method': 'KISS-ICP'},
                     'dataset': {'pcd_dir': 'dogfooding_results/kitti_seq_00_full', 'gt_csv': str(gt),
                                 'extra_args': ['--no-gt-seed']}, 'variants': [variant]}
        aggregate_path = results / 'fixture_reference_matrix.json'
        aggregate_path.write_text(json.dumps(aggregate, indent=2) + '\n')
        shutil.copyfile(aggregate_path, out / (name + '_reference.json'))
        report = out / (name + '_report.json')
        run(name, [sys.executable, str(scripts / 'audit_kitti_drift.py'), '--binary', str(binary),
                   '--data-root', str(case), '--sequence', '00', '--work-dir', str(out / (name + '_replay')),
                   '--output', str(report)], case, expected)
        payload = json.loads(report.read_text())
        assert payload['exit_code'] == expected
        row = payload['rows'][0]
        if metric:
            assert row['drift_metrics'] == [metric]
        reports.append({'case': name, 'expected_exit_code': expected, 'actual_exit_code': payload['exit_code'],
                        'audit_status': row['audit_status'], 'drift_metrics': row.get('drift_metrics'),
                        'process_exit_code': row['process_exit_code']})
    (out / 'comparison.json').write_text(json.dumps({'scope': 'tooling/exit-code validation only', 'cases': reports}, indent=2) + '\n')
    (out / 'exit.json').write_text('{"exit_code": 0}\n')
except BaseException as e:
    (out / 'exit.json').write_text(json.dumps({'exit_code': 1, 'error': str(e)}) + '\n')
    traceback.print_exc()
    raise
