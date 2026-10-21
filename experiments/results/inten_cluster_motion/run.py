"""Build and run the frozen feasibility trial; there is no raw-data acquisition."""
from pathlib import Path
from common import E, W, R, PY, read, write, sha, git, run, restore, module


def main():
    assert not git('status', '--porcelain'), 'Committed source required'
    protocol = read(E/'protocol.json')
    for name, digest in protocol['source_sha256'].items():
        assert sha(W/name) == digest, name
    stage = R/'stage'
    stage.mkdir()
    build = module('cluster_build', W/'experiments/results/inten_facade_loss/build.py')
    flags = build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags += ['-I'+str(W/'evaluation/src'), '-I'+str(E)]
    obj, binary = R/'native.o', R/'probe'
    assert run('compiler', build.docker()+['c++', '--version'], stage) == 0
    assert run('compile_native', build.docker()+['c++', *flags, '-c', str(E/'diagnostic_native.cpp'), '-o', str(obj)], stage) == 0
    old = read(W/'experiments/results/inten_current_correspondences/baseline/compile_probe_command.json')['command']
    first = old.index('c++')
    libs = old[first+1:]
    libs = libs[libs.index('/workspace/localization-env/inten-current-correspondences/native.o')+1:libs.index('-o')]
    assert all(not token.endswith('/libinten_loam.a') for token in libs)
    static = {token: sha(token) for token in libs if token.endswith('.a')}
    assert static == read(W/'experiments/results/inten_current_correspondences/baseline/binary.json')['linked_static_libraries']
    assert run('compile_probe', build.docker()+['c++', *flags, str(E/'probe.cpp'), str(obj), *libs, '-o', str(binary)], stage) == 0
    assert run('linked_libraries', build.docker()+['ldd', str(binary)], stage) == 0
    write(stage/'binary.json', dict(path=str(binary), sha256=sha(binary), object_sha256=sha(obj),
          source_commit=git('rev-parse', 'HEAD'), linked_static_libraries=static,
          flags_source=str(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make'),
          flags_source_sha256=sha(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make')))
    assert run('mechanism_tests', build.docker()+[str(binary), '--self-test'], stage) == 0
    previous = W/'experiments/results/inten_current_correspondences/baseline'
    sources = [restore(previous, name, R/'inputs'/name) for name in
               ('features.jsonl', 'native.stdout', 'report.json', 'geometry.jsonl')]
    sources.append(restore(W/'experiments/results/inten_facade_loss/controls', 'native.stdout.log', R/'inputs/first_systems.jsonl'))
    write(stage/'input_sources.json', dict(files=sources, no_raw_scans_acquired=True,
          no_gt_in_native_input=True, original_official_scan_hash_manifest=str((previous/'inputs.json').relative_to(W)),
          original_official_scan_hash_manifest_sha256=sha(previous/'inputs.json')))
    traces = {v['frame']: v for v in map(__import__('json').loads, (R/'inputs/native.stdout').read_text().splitlines())}
    text = R/'features.txt'
    with text.open('x') as out, (R/'inputs/features.jsonl').open() as source:
        for line in source:
            row = __import__('json').loads(line)
            out.write(str(row['frame'])+'\n')
            for features in row['features']:
                out.write(str(len(features))+'\n')
                for x, y, z, ring, cluster, intensity in features:
                    out.write(f'{x:.17g} {y:.17g} {z:.17g} {ring} {cluster} {intensity:.17g}\n')
    payload = ''.join(str(i)+' '+' '.join(format(v, '.17g') for v in traces[i]['s2s']['input'])+'\n'
                      for i in protocol['frames']).encode()
    (stage/'input.txt').write_bytes(payload)
    write(stage/'generated_features.json', dict(path=str(text), bytes=text.stat().st_size, sha256=sha(text),
          source_sha256=sources[0]['sha256'], reconstruction='run.py: features JSONL -> 17 significant digits, unchanged feature/class order'))
    assert run('native', build.docker()+[str(binary), str(text)], stage, payload, jsonl=True) == 0
    assert sha(binary) == read(stage/'binary.json')['sha256']
    write(R/'native_complete.json', dict(exit_code=0, source_commit=git('rev-parse', 'HEAD'), stage=str(stage)))


if __name__ == '__main__':
    main()
