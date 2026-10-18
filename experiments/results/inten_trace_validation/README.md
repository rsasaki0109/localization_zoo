# Offline passive trace validation

`evaluation/scripts/check_inten_trace.py` validates the streamed InTEn records
before offline analysis. It is manually invoked repository tooling, outside
the paper. It performs no native run, supplies no pose and selects no setting.
This branch depends on [PR #141](https://github.com/rsasaki0109/localization_zoo/pull/141).

The [protocol](protocol.json) pins the current tool source and corpus.
All inputs are committed parent traces from the default and unchanged
KITTI 07-selected plane-check settings, transferred to the short MCD fixture;
00 influenced the earlier plane diagnosis. The lifecycle case substitutes an
empty middle scan. No new scans are read or fetched.

## Closed validation

Final evidence was committed at `e27ff0917ceec332a0ca12208811cc5c47629e49`
before these numbers. All 9 mechanism/CLI unit tests pass with actual exit 0.
All 3 committed trace tuples validate with exit 0:

| Trace | Selected inputs | Estimator calls | Explicit skips | S2S/S2M calls |
|---|---:|---:|---:|---|
| default | 3 | 3 | 0 | 2 / 2 |
| plane_check | 3 | 3 | 0 | 2 / 2 |
| skipped_input | 3 | 2 | 1 | 1 / 1 |

All 7 synthetic corruption cases are rejected with exit 1 and create no
success report: missing end, data after end, inconsistent footer, JSON NaN,
a receipt from different arguments, failed-native receipt, and a mismatched
method frame count. The original source files remain byte-identical.
Unit tests additionally cover duplicate keys, unknown schema, malformed
support/matrices/flags, an unattempted iteration followed by more steps,
large integers, tagged failed-step values and output-file preservation.
These are parser/contract checks; odometry accuracy is outside this study.

[Final unit command/log](evidence/final/unit/),
[complete corpus summary](evidence/final/corpus/summary.json) and
[driver receipt](evidence/final/corpus_command/command.json) retain actual
exits and hashes. Earlier closed checks remain unchanged in `evidence/`;
the current-source results are under `evidence/final/`.

The corpus command executed from the repository root was:

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_validation/validate_corpus.py \
  --output-dir /workspace/localization-env/inten-trace-validator/final_corpus
```

Use a fresh output directory. This regenerates explicit synthetic corruption
from the committed traces and runs only the Python checker.

## Scope

[Usage and contract](../../../docs/inten_loam_passive_trace.md#validate-a-stored-trace)
explain the required native receipt and input/estimator index distinction.
The report covers structure and run consistency. Motion accuracy, matrix
conditioning, GT indexing, binary/input authenticity and solver cause need
separate evidence. Matching argv cannot distinguish repeated runs using the
same arguments; keep collector/source/input hashes with the experiment.

The empty-input case has explicit skips. The inherited runner removes an
empty scan without retiming GT; remap GT before interpreting its accuracy
fields. No accuracy number from that case is advertised here.

All native source/CMake, old result studies, published aggregate hashes,
Table 6, root README/HTML leaderboard and paper-number JSON are preserved.
The original whole-zoo replay continues in its separate frozen tree. Native
build/CTest evidence is inherited from the unchanged parent implementation;
this Python-only change requires no local native rebuild or replay.
