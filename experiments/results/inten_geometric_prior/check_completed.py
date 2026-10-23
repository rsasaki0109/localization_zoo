#!/usr/bin/env python3
"""Reconstruct full-sequence controls and scores from saved native output."""
import argparse
import gzip
import itertools
import json
from pathlib import Path
import sys
import numpy as np
from receipt import save, sha
from run_sequences import archived_frames


def main():
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--sequence', action='append', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    study = Path(__file__).resolve().parent
    sys.path.insert(0, str(args.source_root / 'evaluation/scripts'))
    from diagnose_kitti_steps import diagnose
    from evaluate_external_kitti_odometry import load_reference_csv
    binary = json.loads((study / 'binaries.json').read_text())
    reports = []
    for sequence in args.sequence:
        assert sequence in json.loads((study / 'protocol.json').read_text())['sequence_order']
        folder = study / 'sequences' / sequence
        command = json.loads((folder / 'native_command.json').read_text())
        summary = json.loads((folder / 'summary.json').read_text())
        archive = json.loads((folder / 'archive.json').read_text())
        for name, entry in archive.items():
            assert Path(name).name == name
            path = folder / name
            assert path.stat().st_size == entry['bytes'] and sha(path) == entry['sha256'], name
        assert command['exit_code'] == summary['native_exit_code'] == 0
        assert command['binary_sha256'] == binary['runner_sha256']
        assert command['native_source_commit'] == binary['source_commit']
        original = args.source_root / 'experiments/results/inten_facade_pca/sequences' / sequence
        assert command['input_manifest_sha256'] == sha(original / 'inputs.json')
        pins = json.loads((original / 'inputs.json').read_text())['files']
        count = len([name for name in pins if name.endswith('.bin')])
        assert command['frames_completed'] == summary['frames'] == count
        trace = original / 'candidate/trace.jsonl.parts'
        assert command['trace_shards'] == {p.name: sha(p) for p in sorted(trace.glob('*.gz'))}
        inputs = [json.loads(line) for line in (folder / 'inputs.jsonl').read_text().splitlines()]
        exported = {name: np.loadtxt(folder / (name + '.txt')) for name in ('reference', 'candidate')}
        poses = {name: [] for name in exported}
        maximum = 0.0
        seen = 0
        with gzip.open(folder / 'native.jsonl.gz', 'rt') as stream:
            rows = (json.loads(line) for line in stream)
            for i, (row, raw, old) in enumerate(itertools.zip_longest(rows, inputs, archived_frames(trace))):
                assert row is not None and raw is not None and old is not None
                assert row['frame'] == raw['frame'] == old['input_index'] == i
                pin = pins[f'{i:06d}.bin']
                assert raw['sha256'] == pin['sha256'] and raw['bytes'] == pin['bytes']
                error = float(np.max(np.abs(np.asarray(row['reference']['pose']) - old['pose'])))
                assert error == raw['baseline_max_abs_error'] and error <= 1e-8
                maximum = max(maximum, error)
                for name in poses:
                    pose = np.asarray(row[name]['pose']).reshape(4, 4)
                    assert np.isfinite(pose).all()
                    assert np.array_equal(pose[:3].ravel(), exported[name][i])
                    poses[name].append(pose)
                seen += 1
        assert seen == count and all(len(value) == count for value in exported.values())
        assert maximum == command['maximum_baseline_matrix_error'] and summary['controls_valid']
        gtpath = args.source_root / 'experiments/reference_data' / f'kitti_seq_{sequence}_full_gt.csv'
        assert sha(gtpath) == summary['gt_sha256']
        truth = load_reference_csv(gtpath)
        assert len(truth) == count
        for name, trajectory in poses.items():
            # Same saved poses, evaluator, interpreter and inputs: require the
            # complete diagnostic object, not just a selected score, to match.
            assert diagnose(np.asarray(trajectory), truth) == summary['methods'][name], name
        reports.append({'sequence': sequence, 'frames': seen, 'all_controls_valid': True,
                        'maximum_baseline_matrix_error': maximum,
                        'all_saved_diagnostics_reconstructed': True,
                        'summary_sha256': sha(folder / 'summary.json'),
                        'archive_sha256': sha(folder / 'archive.json')})
    save(args.output, {'sequences': reports, 'all_requested_checks_passed': True})


if __name__ == '__main__':
    main()
