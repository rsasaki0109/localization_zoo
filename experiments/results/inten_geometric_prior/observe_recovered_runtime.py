#!/usr/bin/env python3
"""Snapshot a live controller without changing its inputs or native state."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    study = Path(__file__).resolve().parent
    args.output.mkdir(parents=True, exist_ok=False)
    source = study / 'sequences/00'
    # Read only complete, flushed lines. This is an observation, not a final audit.
    data = (source / 'inputs.jsonl').read_bytes()
    data = data[:data.rfind(b'\n') + 1]
    rows = [json.loads(line) for line in data.splitlines()]
    pins_path = args.source_root / 'experiments/results/inten_facade_pca/sequences/00/inputs.json'
    pins = json.loads(pins_path.read_text())['files']
    assert rows and [row['frame'] for row in rows] == list(range(len(rows)))
    for row in rows:
        pin = pins[f"{row['frame']:06d}.bin"]
        assert row['sha256'] == pin['sha256'] and row['bytes'] == pin['bytes']
    (args.output / 'inputs_observed.jsonl').write_bytes(data)
    observed = {'observed_unix': time.time(), 'sequence': '00',
                'complete_input_records_observed': len(rows),
                'last_input_frame_observed': rows[-1]['frame'],
                'maximum_recorded_baseline_matrix_error': max(row['baseline_max_abs_error'] for row in rows),
                'input_manifest_sha256': hashlib.sha256(pins_path.read_bytes()).hexdigest(),
                'full_sequence_complete': False,
                'interpretation': 'Live acquisition progress only; no complete accuracy result or independent native audit.'}
    processes = subprocess.run(['ps', '-eo', 'pid,ppid,stat,args'], capture_output=True, text=True, check=True)
    observed['processes'] = [line for line in processes.stdout.splitlines()
                            if ('/run_sequences.py ' in line or '/build/stream_runner' in line)
                            and 'sandbox' not in line and 'bash -' not in line]
    for src, name in [
        (source / 'native_command.json', 'native_command_observed.json'),
        (study / 'commands/full_trial_extended_acquisition_wait.json', 'controller_receipt_observed.json'),
        (study / 'commands/full_trial_extended_acquisition_wait.stdout', 'controller_stdout_observed.txt'),
    ]:
        (args.output / name).write_bytes(src.read_bytes())
    failed = sorted(source.glob('fetch_*.json'), key=lambda p: p.stat().st_mtime)
    if failed:
        (args.output / failed[-1].name).write_bytes(failed[-1].read_bytes())
    argv = ['docker', 'exec', 'localization-zoo-dev', 'sha256sum',
            json.loads((study / 'binaries.json').read_text())['runner_container_path']]
    binary = subprocess.run(argv, capture_output=True, text=True)
    observed['binary_check'] = {'argv': argv, 'exit_code': binary.returncode,
                                'stdout': binary.stdout, 'stderr': binary.stderr}
    assert binary.returncode == 0
    assert binary.stdout.split()[0] == json.loads((study / 'binaries.json').read_text())['runner_sha256']
    observed['files'] = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in args.output.iterdir()}
    (args.output / 'observation.json').write_text(json.dumps(observed, indent=2) + '\n')
    print(json.dumps(observed, indent=2))


if __name__ == '__main__':
    main()
