# Reader compatibility with integrated accelerated exports

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
