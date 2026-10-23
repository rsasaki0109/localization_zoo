#!/usr/bin/env python3
"""Fail closed on missing frames, broken provenance or replay controls."""
import argparse
import gzip
import json
from pathlib import Path
from receipt import save, sha


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--study',type=Path,default=Path(__file__).resolve().parent)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args(); root=a.study
    spec=json.loads((root/'protocol.json').read_text())
    rows=[json.loads(line) for line in (root/'kiss/frames.jsonl').read_text().splitlines()]
    pins=json.loads((a.source_root/'experiments/results/inten_facade_pca/sequences/01/inputs.json').read_text())['files']
    assert [r['frame'] for r in rows]==list(range(spec['independent']['frames']))
    assert all(r['raw_sha256']==pins[f"{r['frame']:06d}.bin"]['sha256'] for r in rows)
    completion=json.loads((root/'kiss/completion.json').read_text())
    assert completion['poses_sha256']==sha(root/'kiss/poses.txt')
    assert completion['frames_sha256']==sha(root/'kiss/frames.jsonl')
    assert completion['exit_code']==0 and completion['frames']==len(rows)
    pair_completion=json.loads((root/'probe/completion.json').read_text())
    assert pair_completion['raw_sha256']==sha(root/'probe/raw.jsonl.gz')
    assert pair_completion['kiss_poses_sha256']==completion['poses_sha256']
    first,last=spec['diagnostic']['frame_range_inclusive']
    frames=list(range(first,last+1))+spec['diagnostic']['additional_frames']
    assert pair_completion['selected_frames']==frames
    with gzip.open(root/'probe/raw.jsonl.gz','rt') as f:
        pairs=[json.loads(line) for line in f if line.startswith('{')]
    expected=[f'{i}_{mode}' for i in frames for mode in spec['diagnostic']['seeds']]
    assert [r['id'] for r in pairs]==expected
    replay=json.loads((root/'probe/reference_replay.json').read_text())
    assert [r['frame'] for r in replay]==frames
    assert all(r['native_replay_max_abs_error']<=1e-8 and r['loader_point_counts_equal'] for r in replay)
    assert pair_completion['native_replay_all_equal'] and pair_completion['loader_point_counts_all_equal']
    report=json.loads((root/'report/summary.json').read_text())
    assert report['controls_valid']
    ref=json.loads((a.source_root/'experiments/results/inten_facade_pca/sequences/01/candidate/summary.json').read_text())
    method=next(m for m in ref['methods'] if m['name']=='InTEn_LOAM')
    assert abs(report['trajectories']['inten_fixed']['official_metrics_from_saved_poses']['kitti_rte_trans_pct']-method['kitti_rte_trans_pct'])<.001
    receipts={}
    for name in ['fetch_missing','preprocess_build','input_tests','kiss','probe_retry','analysis']:
        receipt=json.loads((root/'commands'/f'{name}.json').read_text())
        assert receipt['exit_code']==0 and receipt['tracked_dirty']==[],name
        for suffix,digest in receipt['logs'].items():
            assert sha(root/'commands'/f'{name}{suffix}')==digest
        receipts[name]=receipt['source_commit']
    save(a.output,{'exit_code':0,'frames_verified':len(rows),'fixed_pairs_verified':len(pairs),
        'max_native_replay_error':max(r['native_replay_max_abs_error'] for r in replay),
        'successful_receipt_sources':receipts,
        'known_acquisition_failures_retained':['fetch.json','fetch_resume.json','probe.json']})


if __name__=='__main__':
    main()
