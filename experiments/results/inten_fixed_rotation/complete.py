"""Complete the closed feasibility increment with real exits and preserved publications."""
from pathlib import Path
import hashlib
import json
import os
import re
import subprocess
import time
from common import E, W, R, PY, BASE, read, write, sha, git, module

ENV = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1',
           PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           MPLCONFIGDIR=str(R/'matplotlib'), XDG_CACHE_HOME=str(R/'cache'))


def execute(label, command, folder):
    folder.mkdir(parents=True, exist_ok=True)
    started = time.time()
    with (folder/(label+'.stdout.log')).open('xb') as out, (folder/(label+'.stderr.log')).open('xb') as err:
        result = subprocess.run(command, cwd=W, env=ENV, stdout=out, stderr=err)
    write(folder/(label+'_command.json'), dict(command=command, cwd=str(W), source_commit=git('rev-parse', 'HEAD'),
          started_unix=started, finished_unix=time.time(), exit_code=result.returncode,
          environment={**{k: ENV[k] for k in ('OMP_NUM_THREADS', 'OPENBLAS_NUM_THREADS', 'PYTHONDONTWRITEBYTECODE', 'MPLCONFIGDIR', 'XDG_CACHE_HOME')},
                       'PATH_prefix': str(Path(PY).parent)}))
    print(label, result.returncode, flush=True)
    return result.returncode


def require_command(label, command, folder):
    if execute(label, command, folder):
        raise RuntimeError(label+' failed; preserve this attempt before correcting anything')


def main():
    assert not git('status', '--porcelain')
    post = read(E/'postprocessing_protocol.json')
    for name, digest in post['source_sha256'].items(): assert sha(E/name) == digest
    archive = module('fixed_rotation_completion_archive', E/'archive.py').archive
    build = module('fixed_rotation_completion_build', W/'experiments/results/inten_facade_loss/build.py')
    stage = R/'completion_stage'
    try:
        require_command('runtime', [PY, '-c', 'import json,sys,numpy,matplotlib;assert numpy.__version__=="1.26.4" and matplotlib.__version__=="3.8.4";print(json.dumps(dict(python=sys.version,numpy=numpy.__version__,matplotlib=matplotlib.__version__)))'], stage)
        require_command('configure', build.docker()+['cmake', '-S', str(W), '-B', str(R/'build'),
                        '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_PYTHON_BINDINGS=OFF'], stage)
        require_command('build', build.docker()+['cmake', '--build', str(R/'build'), '--target',
                        'test_inten_loam', 'test_inten_loam_paper', 'test_inten_trace_jsonl', '-j2'], stage)
        require_command('ctest', build.docker()+['ctest', '--test-dir', str(R/'build'), '--output-on-failure',
                        '-R', '^test_inten_(loam|loam_paper|trace_jsonl)$'], stage)
        write(stage/'native_binary_hashes.json', {str(p): sha(p) for p in
              (R/'build/papers/inten_loam/test_inten_loam', R/'build/papers/inten_loam/test_inten_loam_paper',
               R/'build/evaluation/test_inten_trace_jsonl', R/'build/papers/inten_loam/libinten_loam.a')})
        require_command('figures', [PY, str(E/'figures.py')], stage)
        archive(R/'figures', E/'figures')
        require_command('write_docs', [PY, str(E/'write_docs.py')], stage)
        index = W/'experiments/results/index.json'; saved = index.read_bytes()
        for label, script in (('study_docs', 'refresh_study_docs.py'), ('variant_table', 'generate_full_variant_table.py'),
                              ('paper_ratio', 'generate_paper_ratio_table.py')):
            require_command(label, [PY, 'evaluation/scripts/'+script], stage)
        current = index.read_bytes()
        left, right = json.loads(saved), json.loads(current)
        before_clock, after_clock = left.pop('generated_at', None), right.pop('generated_at', None)
        assert left == right, 'Generator changed substantive index data'
        index.write_bytes(saved)
        write(stage/'index_disposition.json', dict(exit_code=0, substantive_fields_equal=True,
              original_clock=before_clock, regenerated_clock=after_clock, original_sha256=hashlib.sha256(saved).hexdigest(),
              regenerated_sha256=hashlib.sha256(current).hexdigest(), retained_committed_index_bytes=True))
        disposition = []
        for name in git('diff', '--name-only', '--', 'docs').splitlines():
            if name == 'docs/roadmap.md': continue
            before = subprocess.check_output(['git', 'show', 'HEAD:'+name], cwd=W)
            after = (W/name).read_bytes()
            normalize = lambda raw: re.sub(rb'(?m)^(_Generated at |>[ ]Generated: )\d{4}-\d{2}-\d{2}T[^\s]+', rb'\1CLOCK', raw)
            assert normalize(before) == normalize(after), 'Substantive generated-doc drift: '+name
            (W/name).write_bytes(before)
            disposition.append(dict(path=name, original_sha256=hashlib.sha256(before).hexdigest(),
                                    regenerated_sha256=hashlib.sha256(after).hexdigest(), only_generation_clock_changed=True))
        write(stage/'generated_docs_disposition.json', dict(exit_code=0, clock_only_restored=disposition))
        require_command('precommit_preservation', [PY, str(E/'verify.py'), '--output', str(stage/'precommit_preservation.json')], stage)
    except BaseException:
        archive(stage, E/'completion_failure')
        raise
    archive(stage, E/'completion/docs')
    subprocess.run(['git', 'add', '-f', str(E), 'papers/inten_loam/README.md', 'docs/roadmap.md',
                    ':(exclude)experiments/results/inten_fixed_rotation/__pycache__/**'], cwd=W, check=True)
    subprocess.run(['git', 'diff', '--cached', '--check'], cwd=W, check=True)
    subprocess.run(['git', 'commit', '-m', 'Report fixed estimated rotation rescue and retained negatives'], cwd=W, check=True)
    checks = R/'final_checks'
    commands = [('python', [PY, '-m', 'unittest', 'discover', '-s', 'tests']),
                ('showcase', [PY, 'evaluation/scripts/validate_showcase.py']),
                ('leaderboard', [PY, 'evaluation/scripts/generate_leaderboard.py', '--check']),
                ('paper_ready', [PY, 'evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
                ('preservation', [PY, str(E/'verify.py'), '--output', str(E/'checks/preservation.json')]),
                ('whitespace', ['git', 'diff', BASE, '--check'])]
    codes = {name: execute(name, command, checks) for name, command in commands}
    write(checks/'result.json', dict(exit_code=int(any(codes.values())), codes=codes))
    archive(checks, E/'completion/final')
    assert not any(codes.values()), 'Checks failed; actual exits and all output retained'
    subprocess.run(['git', 'add', '-f', str(E/'checks'), str(E/'completion')], cwd=W, check=True)
    subprocess.run(['git', 'diff', '--cached', '--check'], cwd=W, check=True)
    subprocess.run(['git', 'commit', '-m', 'Verify fixed estimated rotation and preserve inherited evidence'], cwd=W, check=True)
    print('Feasibility results, docs and actual-exit checks complete; push and exact-head CI remain', flush=True)


if __name__ == '__main__':
    main()
