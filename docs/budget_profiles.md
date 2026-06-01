# Budget Profiles

This page defines the per-variant wall-clock budgets that localization_zoo uses when running benchmark manifests.
Profiles let the repo treat heavy method families (e.g. `orb_slam3`, `lvi_sam`, `r2live`) honestly without silently relaxing the budget for individual methods.

> **Key rule:** Within a profile, every family runs under the **same** budget.
> If a method cannot finish, that is a measurement under the chosen profile, not an opportunity to relax the budget for one family.

## Profiles

| Profile | Window | Per-variant timeout | Intended use |
|---------|--------|---------------------|--------------|
| `smoke_200f_120s` | 200 frames | 120 s | Contract regression and quick local sanity. Not a publishable accuracy result. |
| `practical_full_300s` | Full sequence | 300 s | Default for the primary leaderboard / docs tables. Reflects what an OSS user can run without an industrial machine. |
| `extended_full_1800s` | Full sequence | 1800 s | Secondary profile for heavy multimodal pipelines (`orb_slam3`, `lvi_sam`, `r2live`) and very long public sequences. Reported separately. |
| `reference_full_unbounded` | Full sequence | No timeout | Diagnostic only. Used when chasing a paper-reported number under a known-faithful protocol. Never the primary number. |

## Mapping to manifest fields

A manifest opts into a profile via the new `problem.budget_profile` field. The runner already honors `problem.variant_timeout_seconds`; `budget_profile` is the recommended way to express budget intent and the timeout value should be set to match.

```json
{
  "problem": {
    "id": "litamin2_kitti_seq_00_full",
    "window_frames": null,
    "budget_profile": "practical_full_300s",
    "variant_timeout_seconds": 300
  }
}
```

If both `budget_profile` and `variant_timeout_seconds` are set, the runner uses the explicit numeric value but the profile name is propagated to the aggregate JSON for downstream docs.

## Reporting rules

- **Primary tables** (`docs/experiments.md`, `docs/paper_comparison.md`): use `practical_full_300s` for full-sequence reports and `smoke_200f_120s` for short-window contract checks. Heavy methods are listed with their actual timeout outcome — not with a silently expanded budget.
- **Secondary tables**: results from `extended_full_1800s` may be added in a separately labeled section. They must not be merged into the primary number.
- **Pareto / leaderboard plots**: only include results from a single profile per axis. Mixing profiles on one plot is forbidden.
- **Diagnostic profile** (`reference_full_unbounded`) is allowed for repro investigation, but cannot be used as the primary reported number.

## Cross-references

- Reproduction labeling: [`docs/reproduction_status.md`](reproduction_status.md)
- Runtime status taxonomy: [`docs/status_taxonomy.md`](status_taxonomy.md)
- Family registry (tier, supports, status): [`experiments/families.json`](../experiments/families.json)
