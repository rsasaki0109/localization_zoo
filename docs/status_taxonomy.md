# Variant Status Taxonomy

This page defines the per-variant `status` values used in aggregate JSON, runner output, and downstream docs.
The goal is to make a benchmark failure tell the reader *why* it failed, not just that it failed.

## Current implementation status

The runner and C++ dogfooding binaries emit the lower-case target taxonomy (`ok`, `skipped`, `timeout_budget`) as of the status-taxonomy refactor. The richer reasons below (`tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`) are reserved enum values that future detection logic inside `pcd_dogfooding`, `multimodal_dogfooding`, and `run_experiment_matrix.py` will surface — at the time of writing, no code path emits them yet.

Older aggregate JSON files predating this refactor may still carry the legacy uppercase values (`OK`, `SKIPPED`, `TIMED_OUT`); the runner normalizes them at ingest using the migration table below, so they remain readable.

## Target taxonomy

| Status | Meaning | Treated as "valid metric"? |
|--------|---------|----------------------------|
| `ok` | Variant finished within the budget profile and produced metrics (ATE/RPE/FPS as applicable). | Yes |
| `timeout_budget` | Variant did not finish within the per-variant timeout. The algorithm was working; the budget did not fit it. | No |
| `tracking_lost` | The algorithm reported tracking failure or trajectory divergence partway through. | No |
| `init_failed` | The algorithm could not initialize (e.g. IMU bias unobservable, insufficient features). | No |
| `input_unsupported` | The dataset variant cannot be consumed by this method (e.g. missing camera intrinsics for a camera-aware family, wrong sensor layout). | No |
| `metric_invalid` | A trajectory was produced but GT alignment or scale recovery failed; the metric is not reliable. | No |
| `no_gt` | The estimate exists but no ground truth was supplied for this run (qualitative-only). | No |
| `skipped` | The variant was intentionally not executed (e.g. manifest precondition not met, dataset not present locally). | No |

## Why this matters

A timeout under `practical_full_300s` is a fair measurement of the chosen profile — it should not be hidden behind a generic "fail". An `init_failed` on a sequence with no IMU is a dataset mismatch, not an algorithm regression. Mixing those into one bucket makes the docs less useful and hurts the OSS's reputation for honesty.

## Migration table

The runner normalizes any legacy strings it encounters when reading older aggregate JSON, so downstream consumers can compare against the lower-case target enum directly:

| Legacy emitted value | Mapped target value |
|----------------------|---------------------|
| `OK` | `ok` |
| `TIMED_OUT` | `timeout_budget` |
| `SKIPPED` | `skipped` |

Reserved-but-not-yet-emitted values (`tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`) will start appearing once:

1. `pcd_dogfooding` and `multimodal_dogfooding` set `MethodResult::status` to a specific reason on early exit (the field exists; only detection logic is missing).
2. `run_experiment_matrix.py` surfaces the C++ side reason instead of collapsing every non-ok path to a single failure label.
3. Aggregate JSON consumers (`generate_paper_comparison.py`, `export_benchmark_site.py`) treat the new values as non-valid while still rendering the reason in tables.

## Forward compatibility

- Status values are **lower-case** strings.
- Unknown values must be treated as non-valid metrics and surfaced verbatim by docs generators.
- Aggregate consumers must not assert against any closed enum; they should compare to `ok` for validity and pass everything else through.

## Cross-references

- Budget design: [`docs/budget_profiles.md`](budget_profiles.md)
- Reproduction labeling: [`docs/reproduction_status.md`](reproduction_status.md)
- Family registry: [`experiments/families.json`](../experiments/families.json)
