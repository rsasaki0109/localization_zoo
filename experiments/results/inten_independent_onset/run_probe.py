#!/usr/bin/env python3
"""Fixed-pair controls; existing frozen probe, no estimator rebuild."""
import argparse
import gzip
import json
from pathlib import Path
import subprocess
import sys
from receipt import save, sha


def main():
    p = argparse.ArgumentParser(__doc__)
    for name in ['source-root','scans','kiss','output']:
        p.add_argument('--'+name, type=Path, required=True)
    p.add_argument('--container', default='localization-zoo-dev')
    p.add_argument('--probe', type=Path, required=True)
    p.add_argument('--container-scratch', required=True)
    a = p.parse_args()
    sys.path.insert(0, str(a.source_root/'evaluation/scripts'))
    import numpy as np
    from evaluate_external_kitti_odometry import load_kitti_poses
    protocol = json.loads(Path(__file__).with_name('protocol.json').read_text())
    spec = protocol['diagnostic']
    assert sha(a.probe) == spec['probe_sha256']
    first, last = spec['frame_range_inclusive']
    selected = list(range(first,last+1))+spec['additional_frames']
    archive = a.source_root/'experiments/results/inten_facade_pca'
    trace_path = archive/'sequences/01/candidate/trace.jsonl.parts/part-00000.gz'
    with gzip.open(trace_path,'rt') as f:
        traces = {r['input_index']:r for line in f if (r:=json.loads(line))['type']=='frame' and r['input_index'] in selected}
    assert sorted(traces) == selected
    kiss = load_kitti_poses(a.kiss/'poses.txt')
    frames = {r['frame']:r for line in (a.kiss/'frames.jsonl').read_text().splitlines() if (r:=json.loads(line))}
    pins = json.loads((archive/'sequences/01/inputs.json').read_text())['files']
    a.output.mkdir(parents=True, exist_ok=False)
    commands=[]
    docker=['docker']
    def run(argv, stdin=None):
        result=subprocess.run(argv,input=stdin,text=True,capture_output=True)
        commands.append({'argv':argv,'stdin':stdin,'exit_code':result.returncode,
                         'stdout':result.stdout,'stderr':result.stderr})
        save(a.output/'commands.json',commands)
        if result.returncode:
            raise RuntimeError(f'command failed: {argv}')
        return result.stdout
    run(docker+['inspect','--format','{{.Image}}',a.container])
    run(docker+['exec',a.container,'ldd',str(a.probe)])
    run(docker+['exec',a.container,'mkdir',a.container_scratch])
    raw=[]; results=[]; comparisons=[]
    for i in selected:
        targets=[]
        for index,name in [(i-1,'previous.bin'),(i,'current.bin')]:
            source=a.scans/f'{index:06d}.bin'
            assert sha(source)==pins[source.name]['sha256']
            target=a.container_scratch+'/'+name
            run(docker+['cp',str(source),a.container+':'+target])
            actual=run(docker+['exec',a.container,'sha256sum',target]).split()[0]
            assert actual==pins[source.name]['sha256']
            targets.append(target)
        seeds={'native':np.array(traces[i]['s2s']['input']).reshape(4,4),
               'kiss_current':np.linalg.inv(kiss[i-1])@kiss[i],
               'kiss_previous':np.linalg.inv(kiss[i-2])@kiss[i-1]}
        stdin=''.join(f'{i}_{mode} candidate '+ ' '.join(targets)+' '+
                      ' '.join(format(x,'.17g') for x in seed.flat)+'\n' for mode,seed in seeds.items())
        stdout=run(docker+['exec','-i','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1',
                           a.container,str(a.probe)],stdin)
        raw.append(stdout)
        rows=[json.loads(line) for line in stdout.splitlines() if line.startswith('{')]
        assert [r['id'] for r in rows] == [f'{i}_{mode}' for mode in seeds]
        for row in rows:
            assert row['previous_points']==frames[i-1]['input_points']
            assert row['current_points']==frames[i]['input_points']
        control=np.array(rows[0]['output']).reshape(4,4)
        error=float(np.max(np.abs(control-np.array(traces[i]['s2s']['output']).reshape(4,4))))
        comparisons.append({'frame':i,'native_replay_max_abs_error':error,
                            'within_tolerance':error<=1e-8,'loader_point_counts_equal':True})
        results.extend(rows)
        print(f'frame {i} replay {error:.3g}',flush=True)
    (a.output/'raw.jsonl.gz').write_bytes(gzip.compress(''.join(raw).encode(),mtime=0))
    save(a.output/'reference_replay.json',comparisons)
    save(a.output/'completion.json',{'selected_frames':selected,'rows':len(results),
        'source_trace_sha256':sha(trace_path),'kiss_poses_sha256':sha(a.kiss/'poses.txt'),
        'raw_sha256':sha(a.output/'raw.jsonl.gz'),
        'native_replay_all_equal':all(r['within_tolerance'] for r in comparisons),
        'loader_point_counts_all_equal':True})
    # Remove only the two files and empty directory created by this invocation.
    run(docker+['exec',a.container,'rm',a.container_scratch+'/previous.bin',a.container_scratch+'/current.bin'])
    run(docker+['exec',a.container,'rmdir',a.container_scratch])
    assert all(r['within_tolerance'] for r in comparisons), 'control mismatch: intervention interpretation invalid'


if __name__=='__main__':
    main()
