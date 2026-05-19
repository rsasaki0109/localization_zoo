# Minimal Interfaces

_Generated at 2026-05-19T20:32:55+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Stable Core

### CLI

`<stable_binary> <pcd_dir> <gt_csv> [max_frames] --methods <selector> --summary-json <path> [dataset flags...] [variant flags...]`

Current active binaries: `build/evaluation/pcd_dogfooding`

### Summary JSON Contract

| Field | Type | Meaning |
|-------|------|---------|
| `pcd_dir` | string | Input PCD sequence directory. |
| `gt_csv` | string | Reference trajectory CSV matched to the sequence. |
| `num_frames` | integer | Number of frames evaluated. |
| `trajectory_length_m` | number | Total GT path length in meters. |
| `timestamp_source` | string | Timestamp source used by the evaluator. |
| `methods[]` | array | Per-method results emitted by the benchmark. |
| `methods[].name` | string | Human-readable method name. |
| `methods[].status` | string | Per-variant status from the target taxonomy in `docs/status_taxonomy.md` (`ok`, `skipped`, `timeout_budget`, ...). Legacy uppercase values (`OK`/`SKIPPED`/`TIMED_OUT`) are still accepted at ingest and normalized to the new enum. |
| `methods[].ate_m` | number or null | Absolute trajectory error in meters. |
| `methods[].rpe_trans_pct` | number or null | Average 100 m relative translation error in percent. |
| `methods[].rpe_rot_deg_per_m` | number or null | Average 100 m relative rotation error in degrees per meter. |
| `methods[].frames` | integer | Number of poses evaluated for the method. |
| `methods[].time_ms` | number or null | End-to-end runtime in milliseconds. |
| `methods[].fps` | number or null | Effective frames per second. |
| `methods[].note` | string | Free-form method note or skip reason. |

## Experimental Layer

### Manifest Contract

Every active search problem lives in `experiments/*.json` and must define:

| Field | Type | Meaning |
|-------|------|---------|
| `problem.id` | string | Stable identifier for the search problem. |
| `problem.state` | string | `ready` or `blocked`. Missing means `ready`. Generated outputs may downgrade a `ready` problem to `skipped` if every variant is skipped. |
| `problem.blocker` | string | Why the problem cannot be benchmarked yet. Optional for blocked problems. |
| `problem.next_step` | string | The next concrete step to unblock the problem. Optional for blocked problems. |
| `problem.variant_timeout_seconds` | number | Optional per-variant wall-clock timeout budget used by the runner. |
| `problem.dataset` | object | Shared dataset paths for every variant. |
| `problem.dataset.extra_args` | array | Optional fixed CLI args shared by every variant for that dataset. |
| `stable_interface.binary` | string | Stable benchmark entrypoint. |
| `stable_interface.methods` | string | Shared method selector for comparability. |
| `stable_interface.primary_method` | string | Result key to extract from summary JSON. |
| `variants[]` | array | Concrete variants to compare, keep, or discard. |
| `variants[].args` | array | Extra CLI flags layered on the stable core. |

Current active selectors: `ct_icp`

### Runner Contract

`python3 evaluation/scripts/run_experiment_matrix.py [--manifest <path>]... [--reuse-existing] [--reuse-aggregates] [--variant-timeout-seconds <seconds>]`

If no manifest is specified, the runner executes every `experiments/*_matrix.json` file.

The runner is responsible for:

- running every variant against the same dataset and method selector
- saving per-run summary JSON and logs under `experiments/results/runs/`
- regenerating `docs/experiments.md`, `docs/decisions.md`, and `docs/interfaces.md`
- writing per-problem aggregate JSON files plus `experiments/results/index.json`
- optionally reusing existing per-variant summaries to avoid rerunning expensive variants
- optionally reusing existing aggregate JSON files to rebuild docs without touching local data
- marking timed-out variants in aggregate results instead of blocking the whole study

## Stability Boundary

- Stable core: the configured stable binary plus the `--summary-json` result contract.
- Experimental surface: manifests, run logs, aggregate results, and generated decision docs.
- Promotion rule: a new default must emerge from shared data and shared metrics, not from a separate code path.

## Active Problems

| Problem | Status | Manifest | Selector | Current Default | Aggregate |
|---------|--------|----------|----------|-----------------|-----------|
| CT-ICP recipe sensitivity test on MCD TUHH night_09 (108 frames) | `ready` | `experiments/ct_icp_mcd_tuhh_night_09_recipes_matrix.json` | `ct_icp` | `dense_reference` | `experiments/results/ct_icp_mcd_tuhh_night_09_recipes_matrix.json` |
