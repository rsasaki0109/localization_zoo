"""Exact complete01 and interrupted07-prefix controls against the original producer."""
from pathlib import Path
import gzip,hashlib,json
E=Path(__file__).resolve().parent;W=E.parents[2]
def read(p):return json.loads(Path(p).read_text())
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def require(ok,reason):
    if not ok:raise ValueError(reason)

def lines(folder,name,partial=False):
    entry=read(folder/'archive.json')['files'][name]
    require(entry['storage']=='ordered_gzip_shards','Unexpected trace storage')
    remainder=b'';digest=hashlib.sha256();length=0
    for part in entry['parts']:
        path=folder/part['path'];require(sha(path)==part['gzip_sha256'],'Original shard changed')
        raw=gzip.decompress(path.read_bytes());require(hashlib.sha256(raw).hexdigest()==part['raw_sha256'] and len(raw)==part['raw_bytes'],'Original decoded shard changed')
        digest.update(raw);length+=len(raw);chunks=(remainder+raw).split(b'\n');remainder=chunks.pop()
        for row in chunks:yield row+b'\n',False
    require(digest.hexdigest()==entry['sha256'] and length==entry['bytes'],'Original trace stream changed')
    if remainder:
        require(partial,'Full original trace lacks terminal newline');yield remainder,True

def compare_trace(old,logical,fresh,old_root,new_root,partial=False):
    frames=records=truncated=0
    with fresh.open('rb') as stream:
        for original,fragment in lines(old,logical,partial):
            try:record=json.loads(original)
            except ValueError:
                require(partial and fragment,'Corrupt complete original record');truncated=len(original);continue
            actual=stream.readline();require(bool(actual),'Fresh trace truncated')
            if record['type']=='header':
                other=json.loads(actual);record.pop('argv');other.pop('argv');require(record==other,'Effective original header changed')
            else:
                normalized=original
                if record['type']=='frame':
                    require(record['scan_path']==old_root+f"{record['input_index']:06d}.bin/cloud.pcd",'Unexpected original scan identity')
                    normalized=original.replace(('\"scan_path\":\"'+old_root).encode(),('\"scan_path\":\"'+new_root).encode(),1)
                    frames+=1
                if fragment:normalized+=b'\n'
                require(actual==normalized,'Original/new numerical trace mismatch')
            records+=1
        if not partial:require(not stream.read(1),'Fresh trace has extra records')
    require(frames>0,'No original frames compared')
    return dict(complete_original_frames_compared=frames,records=records,partial_prefix_only=partial,
                ignored_truncated_terminal_bytes=truncated,numerical_trace_body_exact=True,header_exact_except_argv=True,
                scan_root_substitution=dict(original=old_root,current=new_root))

def validate(seq,stage):
    original=E.parent/'inten_recursive_consensus';checks={}
    if seq=='01':
        folder=original/'sequences/01'
        for mode in ('baseline','candidate'):
            current=stage/mode;old=folder/mode;a,b=read(old/'summary.json'),read(current/'summary.json')
            keys=('status','frames','ate_m','rpe_trans_pct','kitti_rte_trans_pct','kitti_rte_rot_deg_per_100m')
            require(all(a['methods'][0][k]==b['methods'][0][k] for k in keys),'Original full01 metric changed')
            names=sorted(p.name for p in (old/'dogfooding_results').glob('*.txt'))
            require(all((old/'dogfooding_results'/n).read_bytes()==(current/'dogfooding_results'/n).read_bytes() for n in names),'Original full01 pose/GT changed')
            checks[mode]=compare_trace(folder,mode+'/trace.jsonl',current/'trace.jsonl',a['pcd_dir'].rstrip('/')+'/',b['pcd_dir'].rstrip('/')+'/')
            checks[mode].update(metrics_exact=list(keys),saved_pose_gt_bytes_equal=names)
    elif seq=='07':
        folder=original/'collection_interruption/partial_seq_07';inputs=read(folder/'inputs.json');b=read(stage/'candidate/summary.json')
        require(read(folder/'candidate/native_command.json')['exit_code'] in (130,-2),'Intentional original exit lost')
        checks['candidate_prefix']=compare_trace(folder,'candidate/trace.jsonl',stage/'candidate/trace.jsonl',inputs['owned_input_dir'].rstrip('/')+'/',b['pcd_dir'].rstrip('/')+'/',True)
    return dict(exit_code=0,sequence=seq,checks=checks,scope='Exact implementation-equivalence control; original full01 negative retained, interrupted07 prefix is not a complete accuracy result. No tuning, GT decision or native resume.')

if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--sequence',required=True);parser.add_argument('--stage',type=Path,required=True);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();result=validate(args.sequence,args.stage)
    with args.output.open('x') as out:out.write(json.dumps(result,indent=2)+'\n')
    print('Original full01 or interrupted07 prefix equivalence checked')
