# Weak-translation prediction prior: full KITTI comparison

This repository extension is outside the paper and default off:
`--inten-loam-paper-set s2s_translation_prior_ratio=0.03`.
The default is zero, preserving the original execution path. This experiment
compares ratio zero with the one fixed ratio declared in the protocol.
Both arms enable the preceding recursive rigid mapped-motion seed and the
same acceleration. Failed row-support completion remains off.

The prior raises only weak sensor-centered marginal translation information
to a fraction of its strongest eigenvalue. Its anchor is the initial S2S
prediction, not ground truth; its information stays fixed throughout that
solve. It can preserve a biased prediction and oppose actual acceleration.
It changes no association, iteration budget or S2M term. See the source and
protocol for the position Jacobian and rotation marginalization convention.
Total passive H/g and residual cost include the prior. Feature counts and
forced class equations still report data terms only. Nonzero prior information
is retained in the optional solver field; disabled/default trace bytes match
the parent format.

The original numerical profile was selected on KITTI07 with00 influencing
earlier diagnosis. The dimensionless fraction0.03 is borrowed from the
[existing DiLO KITTI01-tuned control](../../../papers/dilo/README.md). Here it
sets a soft information floor after rotation marginalization, rather than
DiLO's hard remapping of the raw translation block. It was fixed before all
new InTEn accuracy runs and not retuned on a new sequence score. All inherited
GT-seeded01/02/10 systems
were inspected; known failures and prior full results are development data,
not independent held-out evaluation. Full07 runs first, then all00-10 without
retuning, including negative outcomes. Both arms use continuous natural
prediction, passive traces and `--no-gt-seed`.

@@RESULTS@@

## Commands and evidence

- [Protocol](protocol.json) includes hashed manifests, fixed settings and limits.
- [Inherited information inspection](information_command.json), all cases in
  [information.json](information.json), source in inspect_information.py.
- [Fresh native build and tests](checks/build/), [binary pins](binaries.json),
  [runtime capture command](runtime_command.json), [CLI trace parity](checks/cli_preflight/).
  The first test attempt is retained in checks/initial_build: class-wise sums
  differed from serial totals at double-precision roundoff, and the test now
  compares a relative tolerance. Native behavior was not changed for this test.
- [Collection invocation](collection_execution/command.json) and every native
  receipt retain argv, working directory, source, thread settings and exits.
  The official fetch script acquires only one sequence at a time. Inputs are
  hashed against the earlier official audit; complete lossless evidence is
  round-trip checked, committed and normally pushed before scan deletion.
  Cleanup is committed and pushed before acquiring the next sequence.
- Full07 ratio-zero control exactly matches the immutable PR156 enabled runner.
  Inherited natural-seed controls check the original default-off pair behavior.
- [Postprocessing protocol](postprocessing_protocol.json),
  [report command](report_execution/command.json), [import command](import_execution/command.json),
  [document generation](docs_execution/), [final checks](checks/final/).
  Sources are committed before execution, and numerical evidence before prose.
- Old aggregates, published paper comparisons and prior studies remain intact.
  Neither arm is adopted per sequence. Both remain unscored and non-default;
  native timing is retained but paired processes are not a standalone FPS test.

```bash
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_prior/build.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_prior/collect.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_prior/report.py \
  --output-dir experiments/results/inten_translation_prior/report
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_prior/import_results.py
OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 PYTHONDONTWRITEBYTECODE=1 \
  /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_prior/write_docs.py
```

Build and collection require fresh isolated paths. Do not rerun into frozen
evidence. Every command retains its actual exit; failures are never read as
passes through a shell pipeline. Local checks precede full native/ROS PR CI.
