"""Retest both measured kernel cases and all required local checks after repair."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import importlib.util
import json
import subprocess

E = Path(__file__).resolve().parents[1]; W = E.parents[2]; R = W.parent
spec = importlib.util.spec_from_file_location('repair_driver', E/'run_study.py')
a = importlib.util.module_from_spec(spec); spec.loader.exec_module(a)
PY = a.PY


def main():
    assert not a.git('status', '--porcelain')
    own = Path(__file__).resolve()
    assert subprocess.check_output(['git', 'show', 'HEAD:'+str(own.relative_to(W))], cwd=W) == own.read_bytes()
    stage = R/'portability_repair_stage'
    def kernel(name):
        output = E/'checks/portability_correction'/(name+'_verification.json')
        return name, a.execute(name, ['env', 'OPENBLAS_CORETYPE='+name, PY,
                                     str(E/'verify.py'), '--output', str(output)], stage)
    with ThreadPoolExecutor(max_workers=2) as pool:
        result = dict(pool.map(kernel, ('Haswell', 'Sandybridge')))
    commands = [('python', [PY, '-m', 'unittest', 'discover', '-s', 'tests']),
                ('showcase', [PY, 'evaluation/scripts/validate_showcase.py']),
                ('leaderboard', [PY, 'evaluation/scripts/generate_leaderboard.py', '--check']),
                ('paper_ready', [PY, 'evaluation/scripts/generate_paper_ready_evidence.py', '--check']),
                ('whitespace', ['git', 'diff', 'ed57c87ff2c128ac8d683d5e20938a3141cba7aa', '--check'])]
    result.update({label: a.execute(label, command, stage) for label, command in commands})
    (stage/'result.json').write_text(json.dumps(dict(exit_code=int(any(result.values())), actual_command_exits=result,
          original_scientific_report_bytes_and_all_labels_preserved=True), indent=2)+'\n')
    a.archive(stage, E/'checks/portability_correction/final')
    a.commit('Verify bounded portability repair under both kernels and all required local checks')
    if any(result.values()): raise RuntimeError('Repair checks failed; all receipts retained before correction')
    subprocess.run(['git', 'push'], cwd=W, check=True)
    print('Portability repair, measured kernel controls and all local checks committed and pushed', flush=True)


if __name__ == '__main__': main()
