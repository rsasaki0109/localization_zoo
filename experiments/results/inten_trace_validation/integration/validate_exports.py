"""Validate committed integrated exports without invoking native code or scans."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    a = parser.parse_args()
    w = a.repo.resolve()
    out = a.output_dir.resolve()
    assert not out.exists()
    source = '8752c852d750277a2bbeda49ec7c36f154558529'
    guard = ['git', 'diff', '--exit-code', source, '--',
             'evaluation/scripts/check_inten_trace.py', 'tests/test_inten_trace_validation.py']
    subprocess.run(guard, cwd=w, check=True)
    head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=w, text=True).strip()
    root = w / 'experiments/results/inten_trace_export/integration/evidence/fixture'
    rows = []
    out.mkdir()
    for profile in ['paper_default', 'paper_odometry_plane_check']:
        for mode in ['default', 'accelerated']:
            variant = profile + '__' + mode
            folder = root / variant / 'candidate_on'
            paths = [folder / name for name in ['trace.jsonl', 'summary.json', 'command.json']]
            before = {}
            for p in paths:
                data = p.read_bytes()
                assert data == subprocess.check_output(['git', 'show', head + ':' + p.relative_to(w).as_posix()], cwd=w)
                before[p.name] = hashlib.sha256(data).hexdigest()
            with paths[0].open() as f:
                header = json.loads(next(f))
            assert header['params']['accelerated'] is (mode == 'accelerated')
            report = out / (variant + '.json')
            command = [sys.executable, str(w / 'evaluation/scripts/check_inten_trace.py'),
                '--trace', str(paths[0]), '--summary', str(paths[1]),
                '--receipt', str(paths[2]), '--output', str(report)]
            start = time.time()
            with (out / (variant + '.log')).open('w') as log:
                p = subprocess.run(command, cwd=w, stdout=log, stderr=subprocess.STDOUT)
            receipt = dict(command=command, cwd=str(w), exit_code=p.returncode,
                tool_source_commit=source, input_commit=head, input_sha256=before,
                started_unix=start, finished_unix=time.time())
            (out / (variant + '_command.json')).write_text(json.dumps(receipt, indent=2) + '\n')
            assert p.returncode == 0 and report.is_file(), variant
            assert {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in paths} == before
            rows.append(dict(profile=profile, execution_mode=mode, exit_code=0,
                effective_accelerated=header['params']['accelerated'], report=report.name,
                input_sha256=before, report_sha256=hashlib.sha256(report.read_bytes()).hexdigest()))
    subprocess.run(guard, cwd=w, check=True)
    (out / 'summary.json').write_text(json.dumps(dict(exit_code=0, tool_source_commit=source,
        input_commit=head, source_guard_command=guard, checks=rows,
        scope='Record/run consistency only. No native invocation, raw scan read/fetch, setting selection, accuracy or solver-cause claim.'), indent=2) + '\n')
    print('Unchanged reader validates default/accelerated committed exports.')


if __name__ == '__main__':
    main()
