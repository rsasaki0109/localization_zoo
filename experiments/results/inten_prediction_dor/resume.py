"""Explicit continuation after parser failure; never rerun compiled/acquired work."""
from run import E, W, R, PY, build, read, write, sha, run, module


def main():
    stage = R/'staging'
    assert read(E/'initial_failure/driver_command.json')['exit_code'] == 1
    assert not (stage/'native_command.json').exists()
    assert sha(R/'probe') == read(stage/'binary.json')['sha256']
    assert sha(R/'native.o') == read(stage/'binary.json')['object_sha256']
    instrument = module('native_transform', E/'instrument.py')
    source = (W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text()
    assert instrument.transformed(source) == (E/'diagnostic_native.cpp').read_text()
    raw = R/'kitti/dataset/sequences/01/velodyne'
    inputs = {f.name: dict(sha256=sha(f), bytes=f.stat().st_size) for f in sorted(raw.iterdir()) if f.is_file()}
    assert inputs == read(stage/'inputs.json')['files']
    assert inputs == read(W/'experiments/results/shared_near_range/sequences/01/inputs.json')['files']
    last = max(int(k[:-4]) for k in inputs if k.endswith('.bin'))
    p = read(E/'protocol.json')
    payload = ('\n'.join(map(str, [last, *p['detailed_frames']]))+'\n').encode()
    (stage/'input.txt').write_bytes(payload)
    assert run('native', build.docker()+[str(R/'probe'), str(raw), str(stage/'correspondences.jsonl'),
               str(stage/'masks.jsonl'), str(stage/'ranges.jsonl')], stage, payload) == 0
    assert sha(R/'probe') == read(stage/'binary.json')['sha256']
    write(R/'collection_complete.json', dict(exit_code=0, stage=str(stage), raw_retained=True,
          resumed_only_after_initial_parser_failure=True, no_compile_or_fetch_rerun=True))


if __name__ == '__main__':
    main()
