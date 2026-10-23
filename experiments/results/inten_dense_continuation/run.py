from common import *
from build import docker
V=module('dense_old_motion',E.parent/'inten_iteration_budget/verify.py')
def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    for group in ('old','01','02','08'):
        parent=E.parent/'inten_iteration_budget/groups'/group;meta=read(parent/'input_sources.json')
        features=Path(meta['feature_cache_private']);assert sha(features)==meta['feature_cache_sha256']
        rows=[json.loads(r) for r in V.decode(parent,'native.stdout.jsonl').splitlines()]
        poses={(r['frame'],r['mode']):r['trace']['accepted_relative'] for r in rows if r['type']=='terminal'}
        text=[]
        for f in meta['frames']:
            text.append(str(f))
            for mode in range(12):text.append(' '.join(map(str,[mode,*poses[f,mode]])))
        payload=('\n'.join(text)+'\n').encode();stage=R/('native_'+group);stage.mkdir()
        (stage/'input.txt').write_bytes(payload)
        write(stage/'input_sources.json',dict(parent_group=str(parent.relative_to(W)),parent_archive_sha256=sha(parent/'archive.json'),
            feature_cache_private=str(features),feature_cache_sha256=sha(features),frames=meta['frames'],no_gt_in_native_input=True,no_raw_acquired=True))
        assert sha(R/'probe')==read(E/'checks/build/binaries.json')['binaries']['probe']['sha256']
        assert run('native',docker()+[str(R/'probe'),str(features)],stage,payload,jsonl=True)==0
        module('dense_result_archive',E/'archive.py').archive(stage,E/'groups'/group)
if __name__=='__main__':main()
