"""Recompute all stage crossings and preserve the completed parent experiment."""
from pathlib import Path
import argparse
import importlib.util
import json
import csv
import math
import subprocess
import tempfile

E = Path(__file__).resolve().parent
W = E.parents[2]


def validate(history=True):
    spec = importlib.util.spec_from_file_location('failure_origin_analysis', E/'analyze.py')
    analysis = importlib.util.module_from_spec(spec); spec.loader.exec_module(analysis)
    with tempfile.TemporaryDirectory() as temporary:
        output = Path(temporary)/'report'
        summary = analysis.build(output)
        expected = {p.name: p for p in (E/'report').iterdir() if p.is_file()}
        actual = {p.name: p for p in output.iterdir() if p.is_file()}
        if actual.keys() != expected.keys(): raise ValueError('Report file coverage differs')
        def equal(a, b):
            if isinstance(a, dict): return a.keys() == b.keys() and all(equal(a[k], b[k]) for k in a)
            if isinstance(a, list): return len(a) == len(b) and all(equal(x, y) for x, y in zip(a, b))
            if isinstance(a, float): return isinstance(b, (int, float)) and math.isclose(a, b, rel_tol=1e-10, abs_tol=1e-12)
            return a == b
        for name in expected:
            if name.endswith('.json'):
                ok = equal(json.loads(expected[name].read_text()), json.loads(actual[name].read_text()))
            else:
                left = list(csv.reader(expected[name].open())); right = list(csv.reader(actual[name].open()))
                def cell(a, b):
                    if a == b: return True
                    try: return math.isclose(float(a), float(b), rel_tol=1e-10, abs_tol=1e-12)
                    except ValueError: return False
                ok = len(left) == len(right) and all(len(a) == len(b) and all(cell(x, y) for x, y in zip(a, b)) for a, b in zip(left, right))
            if not ok: raise ValueError('Committed report does not reconstruct: '+name)
    if summary['sequence_count'] != 11: raise ValueError('Missing full sequence')
    protocol = analysis.read(E/'protocol.json')
    if history:
        base = protocol['base_commit']
        allowed = ('experiments/results/inten_consensus_failure_origin/',
                   'tests/test_inten_consensus_failure_origin.py')
        changed = subprocess.check_output(['git', 'diff', '--name-only', base, '--'], cwd=W, text=True).splitlines()
        for name in changed:
            if not name.startswith(allowed) and name not in ('papers/inten_loam/README.md', 'docs/roadmap.md'):
                raise ValueError('Historical/source/publication changed: '+name)
        original = subprocess.check_output(['git', 'show', base+':papers/inten_loam/README.md'], cwd=W)
        if not (W/'papers/inten_loam/README.md').read_bytes().startswith(original):
            raise ValueError('Historical method README changed')
    return dict(exit_code=0, full_sequences=11, all_report_decisions_and_counts_reconstructed_exactly=True,
                float_comparison_relative_tolerance=1e-10, float_comparison_absolute_tolerance=1e-12,
                native_source_and_historical_results_unchanged=history,
                no_native_rerun_or_new_accuracy_setting=True)


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__); p.add_argument('--output', type=Path, required=True)
    args = p.parse_args(); args.output.write_text(json.dumps(validate(), indent=2)+'\n')
