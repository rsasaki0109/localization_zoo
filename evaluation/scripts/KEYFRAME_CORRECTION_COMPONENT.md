# Keyframe Correction Component

This component turns keyframe registration output into bounded corrections for
an existing pair-prior trajectory.

It is intentionally split into three stages:

1. Local motion prior
   - Input: selected pairwise odometry JSON.
   - Current best source: `selected_prior_stability_motion_0p0025_{window}.json`.

2. Keyframe correction candidate
   - Input: `kiss_keyframe_odometry` JSON with correction transform fields.
   - Required candidate fields:
     - `keyframe_attempted`
     - `keyframe_anchor_accepted`
     - `keyframe_accepted`
     - `keyframe_decision_reason`
     - `keyframe_correction_dx_m`
     - `keyframe_correction_dy_m`
     - `keyframe_correction_transform_yaw_deg`
     - `keyframe_correction_m`
     - `keyframe_correction_yaw_deg`
     - `keyframe_rmse_m`
     - `keyframe_correspondences`

3. Correction gate
   - Implemented by `KeyframeCorrectionConfig` in
     `keyframe_correction_component.py`.
   - Current product-risk recipe:
     - `max_keyframe_correction_m = 0.5`
     - `max_keyframe_yaw_deg = 5.0`
     - `max_keyframe_rmse_m = 1.2`
     - `min_keyframe_correspondences = 6000`

The gate does not use ground truth. Ground truth is used only by benchmark
runners to compute metrics.

`keyframe_decision_reason` is shared with the Python component:

- `not_attempted`
- `anchor_rejected`
- `correction_translation_gate`
- `correction_yaw_gate`
- `rmse_gate`
- `correspondence_gate`
- `missing_correction_transform`
- `accepted`

`keyframe_anchor_accepted` represents the registration candidate validity.
`keyframe_accepted` represents the final runtime gate decision. This keeps
candidate generation separate from correction gating and lets Python replay
different gates from the same candidate log.

## Recipe

Pinned recipe:

```text
experiments/results/intensity_odometry/recipes/stability_motion_kiss_keyframe_corr6000.json
```

Run the component recipe on the 60-frame benchmark:

```bash
python3 evaluation/scripts/run_keyframe_corrected_prior_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --anchor-dir-name kiss_keyframe_i10_c0p5_relog \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/stability_motion_keyframe_component_recipe \
  --recipe-json experiments/results/intensity_odometry/recipes/stability_motion_kiss_keyframe_corr6000.json
```

Expected 60-frame aggregate:

| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) |
| --- | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | 0.060 |
| Stability motion + keyframe recipe | 0.605 | 0.528 | 1.059 | 0.071 |

## API

Use `apply_keyframe_corrections()` from
`evaluation/scripts/keyframe_correction_component.py` for new runners. The CLI
wrappers should only load JSON, call the component, and write summaries.
