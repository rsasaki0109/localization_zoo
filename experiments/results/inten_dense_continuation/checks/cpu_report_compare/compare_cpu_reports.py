from pathlib import Path
import sys
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_dense_continuation')
sys.path.insert(0,str(E))
from common import *
V=module('dense_cpu_compare',E/'verify.py')
checks={}
for study in ('inten_iteration_budget','inten_dense_continuation'):
    expected=read(E.parent/study/'report/summary.json')
    for cpu in ('Haswell','Sandybridge'):
        actual=R/'final_checks_stage'/(study+'_'+cpu+'.json')
        V.V.V.V.equivalent(read(actual),expected)
        checks[study+'_'+cpu]=dict(exit_code=0,report_sha256=sha(actual),reference_sha256=sha(E.parent/study/'report/summary.json'))
write(R/'cpu_compare_stage/result.json',dict(exit_code=0,original_relative_tolerance=1e-10,original_absolute_tolerance=1e-12,checks=checks))
print('All four CPU reports match their committed numerical references with original tolerances.')
