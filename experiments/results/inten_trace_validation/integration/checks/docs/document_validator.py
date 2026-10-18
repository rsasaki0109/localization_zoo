from pathlib import Path
import json
import subprocess

W = Path('/workspace/localization-env/inten-trace-validator/worktree')
E = W / 'experiments/results/inten_trace_validation/integration'
p = E / 'evidence/summary.json'
assert p.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + p.relative_to(W).as_posix()], cwd=W)
s = json.loads(p.read_text())
assert s['exit_code'] == 0 and len(s['checks']) == 4
assert all(row['exit_code'] == 0 for row in s['checks'])
(E / 'README.md').write_text('''# Reader compatibility with integrated accelerated exports

The unchanged checker source `8752c852` accepts the committed exports from the
combined speed/API/CLI source. The new native header includes the effective
`params.accelerated` boolean. The original archived corpus and synthetic negative
cases remain unchanged.

Evidence was committed at `b1ef79df` before this prose. All 4 explicit reader
commands exit 0 and produce reports: default and unchanged KITTI07-selected
plane-check registration, each with acceleration disabled and enabled. Their
trace/summary/receipt files are committed native evidence from `2e6d7689`, reached
through the integrated CLI branch `beb2f828`. Every input file is byte-identical
before and after checking. The reader and its unit-test source remain unchanged.

[Commands, input/report hashes and reports](evidence/summary.json) and the
[batch recipe](validate_exports.py) retain the exact executions. Repeat with:

```bash
python3 experiments/results/inten_trace_validation/integration/validate_exports.py --repo "$PWD" --output-dir /tmp/inten-integrated-reader-new
python3 experiments/results/inten_trace_validation/verify_preservation.py --base-commit beb2f828b4f5a731f42e10fd16c695981d2c3fb3 --output /tmp/inten-reader-preservation-new.json
```

This checks record/run consistency, without invoking an estimator, opening raw
scans, fetching data or following embedded commands/paths. It does not establish
accuracy, GT alignment, conditioning, binary/input authenticity or a solver
cause. No setting is selected; KITTI07 selected the inherited plane check and
KITTI00 influenced the original diagnosis. Native source/CMake are identical to
the integrated CLI parent, and the frozen 177-cell replay is untouched.
''')
for rel, addition in [
    ('papers/inten_loam/README.md', '''\n<!-- INTEN_TRACE_VALIDATION_ACCELERATION -->
## Offline validation of accelerated exports

The unchanged reader accepts all 4 committed default/accelerated export tuples
with exit 0, preserving their input bytes (`b1ef79df`). The original legacy
corpus and synthetic corruptions remain separate. No estimator or raw scan is
opened by these checks. [Commands, hashes and limits](../../experiments/results/inten_trace_validation/integration/README.md)
cover consistency only; full-sequence accuracy and solver cause remain open.
<!-- /INTEN_TRACE_VALIDATION_ACCELERATION -->
'''),
    ('docs/inten_loam_passive_trace.md', '''\n## Validate integrated accelerated exports

The reader also accepts the additive effective acceleration field. All 4
committed export tuples validate with exit 0 using the unchanged checker source;
input bytes remain identical. Evidence was committed at `b1ef79df` before this
statement. [Exact commands and reports](../experiments/results/inten_trace_validation/integration/README.md)
preserve the same consistency-only limits as the original checker. Legacy
schema-version-1 logs remain part of its existing regression corpus.
'''),
    ('experiments/results/inten_trace_validation/README.md', '''\n## Integration with accelerated exports

The [later compatibility check](integration/README.md) retains the original
study and validates the combined API/speed/export records with the unchanged
reader. It adds no estimator or parameter-selection path.
''')]:
    p = W / rel
    assert addition.strip() not in p.read_text()
    p.write_text(p.read_text().rstrip() + '\n' + addition)
print('Documented committed reader compatibility; tool/test/native sources unchanged.')
