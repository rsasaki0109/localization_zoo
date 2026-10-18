from pathlib import Path
import json,subprocess
R=Path(__file__).resolve().parent;REPO=R/'worktree';E=REPO/'experiments/results/inten_trace_validation'
path='experiments/results/inten_trace_validation/evidence/final/corpus/summary.json'
raw=subprocess.check_output(['git','show','e27ff091:'+path],cwd=REPO)
assert raw==(REPO/path).read_bytes();data=json.loads(raw)
assert data['exit_code']==0 and len(data['valid'])==3 and len(data['invalid'])==7
if (E/'README.md').exists():
 print('Committed result documentation already applied');raise SystemExit(0)
text='''# Offline passive trace validation

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
'''
for item in data['valid']:
 d=item['report'];text+=f"| {item['case']} | {d['input_frames']} | {d['processed_frames']} | {d['skipped_frames']} | {d['stages']['s2s']['calls']} / {d['stages']['s2m']['calls']} |\n"
text+='''
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
/workspace/localization-env/host-venv/bin/python experiments/results/inten_trace_validation/validate_corpus.py \\
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
'''
(E/'README.md').write_text(text)
p=REPO/'docs/inten_loam_passive_trace.md';s=p.read_text();s+='''

## Validate a stored trace

Use the offline checker with a trace, its normal native summary, and the
collector's native command/exit receipt. For a committed example:

```bash
trace_run=experiments/results/inten_trace_export/evidence/fixture/paper_odometry_plane_check/candidate_on
python3 evaluation/scripts/check_inten_trace.py \\
  --trace "$trace_run/trace.jsonl" \\
  --summary "$trace_run/summary.json" \\
  --receipt "$trace_run/command.json" \\
  --output /tmp/inten-trace-check.json
```

The output path must be new. Check the command's exit status directly;
invalid input exits nonzero and creates no report. Existing output bytes are
preserved. All trace validation completes before a report is opened.

The receipt requires integer `exit_code: 0`. Its `native_argv` must match
the header argv exactly; alternatively its array-valued `command` must end
with that complete native argv, as with a Docker command prefix. Shell
command strings are not interpreted. Summary input paths, selected method,
status and frame counts must agree. Embedded paths are compared as strings;
the checker opens only the files explicitly passed on its command line.
Archived traces therefore need no original scan tree or executable.

The checker streams one record at a time. It requires the supported schema,
complete header/end, every input index, contiguous estimator call indices,
explicit skips, matrix/vector dimensions, class/total pair counts, typed
step flags and consistent termination. An unattempted iteration terminates
with insufficient pairs; a failed nonfinite step terminates with its
explicit tagged delta. Uncalled stages retain placeholder matrices and
contribute no solver calls/iterations to the counters.

The JSON report contains file hashes, frame/skip counts, stage call/iteration
counts, stop counts and the number of explicit nonfinite values. It retains
`NaN`, `+Infinity` and `-Infinity` as observed tags during validation and
rejects nonstandard JSON numeric literals or duplicate keys. No tag is
replaced with a zero update.

Validation covers record/run consistency. Accuracy, matrix conditioning,
GT remapping and binary/input/source authenticity require separate evidence.
Matching argv alone cannot distinguish repeated executions with identical
arguments; preserve the collector's hashes and provenance. In particular,
explicit skips need GT remapping before the inherited accuracy fields can
be interpreted. The tool adds no estimator gate or setting.

[Committed checker tests, corruptions and commands](../experiments/results/inten_trace_validation/README.md)
cover this contract. Full failure-sequence tracing and cause diagnosis
remain open.
''';p.write_text(s)
p=REPO/'papers/inten_loam/README.md';s=p.read_text();s+='''
<!-- INTEN_TRACE_VALIDATION -->
## Offline trace validation

The manually invoked `evaluation/scripts/check_inten_trace.py` streams a
saved passive trace with its native summary and command/exit receipt.
It checks completion, input/call indices, stage/iteration structure and run
consistency before publishing counters. This analysis tool is outside the
paper and leaves native code/settings unchanged.

All 9 unit/CLI tests pass with exit 0. All 3 committed normal/lifecycle traces
validate with exit 0; all 7 explicit synthetic corruptions return exit 1
and create no report. Evidence was committed at `e27ff091` before these
numbers. [Results, negative cases and exact commands](../../experiments/results/inten_trace_validation/README.md)
retain the scope: accuracy, GT indexing, conditioning and solver cause
require separate evidence. The full failure diagnosis remains open.
<!-- /INTEN_TRACE_VALIDATION -->
''';p.write_text(s)
p=REPO/'docs/roadmap.md';s=p.read_text();needle='    handling separately from full failure-sequence diagnosis.';assert s.count(needle)==1
s=s.replace(needle,needle+'\n    An [offline streaming checker](../experiments/results/inten_trace_validation/README.md)\n    validates completion, stage structure and native run receipts before\n    analysis; it adds no estimator gate or failure-cause finding.');p.write_text(s)
print('Documented committed offline validation results')
