# Recursive mapped-motion S2S seed: full KITTI comparison

This is a repository extension **outside the paper, default off**:
`--inten-loam-paper-set s2s_mapped_prior=1`.

S2S normally reuses its own preceding accepted relative estimate, before S2M
correction. The candidate instead seeds it with the preceding final mapped
relative pose (`previous_pose.inverse() * current_pose`), updated recursively.
The relative rotation is restored through a normalized quaternion, with a
homogeneous bottom row; translation is preserved. Startup uses identity. The accepted S2S cache, deskew, low-count fallback,
associations, robust weights, solver budgets and mapping remain unchanged.
Skipped or rejected S2M still contributes the actual final relative motion.
Mapping-off and forced-pose diagnostic paths keep their original behavior.
The [initial unprojected trial](../inten_mapped_prior/README.md) completed 07
with worse accuracy and aborted 00 after non-rigid roundoff amplified. That
trial and every original command remain stored.00 informed this numerical
correction; this revision restarts every sequence.

No new threshold is chosen; failed row-support completion remains off in both arms.

The hypothesis is that S2M corrections could stop an S2S prediction deficit
from feeding back into the next registration. The earlier one-shot mapped-seed
[interventions](../inten_seed_intervention/README.md) use an already collapsed
trajectory and do not test prevention by recursive feedback. The GT-seeded
negative controls also show why initialization alone may not fix the solver.

The inherited range/elevation/S2M profile was chosen on 07;00 influenced its
diagnosis. Known 01/02/10 failures motivated this boolean. These are development
data, with **no independent held-out claim**. Full 07 ran first, then all other
sequences, with the same parameters and existing acceleration in every arm.
Both arms use continuous natural prediction, passive traces and `--no-gt-seed`.
The candidate is not selected separately per sequence.

@@RESULTS@@

## Reproduce and audit

- [Frozen protocol](protocol.json) and its hashed full-sequence manifests.
- [Native binaries/dependencies](binaries.json), [build/tests](checks/build/),
  [CLI trace parity](checks/cli_preflight/), [actual runtime](runtime.json).
- `collect.py`: official acquisition one sequence at a time, exact scan/GT
  hashes, full native invocation, trace validation and saved-motion diagnosis.
  Every raw archive is committed and normally pushed before deleting owned
  scans; cleanup is committed and pushed before the next acquisition.
- [Collection execution](collection_execution/command.json) and
  [backup execution receipts](backup_execution/) retain actual exits.
- Full 07 default-off control matches the immutable PR155 runner. All inherited
  natural-seed fixed-pair controls on 01/02/10 check default-off estimates,
  first class normal equations and counts; these are compatibility controls,
  not accuracy measurements.
- [Postprocessing protocol](postprocessing_protocol.json): sources committed
  before derivation, full numerical report committed before README prose.
- [Report invocation](report_execution/command.json),
  [import invocation](import_execution/command.json),
  [derived document generation](docs_execution/) and [final checks](checks/final/).
- Earlier aggregates, scientific studies and publication files are preserved.
  Both arms remain non-default and unscored. Raw timing survives in summaries,
  but concurrent processes do not support a standalone FPS comparison.

```bash
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_mapped_prior_rigid/build.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_mapped_prior_rigid/collect.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_mapped_prior_rigid/report.py \
  --output-dir experiments/results/inten_mapped_prior_rigid/report
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_mapped_prior_rigid/import_results.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_mapped_prior_rigid/write_docs.py
```

Build and collection require fresh isolated paths and stop on failed checks.
Original commands, working directories, source commits, hashes and actual exit
codes are stored alongside their execution logs. Do not rerun into old evidence.
