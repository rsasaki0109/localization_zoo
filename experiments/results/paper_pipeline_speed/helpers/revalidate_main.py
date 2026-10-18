"""Check main integration against its frozen executable; no speed measurement."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import time

REPO = Path(__file__).resolve().parents[4]
E = REPO / 'experiments/results/paper_pipeline_speed'
BASE = '7505a2914328d53ac9b319d5f8042189b378d457'
BASE_SOURCE = '0b079dbbcb7c6f58c1d8367ade881c15f003e525'
BASE_PIN = Path('/workspace/localization_zoo/experiments/results/shared_near_range/candidate.json')
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output-dir', type=Path, required=True)
options = parser.parse_args()
out = options.output_dir.resolve()
out.mkdir(parents=True, exist_ok=False)
source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
docker = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
          '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
          '--host=unix:///var/run/docker.sock', 'exec', '--user', '1000:1000',
          '-e', 'OMP_NUM_THREADS=4', '-e', 'OMP_DYNAMIC=FALSE', '-e', 'OPENBLAS_NUM_THREADS=1',
          '-w', str(REPO), 'localization-zoo-dev']
records = []


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def checked(label, command):
    started = time.time()
    with (out / (label + '.log')).open('w') as log:
        result = subprocess.run(command, cwd=REPO, stdout=log, stderr=subprocess.STDOUT)
    records.append(dict(label=label, command=command, cwd=str(REPO), source_commit=source,
                        started_unix=started, finished_unix=time.time(), exit_code=result.returncode))
    (out / 'commands.json').write_text(json.dumps(records, indent=2) + '\n')
    print(label, result.returncode, flush=True)
    if result.returncode:
        raise SystemExit(result.returncode)


try:
    native_paths = ['common', 'papers/*/src', 'papers/*/include', 'papers/*/test',
                    'papers/*/CMakeLists.txt', 'evaluation/src', 'evaluation/CMakeLists.txt', 'CMakeLists.txt']
    checked('main_matches_frozen_native_source', ['git', 'diff', '--exit-code', BASE_SOURCE, BASE, '--', *native_paths])
    checked('source_identity', ['git', 'diff', '--exit-code', source, '--', *native_paths])
    checked('mesh_unchanged', ['git', 'diff', '--exit-code', BASE, '--',
            'papers/mesh_loam/src', 'papers/mesh_loam/include', 'papers/mesh_loam/test',
            'papers/mesh_loam/CMakeLists.txt'])
    pin = json.loads(BASE_PIN.read_text())
    baseline = Path(pin['binary'])
    assert digest(baseline) == pin['sha256']
    checked('configure', docker + ['cmake', '-S', '.', '-B', 'build',
                                  '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_PYTHON_BINDINGS=OFF'])
    checked('build_affected_targets', docker + ['cmake', '--build', 'build', '-j2', '--target',
            'pcd_dogfooding', 'test_imls_paper_odometry', 'test_inten_loam_paper',
            'test_mesh_loam', 'test_initial_registration'])
    checked('ctest_affected', docker + ['ctest', '--test-dir', 'build', '--output-on-failure',
            '-R', '^(test_imls_paper_odometry|test_inten_loam_paper|test_mesh_loam|initial_registration)$'])
    binary = REPO / 'build/evaluation/pcd_dogfooding'
    candidate_hash = digest(binary)
    configs = json.loads((E / 'protocol.json').read_text())['method_configs']
    names = {'imls_slam': 'IMLS_SLAM', 'inten_loam': 'InTEn_LOAM', 'mesh_loam': 'Mesh_LOAM'}
    comparisons = {}
    for method, config in configs.items():
        runs = {}
        for mode in ('baseline', 'default', 'accelerated'):
            label = method + '_' + mode
            folder = out / label
            folder.mkdir()
            cwd = REPO / 'dogfooding_results/main_refresh_fixture' / label
            cwd.mkdir(parents=True, exist_ok=False)
            cmd = docker[:]
            cmd[cmd.index('-w') + 1] = str(cwd)
            cmd += [str(baseline if mode == 'baseline' else binary),
                    str(REPO / 'evaluation/fixtures/mcd_kth_smoke'),
                    str(REPO / 'evaluation/fixtures/mcd_kth_smoke_gt.csv'),
                    '--methods', method, '--summary-json', str(folder / 'summary.json'), *config['args']]
            if mode == 'accelerated':
                cmd.append('--paper-pipeline-accelerated')
            checked(label, cmd)
            summary = json.loads((folder / 'summary.json').read_text())
            row = summary['methods'][0]
            assert row['status'] == 'ok' and row['frames'] == 3
            enabled = 'repository acceleration enabled' in row['note']
            assert enabled == (mode == 'accelerated' and method != 'mesh_loam')
            shutil.copy2(cwd / 'dogfooding_results' / (names[method] + '.txt'), folder / 'poses.txt')
            runs[mode] = row
        checks = {}
        for mode in ('default', 'accelerated'):
            fields = ('ate_m', 'rpe_trans_pct', 'rpe_rot_deg_per_m', 'frames',
                      'kitti_rte_trans_pct', 'kitti_rte_rot_deg_per_100m')
            deltas = {key: (None if runs[mode][key] is None and runs['baseline'][key] is None
                           else abs(runs[mode][key] - runs['baseline'][key])) for key in fields}
            poses_equal = (out / (method + '_baseline') / 'poses.txt').read_bytes() == (out / (method + '_' + mode) / 'poses.txt').read_bytes()
            assert poses_equal and all(v is None or v <= 1e-9 for v in deltas.values())
            checks[mode] = dict(metric_abs_delta=deltas, saved_pose_bytes_equal=poses_equal)
        comparisons[method] = checks
    inputs = [*REPO.glob('evaluation/fixtures/mcd_kth_smoke/*/cloud.pcd'),
              REPO / 'evaluation/fixtures/mcd_kth_smoke/frame_timestamps.csv',
              REPO / 'evaluation/fixtures/mcd_kth_smoke_gt.csv']
    result = dict(source_commit=source, base_main=BASE, baseline_native_source=BASE_SOURCE,
                  baseline_binary=str(baseline), baseline_sha256=pin['sha256'],
                  candidate_binary=str(binary), candidate_sha256=candidate_hash,
                  fixture_sha256={str(p.relative_to(REPO)): digest(p) for p in inputs},
                  configs=configs, comparisons=comparisons, exit_code=0,
                  scope='Real-fixture compatibility only. Reuse the historical fixed registration arguments, including IMLS coarse_frames=1. No speed comparison or new full-sequence claim. Saved-pose equality is limited to serialization precision; official long-segment KITTI RTE is not established by three scans. Native provenance uses executable hashes/source verification, not evaluator_git_commit write-time metadata.')
    assert digest(baseline) == pin['sha256'] and digest(binary) == candidate_hash
    (out / 'comparison.json').write_text(json.dumps(result, indent=2) + '\n')
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=0, source_commit=source)) + '\n')
except BaseException as error:
    code = error.code if isinstance(error, SystemExit) and isinstance(error.code, int) else 1
    (out / 'exit.json').write_text(json.dumps(dict(exit_code=code, error=str(error))) + '\n')
    raise
