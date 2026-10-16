# Remaining fixed-configuration KITTI paper coverage

This covers roadmap Phase 1 item 2: A-LOAM `kitti_default`, LF-GICP
`paper_default`, L-LO `seq07_tuned`, and Quadric-LO `dense_profile`.
The frozen source manifests, arguments, selection provenance, reused aggregates
and sequence coverage are recorded in
`experiments/kitti_remaining_fixed_coverage_plan.json`. L-LO sequence 01 is
excluded because the paper reports NA. Existing matching results are reused;
new sequences do not select parameters.

## Execution evidence

- `environment.json`: host and toolchain, binary source commit and SHA256,
  verified Release flags, thread settings, and the host-capture command.
- `build_commands.json` and `build_*.log`: exact configure/build commands and
  actual return codes.
- `seq_NN_fetch_commands.json` / `.log`: repository downloader commands,
  actual return codes and any retries; verified files are retained on retry.
- `seq_NN_METHOD_commands.json` / `.log`: benchmark and saved-pose diagnostic
  commands with actual return codes.
- `METHOD_launcher.json`: exact isolated-output launcher. Methods may overlap
  only within the same sequence. Each uses one OpenMP and one OpenBLAS thread;
  the binary and fixed method arguments are unchanged by the launcher.
- `seq_NN_METHOD_complete.json`: checked full-frame result and saved paths.
- `seq_NN_complete.json`: all applicable methods were saved and diagnosed
  before that sequence's scans were deleted and the next sequence fetched.
- `controller_exit.json` / `controller_stdout.log`: persistent coordinator
  output and actual process exit status.
- `transport_recovery.json`: a tool connection loss was recovered while the
  benchmark and postprocessing processes continued; only monitoring was restarted.
- `historical_aggregate_sha256.json` and its import/regeneration checks:
  historical aggregate bytes are preserved.
- `import_commands.json` / `import_stdout.log`: canonical diagnostics and
  re-aggregation using `--merge-existing-index --reuse-existing`.
- `regeneration_commands.json` / `regenerate_*.log`: required derived-document
  generation commands and actual return codes.
- `checks/`: check commands, logs and actual exit codes.
- `postprocess_commands.json` / `postprocess_stdout.log`: automatic import,
  raw-evidence commit, report generation and check receipts.
- `validation_commands.json` / `validation_*.log`: replayable coverage,
  fixed-configuration, provenance, historical-result and cleanup checks.
- `render_commands.json` / `render_stdout.json`: final renderer execution
  against the committed input recorded in `summary.json`.

New canonical run directories under `experiments/results/runs/` retain the
summary, host, original stdout log, estimated KITTI poses, evaluated GT,
evaluation manifest and step diagnostics. Raw scans are excluded.
Diagnostics rescore the official full-sequence RTE and match the original
summary before locating drift in shorter blocks. Block endpoint errors are
diagnostics, not official KITTI RTE.

## Reporting

Commit all imported execution evidence before generating numeric claims.
After pinning these methods to their fixed variants through `repo_variants`
in `evaluation/data/paper_reported_numbers.json`, run from the repository root:

```bash
python3 experiments/results/remaining_kitti_coverage/generate_fixed_coverage_readmes.py
```

The renderer reads committed aggregates and paper references, verifies
configuration/full-frame coverage, and records its input commit and command
in `summary.json`. LF-GICP 07 has repeated historical records with the same
arguments; Table 6 pool size can count those records while the configuration
remains fixed.

A-LOAM is compared with LOAM values cited by later papers, not an A-LOAM
per-sequence author table. Quadric-LO 02 already has a separate fallback
validation in its README and is not claimed as previously unseen. LF-GICP's
selection note discloses the calibration trace and earlier 02/08 observations.

FPS describes the recorded execution, including overlap and reused results
on their historical hosts; it is not a controlled cross-machine throughput
comparison. Raw logs retain their original whitespace; source/document
whitespace checks exclude these logs.

Overview means use unrounded committed RTE values. The Table 6 generator's
console mean uses its rounded per-sequence ratio cells; this rounds LiTAMIN2
to 1.00x instead of the raw-value mean 1.005474... (1.01x). The README and
claim overview now use 1.01x, matching the roadmap. `overview_ratio_audit.json`
records an audit of every overview mean against the committed source rows.
