from pathlib import Path
import hashlib,importlib.util,inspect,json
import numpy as np
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information')
S=Path('/tmp/localization-zoo-inten-support-information-20261023')
s=importlib.util.spec_from_file_location('support_frozen_diagnosis',E/'verify.py');v=importlib.util.module_from_spec(s);s.loader.exec_module(v)
original=v.require
def diagnose(ok,why):
    if not ok and why=='Native residual validity differs':
        local=inspect.currentframe().f_back.f_locals
        record=local['record'];source=local['source'];valid=local['valid'];native=local['native_valid']
        indices=np.flatnonzero(valid!=native).tolist()
        result=dict(scope='Diagnostic of the preserved verifier exit1, not successful numerical validation',
            frame=record['frame'],mode=record['mode'],translation_only=record['translation_only'],iteration=record['iteration'],
            feature_class=record['class'],mismatch_sources=indices,
            examples=[dict(source_index=i,feature=source[i].tolist(),native_row=record['rows'][i],python_valid=bool(valid[i]),query=local['x'][i].tolist()) for i in indices[:8]])
        (S/'validity_failure_details.json').write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps(result))
    original(ok,why)
v.require=diagnose
try:v.group_result('old')
except ValueError as error:
    assert str(error)=='Native residual validity differs'
    assert (S/'validity_failure_details.json').exists()
else:raise AssertionError('Expected frozen failure was not reproduced')
