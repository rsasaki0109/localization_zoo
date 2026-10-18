"""Check input-to-estimator indexing when the existing loader skips an empty scan."""
from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,time
E=Path(__file__).resolve().parent;REPO=E.parents[2]
P=json.loads((E/'protocol.json').read_text())


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate-binary',required=True,type=Path)
    parser.add_argument('--output-dir',required=True,type=Path)
    a=parser.parse_args();out=a.output_dir.resolve();assert not out.exists();out.mkdir()
    fixture=out/'input';fixture.mkdir()
    shutil.copyfile(REPO/P['validation']['fixture']/'frame_timestamps.csv',fixture/'frame_timestamps.csv')
    clouds=sorted((REPO/P['validation']['fixture']).glob('*/cloud.pcd'));assert len(clouds)==3
    for i,p in enumerate(clouds):
        dest=fixture/f'{i:08d}'/'cloud.pcd';dest.parent.mkdir()
        if i==1:
            dest.write_text('# .PCD v0.7\nVERSION 0.7\nFIELDS x y z intensity\nSIZE 4 4 4 4\nTYPE F F F F\nCOUNT 1 1 1 1\nWIDTH 0\nHEIGHT 1\nVIEWPOINT 0 0 0 1 0 0 0\nPOINTS 0\nDATA ascii\n')
        else:shutil.copyfile(p,dest)
    gt=REPO/P['validation']['gt']
    flags=next(v['args'] for v in json.loads((REPO/'experiments/inten_loam_kitti_seq_07_full_paper_comp_matrix.json').read_text())['variants'] if v['id']=='paper_odometry_plane_check')
    reference=Path(P['validation']['reference_binary']);candidate=a.candidate_binary.resolve()
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
    assert sha(reference)==P['validation']['reference_binary_sha256']
    binaries={k:dict(path=str(p),sha256=sha(p)) for k,p in [('reference',reference),('candidate',candidate)]}
    prefix=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock','exec','--user','1000:1000','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1']
    exits=[]
    for name,binary in [('reference',reference),('candidate_off',candidate),('candidate_on',candidate)]:
        folder=out/name;folder.mkdir();summary=folder/'summary.json'
        argv=[str(binary),str(fixture),str(gt),'3','--methods','inten_loam','--summary-json',str(summary),*flags]
        if name=='candidate_on':argv+=['--inten-loam-paper-trace-jsonl',str(folder/'trace.jsonl')]
        command=prefix+['-w',str(folder),'localization-zoo-dev',*argv];start=time.time()
        with (folder/'native.log').open('w') as f:r=subprocess.run(command,cwd=REPO,stdout=f,stderr=subprocess.STDOUT)
        (folder/'command.json').write_text(json.dumps(dict(command=command,cwd=str(REPO),native_cwd=str(folder),exit_code=r.returncode,started_unix=start,finished_unix=time.time()),indent=2)+'\n')
        exits.append(r.returncode);assert r.returncode==0
    pose_files={name:{p.name:p for p in (out/name/'dogfooding_results').glob('*.txt')} for name in ['reference','candidate_off','candidate_on']}
    assert {'InTEn_LOAM.txt','gt.txt'}<=pose_files['reference'].keys()
    assert len(pose_files['reference']['InTEn_LOAM.txt'].read_text().splitlines())==2
    for name in ['candidate_off','candidate_on']:
        assert pose_files[name].keys()==pose_files['reference'].keys()
        for key,p in pose_files['reference'].items():assert p.read_bytes()==pose_files[name][key].read_bytes(),(name,key)
    def reject(value):raise ValueError(value)
    records=[json.loads(line,parse_constant=reject) for line in (out/'candidate_on/trace.jsonl').read_text().splitlines()]
    assert [x['type'] for x in records]==['header','frame','skipped_frame','frame','end']
    assert records[0]['input_frames']==3
    assert [x['input_index'] for x in records[1:-1]]==[0,1,2]
    assert [x['frame_index'] for x in records if x['type']=='frame']==[0,1]
    assert records[2]['reason']=='empty_cloud' and 's2s' not in records[2] and 's2m' not in records[2]
    assert records[-1]==dict(type='end',processed_frames=2,skipped_frames=1)
    for name,p in [('reference',reference),('candidate',candidate)]:assert sha(p)==binaries[name]['sha256']
    inputs={str(p.relative_to(out)):sha(p) for p in fixture.rglob('*') if p.is_file()}
    summary=dict(exit_code=0,native_source_commit=P['native_source_commit'],binaries=binaries,native_exits=exits,
                 input_sha256=inputs,record_types=[x['type'] for x in records],input_indices=[0,1,2],
                 estimator_frame_indices=[0,1],processed_frames=2,skipped_frames=1,
                 saved_pose_bytes_identical=True,gt_sha256=sha(gt),
                 scope='Synthetic missing-input lifecycle check only. First/third committed MCD scans, middle replaced by empty PCD. Accuracy fields are not interpreted because the inherited runner removes the empty scan without retiming GT.')
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('Skipped-input indexing and pose parity verified; not an accuracy result.')


if __name__=='__main__':main()
