"""Measure strict ratio-comparison drift under an explicitly selected BLAS kernel.

Only this diagnostic process observes failing math.isclose calls. Native code,
stored results and all exact label/decision checks remain unchanged. The bypass
is restricted to the original 1e-12 comparison, and every bypass is recorded.
"""
from pathlib import Path
import argparse
import importlib.util
import inspect
import json
import math
import os
import platform
import tempfile
import numpy as np

E = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__); p.add_argument('--output', type=Path, required=True)
args = p.parse_args()
spec = importlib.util.spec_from_file_location('kernel_origin_analysis', E/'analyze.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
original = math.isclose
drifts = []; maximum = 0.; calls = 0


def observe(a, b, **kw):
    global maximum, calls
    if kw != dict(rel_tol=1e-12, abs_tol=1e-12):
        raise ValueError('Unexpected comparison in frozen diagnostic probe')
    calls += 1; maximum = max(maximum, abs(a-b))
    if original(a, b, **kw): return True
    frame = inspect.currentframe().f_back.f_locals
    drifts.append(dict(sequence=frame['seq'], mode=frame['mode'], frame=frame['i'],
                       metric=frame['key'], committed=a, reconstructed=b, absolute_difference=abs(a-b)))
    # Never silently permit an unbounded discrepancy, even in the probe.
    if not original(a, b, rel_tol=1e-10, abs_tol=1e-12):
        raise ValueError('Difference exceeds the existing documented portable bound')
    return True


math.isclose = observe
try:
    with tempfile.TemporaryDirectory() as temporary:
        summary = m.build(Path(temporary)/'report')
    previous = json.loads((E/'report/summary.json').read_text())
    assert summary == previous, 'Stage decisions/counts/native metrics changed under kernel override'
finally:
    math.isclose = original
result = dict(exit_code=0, kernel_override=os.environ.get('OPENBLAS_CORETYPE'),
              numpy=np.__version__, python=platform.python_version(), ratio_comparisons=calls,
              max_absolute_difference=maximum, strict_failures=drifts,
              all_labels_counts_and_native_metrics_exact=True,
              all_differences_within_documented_relative_1e_minus10_absolute_1e_minus12=True,
              scope='Offline portability probe only. No estimator, result, threshold or classification is modified.')
args.output.write_text(json.dumps(result, indent=2, allow_nan=False)+'\n')
print('Completed BLAS kernel portability probe', result['kernel_override'], 'strict differences', len(drifts))
