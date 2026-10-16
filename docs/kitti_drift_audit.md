# Replaying KITTI result cells

`evaluation/scripts/audit_kitti_drift.py` checks the best stored RPE variant for
each method and full KITTI sequence. It uses the recorded dataset arguments and
variant arguments without selecting new parameters. The audit covers all stored
full-sequence cells by default, including sequences outside the README's columns.
This is a reproducibility check; it does not replace the fixed-setting paper
comparison or select a new setting for a method.

Inspect the selection before downloading scans:

```bash
python3 evaluation/scripts/audit_kitti_drift.py --list --sequence 00
python3 evaluation/scripts/audit_kitti_drift.py --list --method SuMa --sequence 01
```

`--method` takes the primary method name stored in the aggregate, including case.
Repeat `--sequence` or `--method` to select several. An empty selection, unknown
method, or missing explicitly requested method/sequence pair is an error.

For a sequence at a time, first use the official selective downloader:

```bash
python3 evaluation/scripts/fetch_kitti_odometry_sequences.py \
  --output-root /tmp/localization-kitti --sequence 00
mkdir -p /tmp/localization-audit-data/dogfooding_results
ln -s /tmp/localization-kitti/dataset/sequences/00/velodyne \
  /tmp/localization-audit-data/dogfooding_results/kitti_seq_00_full
python3 evaluation/scripts/audit_kitti_drift.py \
  --sequence 00 --data-root /tmp/localization-audit-data \
  --binary build/evaluation/pcd_dogfooding \
  --work-dir /tmp/localization-audit-00 \
  --output /tmp/localization-audit-00.json
```

Check the exit code directly. The report also stores it:

| Exit | Meaning |
|---|---|
| `0` | Every selected cell matches its stored RPE, ATE and frame count. |
| `1` | A completed replay changed RPE, ATE or frame count. |
| `2` | Invalid selection or inputs, a failed process, or an invalid/missing result. |

RPE and ATE use an absolute tolerance of `1e-6` in their respective units; frame
counts must match exactly. Missing, negative or non-finite error metrics cannot
pass. A failed process cannot pass by leaving a plausible summary behind. When
one cell fails, the report still includes the other selected cells. Its exit is
`2` if any cell failed, otherwise `1` if any cell drifted.

Each row records the exact replay argv, working directory, process exit code,
summary path and combined stdout/stderr log path. The report records the selected
scope, binary SHA-256 and host. Use a new or empty work directory so a later run
cannot accidentally consume a stale summary. Official KITTI RTE is recorded when
the evaluator has valid segments; it is not substituted for the stored RPE.

Filtered runs require an explicit `--output`, preserving the historical full
audit by default. A filtered report proves only its listed cells. Keep the report,
logs and summaries together under a dedicated directory in `experiments/results/`
when committing an audit. A full audit is complete only after every intended cell
has been checked; a passing filtered report does not close the remaining cells.
The leaderboard's verification marker excludes rows with ATE/frame drift or
execution errors even when their RPE is unchanged.

After preserving the report and replay artifacts, delete the downloaded sequence
and its temporary symlink before fetching the next sequence. Keep committed
ground truth and historical result aggregates. Investigate and document a drift;
do not change a stored value merely to make the audit pass.

The audit tooling's [validation records](../experiments/results/kitti_audit_tooling/README.md)
use an isolated test workspace and the committed short MCD fixture. They verify
commands, result checks and exit codes; they are not a new full KITTI audit.
