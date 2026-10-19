"""One fixed equation-level experiment; never reads scans or updates an estimator."""
from pathlib import Path
import argparse
import csv
import gzip
import hashlib
import io
import json
import math
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]


def require(value, message):
    if not value:
        raise RuntimeError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(path, data):
    path.write_text(json.dumps(data, indent=2, allow_nan=False) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args()
    require(not args.output_dir.exists(), 'Output must be fresh')
    protocol = json.loads((HERE / 'protocol.json').read_text())
    source = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip()
    systems = []
    for path, expected in protocol['inputs'].items():
        target = REPO / path
        require(digest(target) == expected, 'Input bytes changed: ' + path)
        committed = subprocess.check_output(['git', 'show', source + ':' + path], cwd=REPO)
        require(committed == target.read_bytes(), 'Uncommitted input: ' + path)
        for row_index, line in enumerate(target.read_text().splitlines()):
            row = json.loads(line)
            require(row['step_attempted'] and row['step_finite'] and row['update_applied'], 'Unavailable observed step')
            require(row['status'] == 'ok', 'Unavailable normal equation')
            systems.append(dict(tuple=target.parent.name, stage=row['stage'], input_file=path,
                row_index=row_index, frame_index=row['frame_index'], input_index=row['input_index'],
                iteration_index=row['iteration_index'], H=row['native_H'], g=row['native_gradient'],
                delta=row['native_delta'], T=row['linearization_pose'], kind='stored_fixture'))
    identity = [float(i == j) for i in range(6) for j in range(6)]
    theta = .3
    pose = [math.cos(theta), -math.sin(theta), 0., 20.,
            math.sin(theta), math.cos(theta), 0., -10., 0., 0., 1., 3., 0., 0., 0., 1.]
    # Six independent scalar constraints, H=I. Known analytic solutions verify
    # the replay and zero-rotation finite-update control independently of data.
    for name, gradient in [('analytic_translation', [-.1, 0., 0., 0., 0., 0.]),
                           ('analytic_rotation', [0., 0., 0., 0., 0., -.02])]:
        systems.append(dict(tuple=name, stage='analytic', kind='analytic_control', H=identity,
                            g=gradient, delta=[-v / (1. + 1e-6) for v in gradient], T=pose))
    cases, lines = [], []
    for index, system in enumerate(systems):
        values = system['H'] + system['g'] + system['delta'] + system['T']
        require(all(type(x) in (float, int) and math.isfinite(x) for x in values), 'Nonfinite observation')
        require(len(system['H']) == 36 and len(system['T']) == 16, 'Invalid dimensions')
        for shift_index, shift in enumerate(protocol['shifts_m']):
            identity = f's{index}_c{shift_index}'
            cases.append(dict(id=identity, system=index, shift_m=shift,
                              shift_norm_m=math.sqrt(sum(x*x for x in shift))))
            lines.append(identity + ' ' + ' '.join(format(float(x), '.17g') for x in shift + values))
    payload = '\n'.join(lines) + '\n'
    prefix = ['env', '-u', 'DOCKER_HOST', '-u', 'DOCKER_CONTEXT', '-u', 'DOCKER_TLS',
              '-u', 'DOCKER_TLS_VERIFY', '-u', 'DOCKER_CERT_PATH', 'docker',
              '--host=unix:///var/run/docker.sock', 'exec', '-i', '--user', '1000:1000',
              '-e', 'OMP_NUM_THREADS=1', '-e', 'OPENBLAS_NUM_THREADS=1',
              'localization-zoo-dev']
    command = prefix + [str(args.binary)]
    args.output_dir.mkdir(parents=True)
    out = args.output_dir
    with gzip.open(out / 'input.txt.gz', 'wt') as stream:
        stream.write(payload)
    started = time.time()
    result = subprocess.run(command, cwd=REPO, input=payload, text=True, capture_output=True)
    with gzip.open(out / 'native.csv.gz', 'wt') as stream:
        stream.write(result.stdout)
    (out / 'native.stderr').write_text(result.stderr)
    write(out / 'command.json', dict(command=command, cwd=str(REPO), source_commit=source,
          started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
          binary=str(args.binary), binary_sha256=digest(args.binary), stdin_sha256=hashlib.sha256(payload.encode()).hexdigest()))
    require(result.returncode == 0, 'Native probe failed; output retained')
    rows = list(csv.DictReader(io.StringIO(result.stdout)))
    require(len(rows) == len(cases), 'Missing native output cases')
    metrics = [key for key in rows[0] if key not in ('id', 'raw_solve_finite', 'covariant_solve_finite')]
    groups = {}
    failures = []
    with gzip.open(out / 'cases.jsonl.gz', 'wt') as stream:
        for expected, native in zip(cases, rows):
            require(native['id'] == expected['id'], 'Changed native output ordering')
            system = systems[expected['system']]
            numbers = {key: float(native[key]) for key in metrics}
            for metric, value in numbers.items():
                if not math.isfinite(value):
                    numbers[metric] = dict(status='nonfinite', raw=native[metric])
            row = dict(**expected, tuple=system['tuple'], stage=system['stage'], kind=system['kind'],
                       metrics=numbers, raw_solve_finite=native['raw_solve_finite'] == '1',
                       covariant_solve_finite=native['covariant_solve_finite'] == '1')
            stream.write(json.dumps(row, allow_nan=False) + '\n')
            key = (system['tuple'], system['stage'], expected['shift_norm_m'])
            group = groups.setdefault(key, dict(tuple=key[0], stage=key[1], shift_norm_m=key[2], cases=0,
                                                metrics={m: dict(finite_count=0, unavailable_count=0, finite_max=None, max_case=None) for m in metrics}))
            group['cases'] += 1
            for metric, value in numbers.items():
                stat = group['metrics'][metric]
                if isinstance(value, dict):
                    stat['unavailable_count'] += 1
                    continue
                stat['finite_count'] += 1
                if stat['finite_max'] is None or value > stat['finite_max']:
                    stat.update(finite_max=value, max_case=row['id'])
            for metric, tolerance in protocol['control_tolerances'].items():
                value = numbers[metric]
                if not isinstance(value, (int, float)) or value > tolerance:
                    failures.append(dict(case=row['id'], control=metric, value=value, tolerance=tolerance))
            if system['tuple'] == 'analytic_translation' and numbers['transported_native_position_error_m'] > 1e-8:
                failures.append(dict(case=row['id'], control='zero_rotation_transport'))
    for path, expected in protocol['inputs'].items():
        require(digest(REPO / path) == expected, 'Input changed during analysis')
    with gzip.open(out / 'systems.jsonl.gz', 'wt') as stream:
        for row in systems:
            stream.write(json.dumps(row, allow_nan=False) + '\n')
    write(out / 'summary.json', dict(source_commit=source, protocol_sha256=digest(HERE / 'protocol.json'),
          command=[sys.executable, *sys.argv], cwd=str(REPO), native_exit_code=result.returncode,
          input_hashes=protocol['inputs'], observed_systems=sum(s['kind']=='stored_fixture' for s in systems),
          analytic_systems=sum(s['kind']=='analytic_control' for s in systems), cases=len(rows),
          groups=list(groups.values()), control_failures=failures, exit_code=int(bool(failures)),
          scope='Fixed equations and transported native steps only; no re-association, odometry replay, KITTI cause or accuracy finding.'))
    print(json.dumps(dict(cases=len(rows), control_failures=len(failures), exit_code=int(bool(failures)))))
    return int(bool(failures))


if __name__ == '__main__':
    raise SystemExit(main())
