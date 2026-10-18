"""Compare pre/post API integration in each execution mode; never fetch scans."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[3]
P = json.loads((E / 'protocol.json').read_text())


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    out = args.output_dir.resolve()
    assert not out.exists()
    reference = Path(P['reference_binary'])
    candidate = W / 'build/evaluation/pcd_dogfooding'
    assert digest(reference) == P['reference_sha256']
    native = ['common', 'evaluation/src', 'evaluation/CMakeLists.txt',
        ':(glob)papers/*/src/**', ':(glob)papers/*/include/**', ':(glob)papers/*/test/**',
        ':(glob)papers/*/CMakeLists.txt', 'CMakeLists.txt']
    for refs in [(P['integration_base'], P['reference_source_commit']),
                 (P['native_source_commit'],)]:
        subprocess.run(['git', 'diff', '--exit-code', *refs, '--', *native], cwd=W, check=True)
    binaries = {name: dict(path=str(b), sha256=digest(b))
                for name, b in [('reference', reference), ('candidate', candidate)]}
    selected = json.loads((W / 'experiments/inten_loam_kitti_seq_07_full_paper_comp_matrix.json').read_text())
    fixed = next(v['args'] for v in selected['variants'] if v['id'] == 'paper_odometry_plane_check')
    prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
        '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
        '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
        '-e', 'OMP_NUM_THREADS=4', '-e', 'OPENBLAS_NUM_THREADS=1']
    out.mkdir(parents=True)
    comparisons, exits = [], []
    for profile in P['registration_profiles']:
        flags = ['--no-gt-seed', '--inten-loam-paper-odometry'] if profile == 'paper_default' else fixed
        for mode in P['execution_modes']:
            execution = ['--paper-pipeline-accelerated'] if mode == 'accelerated' else []
            paths, summaries = {}, {}
            for name, binary in [('reference', reference), ('candidate', candidate)]:
                folder = out / profile / mode / name
                folder.mkdir(parents=True)
                summary = folder / 'summary.json'
                argv = [str(binary), str(W / P['fixture_data']), str(W / P['fixture_gt']),
                    '3', '--methods', 'inten_loam', '--summary-json', str(summary), *flags, *execution]
                command = prefix + ['-w', str(folder), 'localization-zoo-dev', *argv]
                start = time.time()
                with (folder / 'native.log').open('w') as log:
                    result = subprocess.run(command, cwd=W, stdout=log, stderr=subprocess.STDOUT)
                receipt = dict(command=command, native_argv=argv, cwd=str(W), native_cwd=str(folder),
                    binary=binaries[name], source_commit=P['native_source_commit'],
                    exit_code=result.returncode, started_unix=start, finished_unix=time.time())
                (folder / 'command.json').write_text(json.dumps(receipt, indent=2) + '\n')
                exits.append(result.returncode)
                assert result.returncode == 0, receipt
                summaries[name] = json.loads(summary.read_text())['methods'][0]
                paths[name] = {p.name: p for p in (folder / 'dogfooding_results').glob('*.txt')}
            a, b = summaries['reference'], summaries['candidate']
            assert a['status'] == b['status'] == 'ok'
            deltas, unavailable = {}, []
            for key in ['ate_m', 'rpe_trans_pct', 'rpe_rot_deg_per_m', 'frames',
                        'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m']:
                if a[key] is None or b[key] is None:
                    assert a[key] == b[key]
                    unavailable.append(key)
                else:
                    deltas[key] = abs(a[key] - b[key])
                    assert deltas[key] == 0, (profile, mode, key)
            assert paths['reference'].keys() == paths['candidate'].keys()
            assert {'InTEn_LOAM.txt', 'gt.txt'} <= paths['reference'].keys()
            poses = {}
            for name, path in paths['reference'].items():
                assert path.read_bytes() == paths['candidate'][name].read_bytes(), (profile, mode, name)
                poses[name] = dict(identical=True, sha256=digest(path))
            comparisons.append(dict(profile=profile, execution_mode=mode, flags=flags + execution,
                metric_abs_delta=deltas, unavailable_metrics=unavailable, poses=poses))
    for name, b in [('reference', reference), ('candidate', candidate)]:
        assert digest(b) == binaries[name]['sha256']
    files = list((W / P['fixture_data']).glob('*/cloud.pcd'))
    files += [W / P['fixture_data'] / 'frame_timestamps.csv', W / P['fixture_gt']]
    inputs = {p.relative_to(W).as_posix(): digest(p) for p in files}
    (out / 'summary.json').write_text(json.dumps(dict(exit_code=0,
        native_source_commit=P['native_source_commit'], integration_base=P['integration_base'],
        reference_source_commit=P['reference_source_commit'], binaries=binaries,
        input_sha256=inputs, native_exits=exits, comparisons=comparisons,
        scope='Each execution mode is compared to its own pre-API reference. No fast/slow equivalence assumption, speed or full-sequence accuracy claim.'), indent=2) + '\n')
    print('Integrated default/accelerated fixture compatibility verified.')


if __name__ == '__main__':
    main()
