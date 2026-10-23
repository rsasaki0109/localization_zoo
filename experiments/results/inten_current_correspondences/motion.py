"""Execute and summarize the frozen full-point geometric profiles."""
from run import E, W, R, build, read, write, run, sha, module


def main():
    stage = R/'motion_v2'; stage.mkdir()
    library = '/workspace/localization-env/inten-facade-pca/build/papers/inten_loam/libinten_loam.a'
    command = read(R/'staging/compile_probe_command.json')['command']
    command = [str(E/'motion_probe.cpp') if a == str(E/'probe.cpp') else
               library if a == str(R/'native.o') else
               str(R/'motion_v2_probe') if a == str(R/'probe') else a for a in command]
    assert run('compile', command, stage) == 0
    write(stage/'binary.json', dict(path=str(R/'motion_v2_probe'), sha256=sha(R/'motion_v2_probe'),
          native_library_path=library, native_library_sha256=sha(library), native_commit='9a01528b05128794df962b52ace59e727e834417',
          other_libraries=read(R/'staging/binary.json')['linked_static_libraries']))
    lines = []
    raw = R/'kitti/dataset/sequences/01/velodyne'
    for c in read(R/'staging/report.json')['cases']:
        i = c['frame']; values = ' '.join(format(v, '.17g') for v in c['gt_relative'])
        lines.append(f'{i} {raw/(str(i-1).zfill(6)+".bin")} {raw/(str(i).zfill(6)+".bin")} {values}')
    payload = ('\n'.join(lines)+'\n').encode(); (stage/'input.txt').write_bytes(payload)
    assert run('native', build.docker()+[str(R/'motion_v2_probe')], stage, payload) == 0
    rows = list(map(__import__('json').loads, (stage/'native.stdout').read_text().splitlines()))
    assert [(r['frame'], r['region']) for r in rows] == [(i, j) for i in read(E/'motion_protocol.json')['frames'] for j in range(4)]
    report = []
    for r in rows:
        assert len(r['profile']) in (0, 121)
        if not r['profile']:
            assert r.get('unavailable') == 'empty fixed region in at least one frame'
            report.append(r)
            continue
        zero = next(v for v in r['profile'] if v[0] == 0.)
        record = dict(frame=r['frame'], region=r['region'], previous_points=r['previous_points'], current_points=r['current_points'])
        for name, index in [('symmetric_mse', 1), ('trimmed80_mse', 2)]:
            best = min(r['profile'], key=lambda v: v[index])
            record[name] = dict(best_shift_m=best[0], best_mse_m2=best[index], zero_mse_m2=zero[index],
                                optimum_on_boundary=abs(best[0]) == 3.)
        report.append(record)
    write(stage/'report.json', dict(exit_code=0, cases=report, limits=read(E/'motion_protocol.json')['limits']))
    archive = module('motion_archive', W/'experiments/results/inten_natural_traces/run_sequences.py')
    archive.archive(stage, E/'motion')


if __name__ == '__main__':
    main()
