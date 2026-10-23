"""Remove only this study's owned raw sequence, after archived numerical push."""
from common import *
import argparse,shutil

def main():
    p=argparse.ArgumentParser();p.add_argument('--sequence',choices=['01','02','08'],required=True);args=p.parse_args();seq=args.sequence
    assert not git('status','--porcelain')
    head=git('rev-parse','HEAD');branch=git('branch','--show-current')
    remote=git('ls-remote','--heads','origin',branch).split();assert remote[0]==head
    folder=E/'groups'/seq;manifest=read(folder/'archive.json');assert manifest['byte_roundtrip_verified']
    # Recheck EVERY archived file, not just the conclusion.
    for name,entry in manifest['files'].items():
        target=R/'cleanup_verification'/seq/name
        restore(folder,name,target)
    assert read(folder/'result.json')['exit_code']==0
    root=R/('raw_'+seq);scan_dir=root/'dataset/sequences'/seq/'velodyne'
    expected=read(folder/'raw_inputs.json')['files'];actual={scan.name:dict(bytes=scan.stat().st_size,sha256=sha(scan)) for scan in scan_dir.glob('*.bin')}
    assert actual==expected
    assert {p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file()}=={f'dataset/sequences/{seq}/velodyne/{n}' for n in expected}|{f'dataset/sequences/{seq}/velodyne/frame_timestamps.csv'}
    shutil.rmtree(root);assert not root.exists()
    write(E/'cleanup'/(seq+'.json'),dict(sequence=seq,command=[PY,str(E/'cleanup.py'),'--sequence',seq],cwd=str(W),source_commit=head,numerical_push_verified_head=head,remote_ref=branch,archive_manifest_sha256=sha(folder/'archive.json'),all_archive_bytes_roundtrip_rechecked=True,only_owned_raw_removed=str(root),raw_files=expected,exit_code=0))
    print('Owned raw removed after complete archive verification and numerical push:',seq)
if __name__=='__main__':main()
