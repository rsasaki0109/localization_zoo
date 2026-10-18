"""Validate passive CLI export on committed scans only; no data fetching."""
from pathlib import Path
import argparse
import hashlib
import json
import math
import re
import subprocess
import time

E = Path(__file__).resolve().parent
REPO = E.parents[2]
P = json.loads((E / 'protocol.json').read_text())
NATIVE = ['common', 'evaluation/src', 'evaluation/test', 'evaluation/CMakeLists.txt',
          ':(glob)papers/*/src/**', ':(glob)papers/*/include/**',
          ':(glob)papers/*/test/**', ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def strict_json(line):
    def reject(value):
        raise ValueError('Nonstandard numeric JSON literal: ' + value)
    return json.loads(line, parse_constant=reject)


def check_trace(path, method_path, expected_flags):
    records = [strict_json(line) for line in path.read_text().splitlines()]
    h, *frames, end = records
    assert h['type'] == 'header' and h['schema'] == 'localization_zoo.inten_trace' and h['version'] == 1
    assert h['matrix_layout'] == 'row_major_flat'
    assert h['input_frames'] == 3 and len(frames) == 3
    assert end == dict(type='end', processed_frames=3, skipped_frames=0)
    assert all(flag in h['argv'] for flag in expected_flags)
    declarations = (REPO / 'papers/inten_loam/include/inten_loam/inten_loam_paper.h').read_text()
    declarations = declarations.split('struct InTenLoamPaperParams {', 1)[1].split('\n};', 1)[0]
    keys = re.findall(r'^\s*(?:int|double|bool)\s+(\w+)\s*=', declarations, re.M)
    assert set(h['params']) == set(keys), (set(keys) - set(h['params']), set(h['params']) - set(keys))
    for key in ('use_intensity', 'use_mapping', 'use_dor'):
        assert isinstance(h['params'][key], bool)
    pose_rows = [[float(v) for v in row.split()] for row in method_path.read_text().splitlines()]
    assert len(pose_rows) == 3
    iterations, stops, max_pose_rounding = 0, {}, 0.0
    for index, (frame, pose) in enumerate(zip(frames, pose_rows)):
        assert frame['type'] == 'frame' and frame['input_index'] == frame['frame_index'] == index
        assert frame['initialization'] == (index == 0)
        assert frame['scan_path'].endswith('/cloud.pcd')
        for key in ('pose', 'anchored_pose', 'accepted_relative', 's2m_prediction'):
            assert len(frame[key]) == 16 and all(math.isfinite(v) for v in frame[key])
        for a, b in zip(frame['anchored_pose'][:12], pose):
            assert math.isclose(a, b, rel_tol=5e-9, abs_tol=1e-8), (a, b)
            max_pose_rounding = max(max_pose_rounding, abs(a - b))
        for stage in ('s2s', 's2m'):
            s = frame[stage]
            stops[s['stop']] = stops.get(s['stop'], 0) + 1
            assert s['called'] == (index != 0)
            assert len(s['input']) == len(s['output']) == 16
            if not s['called']:
                assert s['stop'] == 'not_run' and not s['iterations']
            else:
                assert s['stop'] in {'no_iterations', 'insufficient_pairs', 'nonfinite_step', 'converged', 'iteration_limit'}
            assert len(s['iterations']) <= h['params'][stage + '_iterations']
            for i, it in enumerate(s['iterations']):
                iterations += 1
                assert it['index'] == i and sum(it['class_pairs']) == it['pairs']
                assert len(it['class_pairs']) == 4 and len(it['H']) == 36
                assert len(it['g']) == len(it['delta']) == 6 and len(it['linearization_pose']) == 16
                assert not it['update_applied'] or (it['step_attempted'] and it['step_finite'])
                for values in (it['H'], it['g'], it['delta'], it['linearization_pose'], [it['weighted_squared_residual']]):
                    assert all((isinstance(v, (int, float)) and math.isfinite(v)) or
                               v in ('NaN', '+Infinity', '-Infinity') for v in values)
        if index:
            assert frame['s2s_fallback'] == (frame['s2s']['returned_pairs'] < 30)
            assert frame['s2m_accepted'] == (frame['s2m']['returned_pairs'] >= 30)
    return dict(records=len(records), processed_frames=len(frames), iterations=iterations,
                stops=stops, max_saved_pose_rounding_abs=max_pose_rounding, strict_json=True,
                effective_parameter_keys=keys, trace_sha256=sha(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', required=True, type=Path)
    parser.add_argument('--candidate-binary', type=Path)
    args = parser.parse_args()
    out = args.output_dir.resolve()
    assert not out.exists(), 'Use a fresh directory.'
    reference = Path(P['validation']['reference_binary'])
    candidate = (args.candidate_binary or REPO / 'build/evaluation/pcd_dogfooding').resolve()
    assert sha(reference) == P['validation']['reference_binary_sha256']
    subprocess.run(['git', 'diff', '--exit-code', P['base_commit'], P['validation']['reference_native_source_commit'], '--',
                    *NATIVE], cwd=REPO, check=True)
    subprocess.run(['git', 'diff', '--exit-code', P['native_source_commit'], '--', *NATIVE], cwd=REPO, check=True)
    source_head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    binaries = {name: dict(path=str(p), sha256=sha(p)) for name, p in [('reference', reference), ('candidate', candidate)]}
    out.mkdir(parents=True)
    fixed = next(v['args'] for v in json.loads((REPO / 'experiments/inten_loam_kitti_seq_07_full_paper_comp_matrix.json').read_text())['variants']
                 if v['id'] == 'paper_odometry_plane_check')
    prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS', '-u', 'DOCKER_TLS_VERIFY',
              '-u', 'DOCKER_CERT_PATH', 'docker', '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
              '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1']
    receipts, comparisons = [], []
    def run(folder, binary, flags, expected=0, trace=None, summary=None):
        folder.mkdir(parents=True)
        summary = summary or folder / 'summary.json'
        argv = [str(binary), str(REPO / P['validation']['fixture']), str(REPO / P['validation']['gt']), '3',
                '--summary-json', str(summary), *flags]
        if trace is not None:
            argv += ['--inten-loam-paper-trace-jsonl', str(trace)]
        command = prefix + ['-w', str(folder), 'localization-zoo-dev', *argv]
        start = time.time()
        with (folder / 'native.log').open('w') as f:
            r = subprocess.run(command, cwd=REPO, stdout=f, stderr=subprocess.STDOUT)
        receipt = dict(command=command, native_argv=argv, cwd=str(REPO), native_cwd=str(folder),
                       source_head=source_head, binary_sha256=sha(binary), exit_code=r.returncode,
                       expected_exit_code=expected, started_unix=start, finished_unix=time.time())
        (folder / 'command.json').write_text(json.dumps(receipt, indent=2) + '\n')
        receipts.append(receipt)
        assert r.returncode == expected, receipt
        return json.loads(summary.read_text()) if expected == 0 else None
    for variant in P['validation']['variants']:
        flags = ['--no-gt-seed', '--inten-loam-paper-odometry'] if variant == 'paper_default' else fixed
        results = {}
        for name, binary in [('reference', reference), ('candidate_off', candidate), ('candidate_on', candidate)]:
            folder = out / variant / name
            results[name] = run(folder, binary, ['--methods', 'inten_loam', *flags],
                                trace=folder / 'trace.jsonl' if name == 'candidate_on' else None)
        metrics, unavailable, paths = {}, [], {}
        for name, summary in results.items():
            m = summary['methods'][0]
            assert m['status'] == 'ok' and m['frames'] == 3
            for key in ['ate_m', 'rpe_trans_pct', 'rpe_rot_deg_per_m', 'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m', 'frames']:
                a = results['reference']['methods'][0][key]
                assert a == m[key], (variant, name, key, a, m[key])
                if a is None:
                    if key not in unavailable: unavailable.append(key)
                else: metrics.setdefault(name, {})[key] = abs(a - m[key])
            paths[name] = {p.name: p for p in (out / variant / name / 'dogfooding_results').glob('*.txt')}
        required = {'InTEn_LOAM.txt', 'gt.txt', Path(results['reference']['artifacts']['evaluated_gt_path']).name}
        assert required <= paths['reference'].keys()
        pose_checks = {}
        for name in ('candidate_off', 'candidate_on'):
            assert paths[name].keys() == paths['reference'].keys()
            pose_checks[name] = {}
            for key, ref in paths['reference'].items():
                assert ref.read_bytes() == paths[name][key].read_bytes(), (variant, name, key)
                pose_checks[name][key] = dict(identical=True, sha256=sha(ref))
        trace = check_trace(out / variant / 'candidate_on/trace.jsonl', paths['candidate_on']['InTEn_LOAM.txt'], flags)
        assert not list((out / variant / 'reference').glob('*.jsonl'))
        assert not list((out / variant / 'candidate_off').glob('*.jsonl'))
        comparisons.append(dict(variant=variant, flags=flags, metric_abs_delta=metrics, unavailable_metrics=unavailable,
                                poses=pose_checks, trace=trace))
    negative = out / 'negative'
    common = ['--methods', 'inten_loam', '--no-gt-seed', '--inten-loam-paper-odometry']
    cases = [
        ('missing_path', [*common, '--inten-loam-paper-trace-jsonl'], None, 1),
        ('wrong_method', ['--methods', 'mesh_loam', '--inten-loam-paper-odometry'], negative / 'wrong_method/trace.jsonl', 1),
        ('missing_paper', ['--methods', 'inten_loam'], negative / 'missing_paper/trace.jsonl', 1),
        ('wrong_extension', common, negative / 'wrong_extension/trace.txt', 1),
        ('missing_parent', common, negative / 'missing_parent/absent/trace.jsonl', 2),
    ]
    for name, flags, trace, expected in cases:
        run(negative / name, candidate, flags, expected, trace=trace)
    folder = negative / 'existing'; folder.mkdir(parents=True)
    trace = folder / 'trace.jsonl'; trace.write_bytes(b'preserve existing trace\n')
    before = trace.read_bytes()
    run(folder / 'run', candidate, common, 2, trace)
    assert trace.read_bytes() == before
    folder = negative / 'summary_collision'
    trace = folder / 'trace.jsonl'
    run(folder, candidate, common, 1, trace, trace)
    assert not trace.exists()
    for name, binary in [('reference', reference), ('candidate', candidate)]:
        assert sha(binary) == binaries[name]['sha256']
    inputs = {str(p.relative_to(REPO)): sha(p) for p in (REPO / P['validation']['fixture']).glob('*/cloud.pcd')}
    inputs[P['validation']['gt']] = sha(REPO / P['validation']['gt'])
    summary = dict(exit_code=0, native_source_commit=P['native_source_commit'], recorded_head=source_head,
                   binaries=binaries, input_sha256=inputs, comparisons=comparisons,
                   successful_native_exits=[r['exit_code'] for r in receipts if r['expected_exit_code'] == 0],
                   negative_expected_exits=[r['exit_code'] for r in receipts if r['expected_exit_code'] != 0],
                   scope=P['scope'])
    (out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('Default-off, trace-on fixture parity and negative CLI cases verified.')


if __name__ == '__main__':
    main()
