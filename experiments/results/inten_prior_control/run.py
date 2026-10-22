from common import *
from build import docker
def main():
    assert not git('status','--porcelain')
    p=read(E/'protocol.json')
    for name,digest in p['source_sha256'].items():assert sha(W/name)==digest,name
    for group in ('old','01','02','08'):
        parent=E.parent/'inten_iteration_budget/groups'/group;meta=read(parent/'input_sources.json')
        features=Path(meta['feature_cache_private']);assert sha(features)==meta['feature_cache_sha256']
        assert sha(parent/'archive.json')==p['input_pins'][group]['archive_sha256']
        assert meta['frames']==p['input_pins'][group]['frames']
        assert sha(parent/'input.txt')==p['input_pins'][group]['input_sha256']
        stage=S/('native_'+group);stage.mkdir()
        payload=(parent/'input.txt').read_bytes();(stage/'input.txt').write_bytes(payload)
        write(stage/'input_sources.json',dict(meta,parent_group=str(parent.relative_to(W)),parent_archive_sha256=sha(parent/'archive.json')))
        assert sha(R/'probe')==read(E/'checks/build/binaries.json')['binaries']['probe']['sha256']
        assert run('native',docker()+[str(R/'probe'),str(features)],stage,payload,jsonl=True)==0
        module('reciprocal_group_archive',E/'archive.py').archive(stage,E/'groups'/group)
if __name__=='__main__':main()
