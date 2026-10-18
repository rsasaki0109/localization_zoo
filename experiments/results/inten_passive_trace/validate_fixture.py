"""Compare tracing-disabled compatibility on committed scans; never fetch data."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import time

E = Path(__file__).resolve().parent
REPO = E.parents[2]
PROTOCOL = json.loads((E / 'protocol.json').read_text())


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    out = args.output_dir.resolve()
    assert not out.exists(), 'Use a fresh output directory.'
    fixture = PROTOCOL['fixture']
    reference = Path(fixture['reference_binary'])
    candidate = REPO / 'build/evaluation/pcd_dogfooding'
    assert sha(reference) == fixture['reference_binary_sha256']
    native_paths = ['common', 'evaluation/src', 'evaluation/CMakeLists.txt',
                    'papers/*/src', 'papers/*/include', 'papers/*/test',
                    'papers/*/CMakeLists.txt', 'CMakeLists.txt']
    subprocess.run(['git', 'diff', '--exit-code', PROTOCOL['base_commit'],
                    fixture['reference_binary_source_commit'], '--', *native_paths],
                   cwd=REPO, check=True)
    subprocess.run(['git', 'diff', '--exit-code', PROTOCOL['native_source_commit'],
                    '--', *native_paths], cwd=REPO, check=True)
    recorded_head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    binaries = {name: dict(path=str(path), sha256=sha(path))
                for name, path in [('reference', reference), ('candidate', candidate)]}
    out.mkdir(parents=True)
    receipts, comparisons = [], []
    variants = json.loads((REPO / 'experiments/inten_loam_kitti_seq_07_full_paper_comp_matrix.json').read_text())
    fixed = next(v['args'] for v in variants['variants'] if v['id'] == 'paper_odometry_plane_check')
    prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
              '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
              '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
              '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1']
    for variant in fixture['variants']:
        flags = ['--no-gt-seed', '--inten-loam-paper-odometry'] if variant == 'paper_default' else fixed
        results = {}
        for name, binary in [('reference', reference), ('candidate', candidate)]:
            folder = out / variant / name
            folder.mkdir(parents=True)
            summary = folder / 'summary.json'
            argv = [str(binary), str(REPO / fixture['data']), str(REPO / fixture['gt']),
                    '3', '--methods', 'inten_loam', '--summary-json', str(summary), *flags]
            command = prefix + ['-w', str(folder), 'localization-zoo-dev', *argv]
            start = time.time()
            with (folder / 'native.log').open('w') as log:
                result = subprocess.run(command, cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
            receipt = dict(command=command, native_argv=argv, cwd=str(REPO), native_cwd=str(folder),
                           binary=binaries[name], variant=variant, recorded_head=recorded_head,
                           exit_code=result.returncode, started_unix=start, finished_unix=time.time())
            (folder / 'command.json').write_text(json.dumps(receipt, indent=2) + '\n')
            receipts.append(receipt)
            assert result.returncode == 0, receipt
            results[name] = json.loads(summary.read_text())
        a, b = (results[name]['methods'][0] for name in ('reference', 'candidate'))
        fields = ['ate_m', 'rpe_trans_pct', 'rpe_rot_deg_per_m', 'kitti_rte_trans_pct',
                  'kitti_rte_rot_deg_per_100m', 'frames']
        delta, unavailable = {}, []
        for key in fields:
            if a[key] is None or b[key] is None:
                assert a[key] == b[key]
                unavailable.append(key)
            else:
                delta[key] = abs(a[key] - b[key])
                assert delta[key] == 0, (variant, key, a[key], b[key])
        assert a['status'] == b['status'] == 'ok'
        paths = {}
        for name in ('reference', 'candidate'):
            paths[name] = {p.name: p for p in (out / variant / name / 'dogfooding_results').glob('*.txt')}
        assert paths['reference'].keys() == paths['candidate'].keys() and paths['reference']
        required = {'InTEn_LOAM.txt', 'gt.txt',
                    Path(results['reference']['artifacts']['evaluated_gt_path']).name}
        assert required <= paths['reference'].keys(), required
        pose_checks = {}
        for key, path in paths['reference'].items():
            other = paths['candidate'][key]
            assert path.read_bytes() == other.read_bytes(), (variant, key)
            pose_checks[key] = dict(identical=True, sha256=sha(path))
        comparisons.append(dict(variant=variant, flags=flags, metric_abs_delta=delta,
                                unavailable_metrics=unavailable, poses=pose_checks))
    for name, path in [('reference', reference), ('candidate', candidate)]:
        assert sha(path) == binaries[name]['sha256']
    inputs = {p.relative_to(REPO).as_posix(): sha(p) for p in (REPO / fixture['data']).glob('*/cloud.pcd')}
    inputs[fixture['gt']] = sha(REPO / fixture['gt'])
    (out / 'summary.json').write_text(json.dumps(dict(exit_code=0,
        source_commit=PROTOCOL['native_source_commit'], recorded_head=recorded_head,
        binaries=binaries, input_sha256=inputs, native_exits=[r['exit_code'] for r in receipts],
        comparisons=comparisons, scope=fixture['comparison']), indent=2) + '\n')
    print('Tracing-disabled fixture compatibility verified; this is not full-sequence accuracy evidence.')


if __name__ == '__main__':
    main()
