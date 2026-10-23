"""Replay the complete frozen case set, with no raw acquisition or GT native input."""
from common import *
from build import docker
import numpy as np
PARENT=E.parent/'inten_zero_consensus_control'
DIAG=module('support_parent_metrics',E.parent/'inten_consensus_failure_origin/analyze.py')
CHECK=module('support_validation',E/'validate.py')

def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    for group in ('old','01','02','08'):
        stage=R/('stage_'+group);stage.mkdir()
        previous=PARENT/'groups'/group;meta=read(previous/'input_sources.json')
        # Reuse only the parent-produced complete feature cache with its exact digest.
        cache=meta['files'][0]
        features=Path(cache.get('path',cache.get('feature_cache_private')))
        assert sha(features)==meta['feature_cache_sha256']==cache['sha256']
        inputs=read(previous/'input_sources.json');frames=inputs['frames']
        write(stage/'input_sources.json',dict(parent_group=str(previous.relative_to(W)),parent_archive_sha256=sha(previous/'archive.json'),feature_cache_private=str(features),feature_cache_sha256=sha(features),feature_cache_bytes=features.stat().st_size,frames=frames,no_raw_acquired=True,no_gt_in_native_input=True))
        payload=(previous/'input.txt').read_bytes();(stage/'input.txt').write_bytes(payload)
        binary=read(E/'checks/build/binaries.json')['binaries']['probe'];assert sha(R/'probe')==binary['sha256']
        assert run('native',docker()+[str(R/'probe'),str(features),str(stage/'primitives.jsonl')],stage,payload,jsonl=True)==0
        assert sha(R/'probe')==binary['sha256']
        results=CHECK.evaluate(group,stage/'native.stdout.jsonl',stage/'primitives.jsonl')
        write(stage/'result.json',dict(exit_code=0,source_commit=git('rev-parse','HEAD'),**results))
        module('support_archive',E/'archive.py').archive(stage,E/'groups'/group)
        print(json.dumps(dict(group=group,cases=len(results['cases']),gate=results['gate_passed'],failed=[c for c in results['gate_checks'] if not c['passed']])),flush=True)
if __name__=='__main__':main()
