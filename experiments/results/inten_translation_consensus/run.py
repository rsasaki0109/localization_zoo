"""Frozen geometry-consensus factorial; no GT or raw-scan acquisition."""
import json
from common import E, W, R, PY, read, write, sha, git, run, restore, module
from consensus import select


def main():
    assert not git('status', '--porcelain')
    protocol = read(E/'protocol.json')
    for name, digest in protocol['source_sha256'].items(): assert sha(W/name) == digest, name
    stage = R/'stage'; stage.mkdir()
    build = module('consensus_build', W/'experiments/results/inten_facade_loss/build.py')
    flags = build.flags(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir')
    flags += ['-I'+str(W/'evaluation/src'), '-I'+str(E)]
    obj, binary = R/'native.o', R/'probe'
    assert run('mechanism_tests', [PY, '-m', 'unittest', 'discover', '-s', str(E), '-p', 'test_consensus.py'], stage) == 0
    assert run('compiler', build.docker()+['c++', '--version'], stage) == 0
    assert run('compile_native', build.docker()+['c++', *flags, '-c', str(E/'diagnostic_native.cpp'), '-o', str(obj)], stage) == 0
    old = read(W/'experiments/results/inten_current_correspondences/baseline/compile_probe_command.json')['command']
    libs = old[old.index('/workspace/localization-env/inten-current-correspondences/native.o')+1:old.index('-o')]
    assert all(not token.endswith('/libinten_loam.a') for token in libs)
    static = {token: sha(token) for token in libs if token.endswith('.a')}
    assert static == read(W/'experiments/results/inten_current_correspondences/baseline/binary.json')['linked_static_libraries']
    assert run('compile_probe', build.docker()+['c++', *flags, str(E/'probe.cpp'), str(obj), *libs, '-o', str(binary)], stage) == 0
    assert run('linked_libraries', build.docker()+['ldd', str(binary)], stage) == 0
    write(stage/'binary.json', dict(path=str(binary), sha256=sha(binary), object_sha256=sha(obj),
          source_commit=git('rev-parse', 'HEAD'), linked_static_libraries=static,
          flags_source=str(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make'),
          flags_source_sha256=sha(build.OLD/'papers/inten_loam/CMakeFiles/test_inten_loam_paper.dir/flags.make')))
    previous = W/'experiments/results/inten_current_correspondences/baseline'
    sources = [restore(previous, name, R/'inputs'/name) for name in ('features.jsonl', 'report.json')]
    sources.append(restore(W/'experiments/results/inten_cluster_motion/controls', 'native.stdout.jsonl', R/'inputs/profiles.jsonl'))
    write(stage/'input_sources.json', dict(files=sources, no_raw_scans_acquired=True, no_gt_in_native_input=True))
    parent = list(map(json.loads, (R/'inputs/profiles.jsonl').read_text().splitlines()))
    terminals = {(v['frame'], v['mode']): v for v in parent if v['type'] == 'terminal'}
    profiles = [v for v in parent if v['type'] == 'profile']
    text = R/'features.txt'
    with text.open('x') as out, (R/'inputs/features.jsonl').open() as source:
        for line in source:
            row = json.loads(line); out.write(str(row['frame'])+'\n')
            for features in row['features']:
                out.write(str(len(features))+'\n')
                for x, y, z, ring, cluster, intensity in features:
                    out.write(f'{x:.17g} {y:.17g} {z:.17g} {ring} {cluster} {intensity:.17g}\n')
    write(stage/'generated_features.json', dict(path=str(text), bytes=text.stat().st_size, sha256=sha(text),
          matches_preceding_serialization=sha(text) == read(W/'experiments/results/inten_cluster_motion/controls/generated_features.json')['sha256']))
    assert read(stage/'generated_features.json')['matches_preceding_serialization']
    lines, selections = [], []
    for i in protocol['frames']:
        records = [v for v in profiles if v['frame'] == i]
        decision = select(records)
        seed = list(terminals[(i, 0)]['solver']['input'])
        direction = records[0]['direction'] if records else [0.]*3
        assert all(r['direction'] == direction for r in records)
        shifted = list(seed)
        for axis in range(3): shifted[4*axis+3] += decision['shift_m']*direction[axis]
        legacy = sorted(v['cluster'] for v in records if v['excluded'])
        masks = [[], [], decision['excluded_clusters'], decision['excluded_clusters'], legacy]
        seeds = [seed, shifted, seed, shifted, seed]
        selections.append(dict(frame=i, decision=decision, direction=direction, original_seed=seed,
                               shifted_seed=shifted, masks=masks))
        for mode in range(5):
            tokens = [str(i), str(mode), str(len(masks[mode])), *map(str, masks[mode]),
                      *(format(v, '.17g') for v in seeds[mode])]
            lines.append(' '.join(tokens))
    write(stage/'consensus.json', dict(frames=selections, rule='protocol.json and consensus.py; no GT'))
    payload = ('\n'.join(lines)+'\n').encode(); (stage/'input.txt').write_bytes(payload)
    assert run('native', build.docker()+[str(binary), str(text), str(stage/'residuals.jsonl')], stage, payload, jsonl=True) == 0
    assert sha(binary) == read(stage/'binary.json')['sha256']
    write(R/'native_complete.json', dict(exit_code=0, source_commit=git('rev-parse', 'HEAD'), stage=str(stage)))


if __name__ == '__main__': main()
