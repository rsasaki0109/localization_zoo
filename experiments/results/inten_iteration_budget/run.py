"""Every inherited same-history endpoint, every field/control, both fixed budgets."""
from common import *
from build import docker
PARENT=E.parent/'inten_nonfacade_consensus_support'
DECODE=module('normalized_decode',PARENT/'validate.py').decode

def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    binaries=read(E/'checks/build/binaries.json')['binaries']
    for group in ('old','01','02','08'):
        stage=R/('replay_'+group);stage.mkdir()
        parent=PARENT/'groups'/group;meta=read(parent/'input_sources.json')
        features=Path(meta['feature_cache_private']);assert sha(features)==meta['feature_cache_sha256']
        payload=(parent/'input.txt').read_bytes();(stage/'input.txt').write_bytes(payload)
        write(stage/'input_sources.json',dict(parent_group=str(parent.relative_to(W)),
            parent_archive_sha256=sha(parent/'archive.json'),feature_cache_private=str(features),
            feature_cache_sha256=sha(features),frames=meta['frames'],
            no_raw_acquired=True,no_gt_in_native_input=True))
        assert sha(R/'probe')==binaries['probe']['sha256']
        assert run('native',docker()+[str(R/'probe'),str(features)],stage,payload,jsonl=True)==0
        module('normalized_archive',E/'archive.py').archive(stage,E/'groups'/group)
if __name__=='__main__':main()
