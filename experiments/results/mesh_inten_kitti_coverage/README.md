# Fixed Mesh-LOAM / InTEn-LOAM KITTI coverage

The selected variants are `paper_odometry_elevation` and
`paper_odometry_plane_check`. Their arguments are copied unchanged from each
method's sequence 00 manifest. The method READMEs record which development
sequences influenced these choices. Existing 00/07 results are reused; the
remaining sequences are new full-sequence executions.

## Execution evidence

- `environment.json`: binary source commit and SHA256, Release build flags,
  toolchain, library versions and thread settings.
- `seq_NN_commands.log`: exact downloader, matrix-runner and diagnostic
  commands with their actual return codes. Downloads use
  `evaluation/scripts/fetch_kitti_odometry_sequences.py`; completed files are
  retained across a failed download retry.
- `parallel_execution.json`: exact InTEn-LOAM commands and isolated-output
  launcher. Both methods may compute on the same sequence concurrently;
  each uses two OpenMP threads and one OpenBLAS thread. The launcher only
  changes the output working directory, using the same benchmark binary
  and method arguments.
- `seq_NN_complete.json`: both full-frame results were checked before the
  raw scans for that sequence were deleted. Only then was the next sequence
  fetched.
- `import_commands.json`: canonical re-aggregation commands and return
  codes, using `--merge-existing-index --reuse-existing`.
- `historical_aggregate_sha256.json`: historical aggregate hashes verified
  unchanged after import.
- `regeneration_commands.json` and `regenerate_*.log`: derived-document
  generation commands and actual return codes. The post-regeneration
  historical aggregate check is recorded separately.
- `transport_recovery.json`: a lost tool connection was recovered while
  the benchmark processes continued. Actual per-command exit codes remain
  in the execution logs; the old top-level tool handles are unavailable.
- `controller_resume_exit.json` / `helper_resume_exit.json` and their
  stdout logs: actual resumed coordinator exit statuses. Saved sequence 06
  estimator outputs were reused for diagnostics and cleanup after the
  original coordinator stopped; the estimators were not rerun.
- `checks/`: check commands, logs and actual exit codes. CTest and the full
  Python suite were run after the Release build; the final documentation
  checks run again after regeneration.
- `validation_commands.json`: full-frame coverage and fixed Table 6 variant
  checks, unchanged paper references and historical aggregates, and the
  source/document whitespace check, with commands and actual return codes.

Run logs retain the benchmark's original whitespace. Source and document
whitespace checks exclude these raw `*.log` files.

New run directories under `experiments/results/runs/` contain `summary.json`,
`host.json`, `run.log`, estimated KITTI poses, the evaluated ground truth,
its evaluation manifest, and `step_diagnostics.json`. Raw scans are excluded.
The saved-pose diagnostics rescore official full-sequence KITTI RTE and check
it against the original summary. Their shorter blocks locate drift and
are not the official KITTI metric.

## Reporting

Commit the imported execution evidence before generating numeric README
claims. From the repository root, regenerate the fixed-setting README tables
and their numerical source summary with:

```bash
python3 experiments/results/mesh_inten_kitti_coverage/generate_fixed_coverage_readmes.py
```

`summary.json` records the input commit and exact generation command. Table 6
restricts these two methods to their selected variants through `repo_variants`
in `evaluation/data/paper_reported_numbers.json`; historical ablations remain
in their original aggregates. Regenerate the other derived files with the
commands in `docs/roadmap.md`.

FPS comes from the recorded runs, including overlapping execution and the
historical 00/07 runs; this coverage exercise does not establish a controlled
cross-machine throughput comparison.
