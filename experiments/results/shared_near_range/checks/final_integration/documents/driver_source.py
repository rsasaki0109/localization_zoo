from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys
import time

repo = Path('/workspace/localization-env/shared-range-final-audit/worktree')
study = repo / 'experiments/results/shared_near_range'
sys.path.insert(0, str(study))
import finish_docs

started = time.time()
revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=repo, text=True).strip()
original = subprocess.check_output(['git', 'rev-parse', 'ee225d09'], cwd=repo, text=True).strip()
report_path = 'experiments/results/shared_near_range/completion/03/report/summary.json'
raw = subprocess.check_output(['git', 'show', 'HEAD:' + report_path], cwd=repo)
if raw != (repo / report_path).read_bytes():
    raise SystemExit('Full report is not committed unchanged')
report = json.loads(raw)
rows, controls = finish_docs.validate_complete(report)
if not (study / 'diagnostics/10/README.md').is_file():
    raise SystemExit('Saved10 interpretation must exist before final integration')
files = {}
for method, module in finish_docs.MODULES.items():
    path = repo / 'papers' / module / 'README.md'
    own = [r for r in rows if r['method'] == method]
    files[path] = finish_docs.strip_old_blocks(path.read_text()) + finish_docs.method_block(
        method, own, '../../experiments/results/shared_near_range/completion/03/report/summary.json')

path = study / 'README.md'
intro = path.read_text().split('## Scope and status', 1)[0]
intro = intro.replace('[current cleaned checkpoint](checkpoints/09/summary.json)',
                      '[earlier cleaned checkpoint](checkpoints/09/summary.json)')
intro = intro.replace('cleanup; live10 results remain excluded.',
                      'cleanup; later sequences are excluded from that checkpoint.')
intro += ('The [KITTI10 CT/Mesh diagnosis](diagnostics/10/README.md) retains both historical\n'
          'differences, current error-growth blocks and the available Mesh paired path.\n'
          'The [final cleaned checkpoint](checkpoints/10/summary.json) includes every\n'
          'selected sequence and verified cleanup. Missing-path/runtime limits remain explicit.\n\n')
original_readme = subprocess.check_output(['git', 'show', original + ':' +
    path.relative_to(repo).as_posix()], cwd=repo, text=True)
body = original_readme[original_readme.index('## Complete KITTI replay'):]
files[path] = intro + body + (
    '\n## Integration with current main\n\n'
    'The original frozen-source report was committed at `28406fe8`, imported as\n'
    '`b07958a4`; original completion receipts from `ee225d09` are imported at\n'
    '`57e19026`. Their [identity and terminal receipts](checks/final_integration/native_completion/)\n'
    'retain the controller exit, cleanup order and byte-identical completion evidence.\n'
    'The original completion PR body is retained verbatim as execution evidence; its\n'
    'PR137-ready wording is historical and is not current PR metadata.\n\n'
    'This audit measures native source `0b079dbb` with the original per-cell settings.\n'
    'Current-main acceleration, passive tracing and bindings have separate compatibility\n'
    'evidence; their presence in this branch does not turn the frozen replay into a\n'
    'whole-zoo measurement of current-main native code. The later InTEn cause study and\n'
    'fresh NCLT work remain open. No estimator, registration argument, published result\n'
    'or paper number is changed by this completion.\n')

path = repo / 'docs/roadmap.md'
text = path.read_text()
start = text.index('   - Third component implemented:')
stop_match = re.search(r'^7\. ', text[start:], re.M)
if stop_match is None:
    raise SystemExit('Current roadmap next item is missing')
stop = start + stop_match.start()
drift = len(report['historical_metric_drift_cells'])
replacement = (
    '   - Third component implemented in [PR #137](https://github.com/rsasaki0109/localization_zoo/pull/137)\n'
    '     and its frozen KITTI audit is complete: the optional sensor-frame near-range\n'
    '     cut is shared in `common/range_filter.h`, used by ELO/DiLO and intensity-preserving\n'
    '     loaders. Float arithmetic, strict boundary, processing order and zero/off\n'
    '     default remain unchanged; standalone cores do not apply it automatically.\n'
    f"     All {len(rows)}/{len(rows)} selected full-sequence cells across KITTI 00-10 finish\n"
    f"     with native exits 0: {len(rows)-drift} historical matches and {drift} differences.\n"
    f"     All {len(controls)} frozen pre-extraction controls have zero recorded metric\n"
    '     differences and byte-identical saved poses. Historical audits retain exit 1;\n'
    '     their original causes remain unresolved. The controller and full report\n'
    '     finish with exit 0; official input hashes were verified before deleting each\n'
    '     owned sequence and alias. These are historical per-cell settings, not a new\n'
    '     fixed-setting accuracy study. All improving and worsening pairs are retained\n'
    '     in method READMEs; published values and defaults are preserved.\n'
    '     [Full report, every numerical pair, commands and saved-path diagnostics](../experiments/results/shared_near_range/README.md)\n'
    '     retain the complete evidence, including all error-growth blocks and available\n'
    '     historical pairs. Missing exact paths and differing runtimes limit causal\n'
    '     interpretation. The environment interruption, exact binary restoration and\n'
    '     fresh restart are retained in the linked recovery record.\n'
    '     Native source remains `0b079dbb`; this is not a whole-zoo measurement of\n'
    '     newer merged acceleration/tracing. Fresh real-NCLT validation remains open\n'
    '     because the enforced official-host allowlist is unresolved. Its historical\n'
    '     DiLO cut/uncut pair also changes range-image geometry and worsens ATE, so it\n'
    '     cannot isolate the cut. Continuous natural InTEn failure tracing is a separate\n'
    '     next task, not an unfinished part of this KITTI audit.\n')
files[path] = text[:start] + replacement + text[stop:]

changes = {}
for path, text in files.items():
    before = path.read_bytes()
    path.write_text(text)
    changes[path.relative_to(repo).as_posix()] = dict(
        before_sha256=hashlib.sha256(before).hexdigest(),
        after_sha256=hashlib.sha256(path.read_bytes()).hexdigest())
subprocess.run(['git', 'diff', '--check'], cwd=repo, check=True)
out = study / 'checks/final_integration/documents'
out.mkdir(parents=True, exist_ok=False)
(out / 'driver_source.py').write_bytes(Path(__file__).read_bytes())
(out / 'command.json').write_text(json.dumps(dict(
    command=[sys.executable, *sys.argv], cwd=str(repo), source_commit=revision,
    original_completion_commit=original, report_sha256=hashlib.sha256(raw).hexdigest(),
    started_unix=started, finished_unix=time.time(), paths=changes, exit_code=0,
    scope='Reuse the sealed renderer for current method READMEs; retain current study introduction and replace only the completed roadmap item. No estimator or scan read.'), indent=2) + '\n')
print('Final documentation integrated from the committed complete report.')
