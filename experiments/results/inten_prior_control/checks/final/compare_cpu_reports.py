from pathlib import Path
import sys
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_prior_control');sys.path.insert(0,str(E))
from common import *
v=module('reciprocal_cpu_compare',E/'verify_portable.py')
reference=read(E/'report/summary.json');checks={}
for cpu in ('Haswell','Sandybridge'):
    path=S/'final_checks_stage'/(cpu+'.json')
    v.P.V.V.V.equivalent(read(path),reference)
    checks[cpu]=dict(report_sha256=sha(path),reference_sha256=sha(E/'report/summary.json'),exit_code=0)
write(S/'final_checks_stage/cpu_comparison.json',dict(exit_code=0,relative_tolerance=1e-10,absolute_tolerance=1e-12,checks=checks))
print('Both CPU reconstructions match committed evidence with the original tolerances.')
