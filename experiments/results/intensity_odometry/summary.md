# Intensity / Visual-LiDAR Odometry Notes

## Scope

- Preserve `PointXYZI::intensity` in `pcd_dogfooding` for intensity-aware methods.
- Feed preserved KITTI PCD intensity into `isc_loam` instead of zero-filled intensity.
- Generate a CUBE-LIO-style cubemap / IGM front-end artifact for a KITTI frame.

## KITTI Seq 02 Smoke

Command:

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/kitti_seq_02_108 \
  experiments/reference_data/kitti_seq_02_108_gt.csv \
  30 \
  --methods isc_loam \
  --summary-json experiments/results/intensity_odometry/isc_loam_kitti_seq_02_30.json
```

Result:

| Method | Frames | ATE [m] | RPE trans [%] | FPS | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| ISC-LOAM | 30 | 13.206 | 43.181 | 33.8 | Uses preserved PCD intensity in intensity scan context. |

This is still drift-level as odometry, but it now exercises the intended
intensity descriptor path rather than a zero-intensity placeholder.

## CUBE-LIO Front-End Demo

Command:

```bash
python3 evaluation/scripts/cube_lio_cubemap_demo.py \
  --pcd dogfooding_results/kitti_seq_02_108/00000000/cloud.pcd \
  --output-dir experiments/results/cube_lio_demo/kitti_seq_02_frame00000000 \
  --size 256 \
  --montage \
  --no-per-face-pngs \
  --export-features-json
```

Artifacts:

- `experiments/results/cube_lio_demo/kitti_seq_02_frame00000000/cubemap_montage_intensity.png`
- `experiments/results/cube_lio_demo/kitti_seq_02_frame00000000/cubemap_montage_igm.png`
- `experiments/results/cube_lio_demo/kitti_seq_02_frame00000000/features.csv`
- `experiments/results/cube_lio_demo/kitti_seq_02_frame00000000/features.json`

The frame produced 5198 semi-dense IGM feature pixels at 256 px face resolution.

## Direct Intensity BEV Odometry Demo

Added:

```bash
evaluation/scripts/intensity_bev_odometry_demo.py
```

This is a compact intensity-based LiDAR odometry scaffold:

1. rasterize each LiDAR scan to a BEV max-reflectance image,
2. search an SE(2) frame-to-frame transform,
3. score candidates with normalized cross-correlation of reflectance values,
4. integrate the best relative transforms into an odometry trajectory.

Commands:

```bash
python3 evaluation/scripts/intensity_bev_odometry_demo.py \
  --sequence-pcd-dir dogfooding_results/kitti_seq_02_108 \
  --gt-csv experiments/reference_data/kitti_seq_02_108_gt.csv \
  --max-frames 30 \
  --output-json experiments/results/intensity_odometry/intensity_bev_kitti_seq_02_30.json
```

30-frame KITTI smoke:

| Sequence | Frames | ATE xy [m] | Step RMSE [m] | Notes |
| --- | ---: | ---: | ---: | --- |
| 02 | 30 | 0.494 | 0.032 | Stable on this short window. |
| 05 | 30 | 15.140 | 1.706 | Fails badly; current search/model is not universal. |
| 08 | 30 | 4.626 | 0.204 | Partial tracking but significant drift. |

Fast preset + previous-best motion prior:

```bash
python3 evaluation/scripts/intensity_bev_odometry_demo.py \
  --preset fast \
  --sequence-pcd-dir dogfooding_results/kitti_seq_02_108 \
  --gt-csv experiments/reference_data/kitti_seq_02_108_gt.csv \
  --max-frames 30 \
  --output-json experiments/results/intensity_odometry/intensity_bev_fast_motion_prior_kitti_seq_02_30.json
```

| Sequence | Frames | ATE xy [m] | Step RMSE [m] | Runtime [s] | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| 02 | 30 | 0.494 | 0.032 | 1.93 | Same accuracy as broad search on this window. |
| 05 | 30 | 14.562 | 1.610 | 1.93 | Slightly better, still failed. |
| 08 | 30 | 4.874 | 0.281 | 1.90 | Slightly worse than broad search. |

Fast preset + motion sanity gate:

The fast/pyramid presets now reject pairwise estimates with translation norm
above 0.75 m or absolute yaw above 4 deg. Rejected pairs hold the previous valid
motion prior, or zero motion if no valid prior exists.

| Sequence | Frames | ATE xy [m] | Step RMSE [m] | Runtime [s] | Accepted / Rejected | Notes |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| 02 | 30 | 0.494 | 0.032 | 1.66 | 29 / 0 | Unchanged; no obviously bad updates. |
| 05 | 30 | 1.538 | 0.176 | 1.67 | 10 / 19 | Large false lateral updates were suppressed. |
| 08 | 30 | 3.135 | 0.187 | 1.69 | 27 / 2 | Moderate improvement from rejecting 1 m jumps. |

Coarse-to-fine pyramid:

| Sequence | Frames | ATE xy [m] | Step RMSE [m] | Runtime [s] | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| 02 | 30 | 0.494 | 0.032 | 4.50 | Same as fast after anchor candidate fallback. |
| 05 | 30 | 16.767 | 1.346 | 4.46 | Worse ATE despite better step length. |
| 08 | 30 | 4.874 | 0.281 | 4.51 | Same as fast motion-prior result. |

Interpretation:

- The scaffold is useful: it can recover a plausible short-window intensity odometry trajectory on seq02.
- It is not yet a production candidate: seq05/08 expose sensitivity to scene content, motion, and the simple SE(2) BEV model.
- A previous-best motion prior mostly improves runtime usability rather than robustness.
- A motion sanity gate is materially useful as a failure detector: it turns
  obviously impossible image matches into hold/prior updates.
- Coarse-to-fine BEV alone did not improve robustness; the next algorithmic
  step should be CUBE-LIO-style cubemap photometric residuals rather than
  more BEV tuning.

## Direct Intensity Cubemap Odometry Demo

Added:

```bash
evaluation/scripts/intensity_cubemap_odometry_demo.py
```

This is the first CUBE-LIO-style odometry scaffold:

1. project each scan into six reflectance cubemap faces,
2. search an SE(2) current-to-previous transform,
3. rasterize the transformed current scan into the previous cubemap frame,
4. score image overlap with normalized cross-correlation,
5. integrate accepted relative transforms.

Command:

```bash
python3 evaluation/scripts/intensity_cubemap_odometry_demo.py \
  --sequence-pcd-dir dogfooding_results/kitti_seq_02_108 \
  --gt-csv experiments/reference_data/kitti_seq_02_108_gt.csv \
  --max-frames 30 \
  --output-json experiments/results/intensity_odometry/intensity_cubemap_kitti_seq_02_30.json
```

30-frame KITTI smoke, intensity-only, face size 64:

| Sequence | Frames | ATE xy [m] | Step RMSE [m] | Runtime [s] | Accepted / Rejected | Notes |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| 02 | 30 | 0.494 | 0.032 | 2.33 | 26 / 3 | Same ATE as BEV gated. |
| 05 | 30 | 3.929 | 0.310 | 2.37 | 26 / 3 | Much better than ungated BEV, worse than BEV gated. |
| 08 | 30 | 3.476 | 0.193 | 2.41 | 26 / 3 | Slightly worse than BEV gated, better than ungated BEV. |

Interpretation:

- Cubemap projection is useful as a CUBE-LIO-compatible scaffold.
- It reduces some false-match behavior versus raw BEV, but still does not beat
  BEV with a strong motion sanity gate.
- The next useful step is not another grid-search variant; it is a local
  photometric optimizer over the cubemap residual, optionally using IGM features
  as a sparse/semi-dense mask.

IGM / semi-dense scoring smoke:

The cubemap demo also supports `--score-channel semi_dense`, which builds a
reference-side IGM mask and scores transformed current intensity only on that
mask. On seq02 12-frame smoke this underperformed the dense intensity score:

| Score channel | Feature quantile | Frames | ATE xy [m] | Step RMSE [m] | Runtime [s] | Accepted / Rejected |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| intensity | n/a | 12 | 0.166 | 0.028 | 0.75 | 11 / 0 |
| semi_dense | 0.90 | 12 | 2.572 | 0.428 | 2.30 | 1 / 10 |
| semi_dense | 0.75 | 12 | 1.737 | 0.472 | 2.22 | 4 / 7 |

Interpretation:

- IGM is not useful as a naive grid-search score in this implementation.
- The sparse mask is too brittle under coarse candidate transforms.
- Keep IGM for the next CUBE-LIO step: local photometric optimization over a
  semi-dense residual mask, not brute-force candidate ranking.

Local photometric optimization smoke:

Added `--local-opt-iters` to the cubemap demo. It applies a small
coordinate-descent refinement after grid search and uses a regularization term
against the grid-search candidate to avoid pure NCC drift.

```bash
python3 evaluation/scripts/intensity_cubemap_odometry_demo.py \
  --local-opt-iters 3 \
  --sequence-pcd-dir dogfooding_results/kitti_seq_05_108 \
  --gt-csv experiments/reference_data/kitti_seq_05_108_gt.csv \
  --max-frames 30 \
  --output-json experiments/results/intensity_odometry/intensity_cubemap_localopt_reg_kitti_seq_05_30.json
```

| Sequence | Baseline cubemap ATE xy [m] | Local-opt ATE xy [m] | Baseline step RMSE [m] | Local-opt step RMSE [m] | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| 02 | 0.494 | 0.847 | 0.032 | 0.044 | Worse; small photometric drift accumulates. |
| 05 | 3.929 | 2.489 | 0.310 | 0.267 | Better; local refinement helps this scene. |
| 08 | 3.476 | 4.893 | 0.193 | 0.208 | Worse; score gain does not imply pose gain. |

Interpretation:

- Local photometric refinement is real but not reliable yet.
- The regularizer prevents the severe drift seen in the unregularized smoke,
  but sequence-dependent bias remains.
- Do not enable local-opt by default. Next acceptance criterion should compare
  score gain against motion magnitude, overlap change, and prior consistency.

Local optimization acceptance gate:

Added acceptance checks for local optimization:

- minimum score gain,
- maximum translation/yaw deviation from grid candidate,
- minimum overlap ratio.

| Sequence | Baseline cubemap ATE xy [m] | Gated local-opt ATE xy [m] | Baseline step RMSE [m] | Gated local-opt step RMSE [m] | Local opt accepted | Notes |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| 02 | 0.494 | 0.358 | 0.032 | 0.037 | 8 / 29 | Better ATE, slightly worse step length. |
| 05 | 3.929 | 2.125 | 0.310 | 0.251 | 6 / 29 | Better on both metrics. |
| 08 | 3.476 | 6.412 | 0.193 | 0.233 | 9 / 29 | Worse; score/overlap gate still accepts biased updates. |

Interpretation:

- The gate is a useful improvement over unconditional local opt.
- It is still not robust enough: seq08 shows that photometric score and overlap
  consistency alone do not calibrate pose error.
- Next gate should include a kinematic consistency model or compare against a
  geometry-backed prior, not only image-space agreement.

External BEV prior for cubemap refinement:

Added `--pair-prior-json` so the cubemap demo can center each pair search on
motions estimated by another front-end. First test uses the BEV fast gated
odometry outputs as external priors, then applies cubemap local optimization.

```bash
python3 evaluation/scripts/intensity_cubemap_odometry_demo.py \
  --local-opt-iters 3 \
  --pair-prior-json experiments/results/intensity_odometry/intensity_bev_fast_gated_kitti_seq_08_30.json \
  --sequence-pcd-dir dogfooding_results/kitti_seq_08_108 \
  --gt-csv experiments/reference_data/kitti_seq_08_108_gt.csv \
  --max-frames 30 \
  --output-json experiments/results/intensity_odometry/intensity_cubemap_bevprior_localopt_kitti_seq_08_30.json
```

| Sequence | BEV gated ATE xy [m] | Cubemap + BEV prior ATE xy [m] | BEV step RMSE [m] | Cubemap + BEV prior step RMSE [m] | Local opt accepted |
| --- | ---: | ---: | ---: | ---: | ---: |
| 02 | 0.494 | 0.537 | 0.032 | 0.032 | 11 / 29 |
| 05 | 1.538 | 3.550 | 0.176 | 0.283 | 9 / 29 |
| 08 | 3.135 | 3.818 | 0.187 | 0.196 | 5 / 29 |

Interpretation:

- BEV prior prevents the worst seq08 cubemap local-opt failure, but does not
  beat BEV gated alone.
- Current cubemap photometric correction is not yet trustworthy as a fusion
  layer; it often adds bias even from a better prior.
- The next useful step is either a true geometric prior (KISS/GICP pair matcher)
  or a stricter local-opt acceptance criterion that can choose "no correction"
  more often.

Geometry ICP pair prior:

Added:

```bash
evaluation/scripts/geometry_icp_odometry_demo.py
```

This is a lightweight 2D point-to-point ICP prior using `scipy.spatial.cKDTree`.
It emits the same pair JSON shape as the intensity demos, so it can be passed
to cubemap odometry with `--pair-prior-json`.

30-frame KITTI smoke with a 0.3 m motion sanity gate:

| Sequence | Geometry ICP ATE xy [m] | Step RMSE [m] | Notes |
| --- | ---: | ---: | --- |
| 02 | 1.939 | 0.113 | Over-estimates small pair motion; many updates rejected/held. |
| 05 | 3.372 | 0.167 | Better step length than raw cubemap, worse ATE than BEV gated. |
| 08 | 5.671 | 0.097 | Smooth but globally biased. |

Cubemap local opt with geometry ICP prior:

| Sequence | Geometry ICP ATE xy [m] | Cubemap + geometry prior ATE xy [m] | Cubemap + geometry prior step RMSE [m] |
| --- | ---: | ---: | ---: |
| 02 | 1.939 | 1.263 | 0.167 |
| 05 | 3.372 | 6.607 | 0.459 |
| 08 | 5.671 | 6.060 | 0.222 |

Interpretation:

- Naive scan-to-scan point ICP is not a trustworthy prior on these KITTI short
  windows.
- It often estimates smooth but biased forward motion, especially when the true
  sampled-frame displacement is small.
- A real KISS/GICP prior should use stronger rejection and possibly point-to-plane
  geometry; the simple ICP scaffold is useful mostly as a negative control.

## KISS Pair Matcher Refresh

Added a reusable `KISSMatcher` API in `papers/kiss_icp` and a
`kiss_pair_odometry` executable. This estimates a source-to-target pair
transform using the existing voxel hash map and robust point-to-point ICP, then
emits the same pair-motion JSON shape as the intensity demos.

Also fixed the visual-LiDAR demo convention:

- candidate transforms are scored as current-scan to previous-frame transforms,
- trajectory integration now composes that transform directly,
- Python GT loading now converts full 6-DoF CSV poses to first-frame-relative
  SE(2), matching the C++ smoke metric.

Command:

```bash
build/evaluation/kiss_pair_odometry \
  dogfooding_results/kitti_seq_02_108 \
  experiments/reference_data/kitti_seq_02_108_gt.csv \
  experiments/results/intensity_odometry/kitti_seq_02_kiss_pair_gate2_30.json \
  30 \
  --target-voxel-size 0.75 \
  --source-voxel-size 0.75 \
  --max-correspondence-distance 1.5 \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 6.0 \
  --max-iterations 30
```

30-frame KITTI smoke after the convention/GT fix:

| Method | Seq 02 ATE / step [m] | Seq 05 ATE / step [m] | Seq 08 ATE / step [m] | Notes |
| --- | ---: | ---: | ---: | --- |
| BEV fast gated | 24.889 / 1.449 | 10.910 / 0.659 | 9.118 / 0.499 | Mostly fails once forward motion is evaluated in the correct frame. |
| Cubemap intensity | 24.889 / 1.449 | 7.451 / 0.580 | 8.794 / 0.492 | Better than BEV on 05/08, still drift-level. |
| KISS pair matcher | 0.426 / 0.040 | 0.765 / 0.055 | 0.878 / 0.082 | Strong geometry prior on all three short windows. |
| Cubemap + KISS search | 22.169 / 1.333 | 5.621 / 0.364 | 5.846 / 0.392 | Free photometric search corrupts a good geometry prior. |
| Cubemap + KISS locked | 0.425 / 0.040 | 0.765 / 0.055 | 0.877 / 0.082 | Confirms pair-prior plumbing; no photometric correction applied. |

Interpretation:

- The previous image-only BEV/cubemap numbers were optimistic because GT and
  transform convention were under-specified.
- A real KISS-style geometry prior is immediately useful and beats the image
  scaffolds by a large margin on seq02/05/08.
- Current cubemap photometric scoring should not be allowed to override KISS
  yet. The next useful step is a "no correction unless calibrated improvement"
  gate, then a point-to-plane/GICP prior comparison.

## KISS Prior Correction Gate

Added `--prior-correction-gate` to the cubemap demo and
`evaluation/scripts/sweep_prior_correction_gate.py` for replaying threshold
sweeps from logged prior/search candidates. The gate keeps the external pair
prior unless the photometric search candidate clears all checks:

- score gain over prior,
- translation delta from prior,
- yaw delta from prior,
- overlap ratio relative to prior.

Recommended smoke threshold from the replay sweep:

```bash
--prior-correction-gate \
--prior-correction-min-score-gain 0.03 \
--prior-correction-max-translation 0.6 \
--prior-correction-max-yaw-deg 0.5
```

30-frame KITTI results:

| Sequence | KISS pair ATE / step [m] | Strict gate ATE / step [m] | Recommended gate ATE / step [m] | Corrections accepted |
| --- | ---: | ---: | ---: | ---: |
| 02 | 0.426 / 0.040 | 0.425 / 0.040 | 0.425 / 0.040 | 0 / 29 |
| 05 | 0.765 / 0.055 | 0.765 / 0.055 | 0.414 / 0.093 | 1 / 29 |
| 08 | 0.878 / 0.082 | 0.877 / 0.082 | 0.394 / 0.121 | 3 / 29 |

Interpretation:

- The safe default is still to preserve KISS when correction evidence is weak.
- A small number of tightly gated photometric corrections can reduce ATE on
  seq05/08, but step RMSE gets worse. Treat this as a localization-smoothing
  candidate, not a clean odometry improvement yet.
- The next product-grade criterion should combine ATE/RPE with correction
  calibration: score gain alone is insufficient.

## SmallGICP Pair Prior

Added:

```bash
build/evaluation/small_gicp_pair_odometry
```

It uses the existing `SmallGICPRegistration` implementation as a frame-to-frame
pair matcher and emits the same pair-motion JSON schema as KISS, BEV, and
cubemap odometry.

Small sweep over 30-frame KITTI windows:

| Voxel / max corr | Seq 02 ATE / step [m] | Seq 05 ATE / step [m] | Seq 08 ATE / step [m] | Mean ATE [m] |
| --- | ---: | ---: | ---: | ---: |
| 0.75 / 512 | 0.565 / 0.062 | 0.190 / 0.017 | 0.531 / 0.067 | 0.429 |
| 0.75 / 1024 | 0.707 / 0.057 | 0.154 / 0.012 | 0.573 / 0.065 | 0.478 |
| 1.00 / 512 | 0.828 / 0.103 | 0.501 / 0.036 | 1.204 / 0.156 | 0.844 |
| 1.00 / 1024 | 0.723 / 0.084 | 0.419 / 0.028 | 0.733 / 0.073 | 0.625 |
| 1.25 / 512 | 0.240 / 0.040 | 0.313 / 0.049 | 1.089 / 0.127 | 0.547 |
| 1.25 / 1024 | 0.239 / 0.032 | 0.233 / 0.050 | 0.789 / 0.069 | 0.420 |

Against the current KISS pair baseline:

| Sequence | KISS ATE / step [m] | SmallGICP 0.75/512 ATE / step [m] | Winner |
| --- | ---: | ---: | --- |
| 02 | 0.426 / 0.040 | 0.565 / 0.062 | KISS |
| 05 | 0.765 / 0.055 | 0.190 / 0.017 | SmallGICP |
| 08 | 0.878 / 0.082 | 0.531 / 0.067 | SmallGICP |

Cubemap correction gate with SmallGICP 0.75/512 prior:

| Sequence | SmallGICP prior ATE / step [m] | SmallGICP + cubemap gate ATE / step [m] | Corrections accepted |
| --- | ---: | ---: | ---: |
| 02 | 0.565 / 0.062 | 0.591 / 0.060 | 1 / 29 |
| 05 | 0.190 / 0.017 | 0.658 / 0.134 | 2 / 29 |
| 08 | 0.531 / 0.067 | 0.419 / 0.169 | 4 / 29 |

Interpretation:

- SmallGICP is a better geometry prior than KISS on seq05/08 and has much lower
  step error there.
- The best pair prior is sequence-dependent: KISS is still better on seq02.
- Photometric cubemap corrections should not be applied uniformly to a strong
  geometry prior. They can reduce ATE on some windows but tend to damage step
  consistency, especially when the prior is already good.
- Next useful experiment is a pair-prior selector / ensemble: compare KISS and
  SmallGICP per pair using runtime, correspondence count, RMSE/fitness, and
  cubemap prior score, then choose one before any photometric correction.

## Pair Prior Selector

Added:

```bash
evaluation/scripts/select_pair_prior.py
```

It loads multiple pair-prior JSONs, scores each candidate transform against the
previous-frame cubemap, and emits a selected pair-prior JSON with the same schema
as the geometry priors. Tested policies:

- `cubemap_score`: choose the candidate with the highest cubemap image score,
- `geometry_score`: choose by geometry confidence proxy,
- `hybrid_zscore`: combine cubemap and geometry ranks,
- `preferred_with_cubemap_margin`: prefer one method unless another wins
  cubemap score by a margin.

For KISS + SmallGICP, the most useful smoke policy was:

```bash
--policy preferred_with_cubemap_margin \
--preferred-name small_gicp \
--score-margin 0.01
```

30-frame KITTI results:

| Method | Seq 02 ATE / step [m] | Seq 05 ATE / step [m] | Seq 08 ATE / step [m] |
| --- | ---: | ---: | ---: |
| KISS pair | 0.426 / 0.040 | 0.765 / 0.055 | 0.878 / 0.082 |
| SmallGICP 0.75/512 | 0.565 / 0.062 | 0.190 / 0.017 | 0.531 / 0.067 |
| Cubemap-score selector | 0.332 / 0.045 | 0.460 / 0.041 | 0.576 / 0.067 |
| SmallGICP-preferred margin selector | 0.353 / 0.054 | 0.378 / 0.033 | 0.560 / 0.065 |
| Selector + cubemap correction gate | 0.353 / 0.054 | 0.135 / 0.100 | 0.528 / 0.132 |

Interpretation:

- Cubemap score can be useful for choosing between geometry priors; it improves
  seq02 beyond either individual prior.
- A simple selector still does not dominate per-sequence best geometry. On
  seq05/08, SmallGICP alone remains the clean odometry winner by step RMSE.
- Cubemap correction after selection can reduce ATE further on seq05, but again
  damages step consistency. Keep it as a localization correction experiment, not
  as the odometry default.
- Next step should add an oracle/diagnostic selector report: per-pair error,
  selected method, cubemap score margin, geometry score, and whether the chosen
  method was actually better. That will show what signal is missing for a
  product-grade selector.

## Selector Diagnostics

Added:

```bash
evaluation/scripts/diagnose_pair_prior_selector.py
```

It compares each candidate pair motion against GT relative motion, reports
which method the selector chose, which method was oracle-best per pair, and how
often each available signal would choose the oracle-best candidate.

Diagnostic outputs:

- `selector_diagnostics_kitti_seq_02_30.{json,md}`
- `selector_diagnostics_kitti_seq_05_30.{json,md}`
- `selector_diagnostics_kitti_seq_08_30.{json,md}`

30-frame diagnostics:

| Sequence | Selector hit rate | Cubemap-score hit rate | Geometry-score hit rate | Oracle counts | Oracle ATE / step [m] |
| --- | ---: | ---: | ---: | --- | ---: |
| 02 | 16 / 29 | 21 / 29 | 19 / 29 | KISS 19, SmallGICP 10 | 0.325 / 0.031 |
| 05 | 20 / 29 | 16 / 29 | 4 / 29 | SmallGICP 25, KISS 4 | 0.188 / 0.015 |
| 08 | 18 / 29 | 17 / 29 | 10 / 29 | SmallGICP 19, KISS 10 | 0.396 / 0.048 |

Interpretation:

- There is meaningful oracle headroom on seq02/08, but not much on seq05
  because SmallGICP already matches the oracle trajectory closely.
- Cubemap score is the strongest single signal on seq02 and still useful on
  seq08, but it under-selects SmallGICP on seq05.
- The geometry confidence proxy from current JSON is misleading because KISS
  score/overlap is not comparable with SmallGICP fitness/overlap. Future
  selector features need calibrated quality metrics, not raw method-local
  scores.
- Product direction: use SmallGICP as the default pair prior, allow KISS only
  when cubemap score advantage is clear and geometry residuals are calibrated.

## Pair Quality Calibration

Added:

```bash
evaluation/scripts/calibrate_pair_prior_quality.py
```

It trains a small ridge ranker to predict negative GT pair translation error
from selector diagnostics and evaluates it with leave-one-sequence-out. Feature
sets tested:

- `all`: cubemap score, method-local geometry score, overlap, motion magnitude,
  yaw magnitude, method one-hot,
- `no_method`: same but no method one-hot,
- `cubemap_only`,
- `motion_cubemap`: cubemap score + motion magnitude + yaw magnitude.

Leave-one-sequence-out results:

| Feature set | Held-out seq02 ATE / step [m] | Held-out seq05 ATE / step [m] | Held-out seq08 ATE / step [m] |
| --- | ---: | ---: | ---: |
| all | 0.504 / 0.057 | 0.765 / 0.055 | 0.877 / 0.082 |
| no_method | 0.504 / 0.057 | 0.520 / 0.041 | 0.638 / 0.072 |
| cubemap_only | 0.351 / 0.059 | 0.496 / 0.040 | 0.817 / 0.082 |
| motion_cubemap | 0.397 / 0.052 | 0.477 / 0.039 | 0.648 / 0.074 |

Interpretation:

- Method one-hot overfits badly with only three short windows. It learns
  sequence-specific method bias instead of transferable quality.
- Removing method identity improves seq05/08 generalization, but still does not
  beat the best simple prior policy.
- Cubemap score alone generalizes enough to help seq02, but cannot recognize
  that SmallGICP should dominate seq05.
- Current diagnostics support a conservative product rule: SmallGICP default,
  KISS fallback only with a strong cubemap advantage and later calibrated
  residual metrics.
- The missing feature is a comparable geometry residual. KISS RMSE and SmallGICP
  fitness are method-local scales; selector calibration needs a shared residual
  computed after applying each candidate to the same scan pair.

## Shared Geometry Residual

Added:

```bash
evaluation/scripts/evaluate_pair_prior_residuals.py
```

It applies each candidate pair transform to the current scan, queries the same
previous-scan KD-tree, and reports shared RMSE, median distance, P95 distance,
inlier ratio, and a common residual score. It also evaluates the inverse
transform and reports bidirectional / symmetric trimmed residuals. The latest
version estimates 2D normals with local PCA and scores candidates with a
symmetric trimmed point-to-line residual. It also evaluates small perturbations
around each candidate and reports residual stability: a good prior should sit in
a local residual basin, not just have a low residual at one pose. This gives KISS
and SmallGICP method-independent geometry and stability signals.

Shared residual signal hit rates against pair oracle:

| Sequence | Shared score | Shared RMSE | Stability score | Stability delta | Cubemap score | Oracle counts |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| 02 | 15 / 29 | 16 / 29 | 20 / 29 | 20 / 29 | 21 / 29 | KISS 19, SmallGICP 10 |
| 05 | 15 / 29 | 13 / 29 | 19 / 29 | 21 / 29 | 16 / 29 | SmallGICP 25, KISS 4 |
| 08 | 18 / 29 | 20 / 29 | 21 / 29 | 20 / 29 | 17 / 29 | SmallGICP 19, KISS 10 |

Calibration with shared residual features:

| Feature set | Held-out seq02 ATE / step [m] | Held-out seq05 ATE / step [m] | Held-out seq08 ATE / step [m] |
| --- | ---: | ---: | ---: |
| shared | 0.400 / 0.053 | 0.677 / 0.051 | 0.824 / 0.078 |
| shared_no_method | 0.390 / 0.051 | 0.349 / 0.033 | 0.499 / 0.059 |

Interpretation:

- Stability is a stronger transferable signal than raw shared residual on these
  windows. It beats cubemap score on seq05/08 and nearly matches it on seq02.
- Method one-hot still overfits. The `shared` feature set keeps learning method
  bias, while `shared_no_method` transfers across all three held-out windows.
- `shared_no_method` now beats the conservative SmallGICP-default /
  cubemap-margin selector on seq05/08 and is close on seq02, making it the first
  calibrated selector that looks potentially useful rather than just diagnostic.
- The failure mode is expected for nearest-neighbor residuals: a transform can
  produce a lower residual against repetitive/local geometry while still having
  worse GT motion.
- The explicit selector policy below turns these diagnostics into integrated
  selected-prior trajectories.

Explicit shared-stability selector policies:

| Policy | seq02 ATE / step [m] | seq05 ATE / step [m] | seq08 ATE / step [m] |
| --- | ---: | ---: | ---: |
| shared_stability_score | 0.325 / 0.041 | 0.297 / 0.028 | 0.602 / 0.062 |
| shared_stability_delta | 0.325 / 0.041 | 0.185 / 0.021 | 0.624 / 0.063 |
| shared_residual_score | 0.336 / 0.045 | 0.489 / 0.043 | 0.753 / 0.073 |
| shared_residual_rmse | 0.343 / 0.039 | 0.343 / 0.034 | 0.710 / 0.071 |
| SmallGICP default + cubemap margin | 0.353 / 0.054 | 0.378 / 0.033 | 0.560 / 0.065 |

Interpretation:

- Stability policies are materially better than raw residual policies.
- `shared_stability_delta` is the best fixed policy on seq05 and matches the
  stability score on seq02, but both stability policies still trail the
  conservative margin rule on seq08 ATE.
- This supports using stability as a selector feature, not as a single
  hard-coded replacement rule yet. The LOO `shared_no_method` ranker remains
  the better direction for a transferable policy.

## Calibrated Selector Policy

Added `--policy calibrated_quality` to:

```bash
evaluation/scripts/select_pair_prior.py
```

The selector reads a calibration JSON from
`calibrate_pair_prior_quality.py`, rebuilds the calibrated feature vector for
each pair candidate, applies the stored mean / scale / weights, and selects the
highest predicted pair quality. This makes the `shared_no_method` ranker usable
as an actual selected-prior trajectory generator instead of only a diagnostic
offline evaluation.

Integrated selector trajectories:

| Policy | seq02 ATE / step [m] | seq05 ATE / step [m] | seq08 ATE / step [m] |
| --- | ---: | ---: | ---: |
| calibrated_quality full model | 0.368 / 0.047 | 0.239 / 0.024 | 0.499 / 0.059 |
| calibrated_quality held-out model | 0.390 / 0.051 | 0.349 / 0.033 | 0.479 / 0.056 |
| shared_stability_delta | 0.325 / 0.041 | 0.185 / 0.021 | 0.624 / 0.063 |
| SmallGICP default + cubemap margin | 0.353 / 0.054 | 0.378 / 0.033 | 0.560 / 0.065 |

Interpretation:

- The calibrated selector is now the best transferable policy on seq08 and
  competitive on seq05.
- The full model is stronger on seq05/08 but is not a clean generalization
  estimate because it has seen all three short windows.
- The held-out model is the conservative estimate: it improves seq05/08 over
  the hand-written margin rule, but seq02 still prefers the simple stability
  delta / KISS-heavy choice.
- Next useful step is not more scoring features. It is a train/validate split
  over more windows so the calibrated selector can be tuned without reusing the
  same three 30-frame clips.

## Window-Level Selector Benchmark

Added:

```bash
evaluation/scripts/run_pair_prior_window_benchmark.py
```

It creates 30-frame KITTI windows from full-sequence PCD/GT inputs using
symlinked PCD directories and sliced GT CSVs, then runs KISS pair odometry,
SmallGICP pair odometry, shared residuals, selector diagnostics, window-level
LOO calibration, and integrated selector policies.

Command:

```bash
python3 evaluation/scripts/run_pair_prior_window_benchmark.py \
  --output-dir experiments/results/intensity_odometry/window_benchmark
```

12-window aggregate over seq02/05/08 starts 0/30/60/90:

| Policy | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] | Window wins |
| --- | ---: | ---: | ---: | ---: | ---: |
| KISS | 0.630 | 0.566 | 0.070 | 1.111 | 0 |
| SmallGICP | 0.506 | 0.463 | 0.063 | 1.270 | 3 |
| SmallGICP default + cubemap margin | 0.518 | 0.406 | 0.066 | 1.092 | 2 |
| shared_stability_delta | 0.498 | 0.481 | 0.059 | 0.842 | 2 |
| Stability margin 0.001 | 0.474 | 0.454 | 0.058 | 0.770 | 1 |
| Stability margin 0.0025 | 0.453 | 0.409 | 0.059 | 0.907 | 2 |
| Stability motion 0.0025 | 0.415 | 0.367 | 0.059 | 0.690 | 1 |
| calibrated_quality full model | 0.511 | 0.459 | 0.062 | 0.786 | 1 |
| calibrated_quality held-out model | 0.562 | 0.467 | 0.065 | 1.166 | 0 |

Full table:

```text
experiments/results/intensity_odometry/window_benchmark/window_benchmark_summary.md
```

Interpretation:

- The 3-window result was too optimistic for calibrated selection. With 12
  windows, held-out `calibrated_quality` wins no windows and has worse mean ATE
  than SmallGICP, margin, and stability.
- The stability-margin selectors supersede raw `shared_stability_delta`, and
  the motion-consistency variant supersedes both. `Stability motion 0.0025`
  gives the best mean ATE and best max ATE in the 12-window benchmark.
- `calibrated_quality full model` remains useful as a diagnostic, but it is not
  a clean generalization estimate because it sees all windows during training
  and is now beaten by the motion-consistency rule on both mean and max ATE.
- Next selector work should validate the motion-consistency gate on longer
  windows and external degeneracy datasets before treating it as a production
  default.

## Stability Margin Sweep

Added:

```bash
evaluation/scripts/sweep_stability_margin_selector.py
```

It replays the 12-window benchmark with `preferred_with_stability_margin`:
SmallGICP remains the default pair prior, and KISS can override only when its
shared stability mean-delta advantage clears a threshold.

Command:

```bash
python3 evaluation/scripts/sweep_stability_margin_selector.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark \
  --reuse
```

Refined threshold sweep over seq02/05/08 starts 0/30/60/90:

| Threshold | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] | Window wins |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0.0000 | 0.498 | 0.481 | 0.059 | 0.842 | 4 |
| 0.0010 | 0.474 | 0.454 | 0.058 | 0.770 | 3 |
| 0.0015 | 0.473 | 0.445 | 0.059 | 0.794 | 1 |
| 0.0020 | 0.469 | 0.410 | 0.059 | 0.907 | 2 |
| 0.0025 | 0.453 | 0.409 | 0.059 | 0.907 | 2 |
| 0.0030 | 0.458 | 0.433 | 0.060 | 0.907 | 2 |
| 0.0040 | 0.462 | 0.426 | 0.060 | 1.031 | 5 |
| 0.0050 | 0.483 | 0.437 | 0.061 | 1.086 | 5 |
| 0.0100 | 0.510 | 0.458 | 0.062 | 1.270 | 5 |
| 0.0200 | 0.506 | 0.458 | 0.063 | 1.270 | 5 |
| 0.0500 | 0.506 | 0.458 | 0.063 | 1.270 | 5 |
| 0.1000 | 0.506 | 0.458 | 0.063 | 1.270 | 5 |

Full table:

```text
experiments/results/intensity_odometry/window_benchmark/stability_margin_sweep/stability_margin_sweep_summary.md
```

Interpretation:

- The conservative stability gate is stronger than the raw
  `shared_stability_delta` selector. Threshold `0.0010` lowers max ATE from
  0.842 m to 0.770 m and mean ATE from 0.498 m to 0.474 m.
- Threshold `0.0025` gives the best mean ATE at 0.453 m, but its worst window is
  0.907 m. For a product-style default, `0.0010` is the safer setting because it
  improves the tail while still improving mean error.
- Larger thresholds collapse back toward SmallGICP-default behavior and bring
  back the seq08 failure mode, visible as max ATE rising to 1.270 m.
- This is the first selector rule in this track that improves both aggregate
  accuracy and worst-case behavior without using a trained model.

## Tail Failure Analysis / Motion Consistency Gate

Added:

```bash
evaluation/scripts/analyze_pair_prior_tail_failures.py
```

The report compares selector policies on the tail windows that dominated
worst-case error after the stability-margin sweep:

```text
experiments/results/intensity_odometry/window_benchmark/tail_failure_analysis/tail_failure_analysis.md
```

Tail finding:

- `kitti_seq_08_start0030_30` is a low-motion window. SmallGICP over-selects
  nonzero motion on near-stationary pairs; KISS is often the oracle winner.
- `kitti_seq_08_start0090_30` is a normal-motion window. SmallGICP should remain
  the default, and KISS overrides are usually harmful.

Added `--policy preferred_with_stability_motion_consistency` to
`select_pair_prior.py`. It first applies the stability-margin rule, then allows
the alternate candidate to override when it is materially closer to the previous
selected pair motion. Current benchmark setting:

```bash
--policy preferred_with_stability_motion_consistency \
--preferred-name small_gicp \
--score-margin 0.0025 \
--motion-consistency-margin 0.04 \
--motion-consistency-min-prev-step 0.0 \
--motion-consistency-max-prev-step 0.30
```

12-window aggregate:

| Policy | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] |
| --- | ---: | ---: | ---: | ---: |
| Stability margin 0.001 | 0.474 | 0.454 | 0.058 | 0.770 |
| Stability margin 0.0025 | 0.453 | 0.409 | 0.059 | 0.907 |
| Stability motion 0.0025 | 0.415 | 0.367 | 0.059 | 0.690 |

Interpretation:

- The motion gate fixes the seq08 start0030 tail without hurting seq08
  start0090, because it is active only when previous selected motion is small.
- It is now the best non-trained selector in this track, but it is still a
  heuristic. Treat it as the next product-candidate selector to validate, not as
  a final localization component.

## 60-Frame Window Validation

Ran the same benchmark contract on 60-frame windows:

```bash
python3 evaluation/scripts/run_pair_prior_window_benchmark.py \
  --window-size 60 \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/window_benchmark_summary.md
```

12-window aggregate over seq02/05/08 starts 0/30/60/90:

| Policy | Mean ATE [m] | Median ATE [m] | Mean step RMSE [m] | Max ATE [m] | Window wins |
| --- | ---: | ---: | ---: | ---: | ---: |
| KISS | 1.071 | 1.097 | 0.073 | 2.246 | 0 |
| SmallGICP | 0.997 | 0.976 | 0.063 | 2.176 | 2 |
| Cubemap margin | 0.976 | 0.960 | 0.067 | 1.951 | 1 |
| shared_stability_delta | 0.879 | 0.932 | 0.061 | 1.789 | 2 |
| Stability margin 0.001 | 0.835 | 0.798 | 0.060 | 1.644 | 1 |
| Stability margin 0.0025 | 0.862 | 0.776 | 0.060 | 1.799 | 3 |
| Stability motion 0.0025 | 0.800 | 0.776 | 0.060 | 1.582 | 1 |
| calibrated_quality full model | 0.890 | 0.795 | 0.063 | 1.808 | 2 |
| calibrated_quality held-out model | 0.939 | 0.870 | 0.065 | 1.999 | 0 |

Tail report:

```text
experiments/results/intensity_odometry/window_benchmark_60/tail_failure_analysis/tail_failure_analysis.md
```

Interpretation:

- The motion-consistency selector generalizes beyond the 30-frame benchmark:
  `Stability motion 0.0025` is still the best aggregate policy on mean ATE,
  mean step RMSE, and max ATE.
- The 60-frame result is not production-grade yet. Max ATE is still 1.582 m,
  dominated by seq08 windows where pure pairwise odometry accumulates drift.
- Tail behavior differs by motion regime. seq08 start0030 still benefits from
  choosing KISS on low-motion pairs, while seq08 start0090 prefers almost pure
  SmallGICP. The motion gate mostly preserves that distinction.
- The next meaningful step is no longer selector feature churn. Add a
  trajectory-level correction layer: keyframe/submap registration, loop-like
  local smoothing, or map-based correction using the selected pair prior as the
  motion model.

## Temporal Smoother Check

Added a GT-free temporal smoother over the selected pair motions:

```bash
python3 evaluation/scripts/run_pair_prior_temporal_smoother_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/temporal_smoother/temporal_smoother_summary.md
```

Best 60-frame setting was `lambda_translation=0.25`, `lambda_yaw=0.0`.
It barely moved the result:

| Method | Mean ATE [m] | Median ATE [m] | Max ATE [m] |
| --- | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 |
| Temporal smoother best | 0.800 | 0.775 | 1.581 |

Interpretation: smoothing pair motions is not enough. The remaining error is
real registration drift, not just noisy pair increments.

## KISS Keyframe Anchor Correction

Added a `kiss_keyframe_odometry` executable that keeps the normal KISS
frame-to-frame odometry but every 10 frames directly registers the current scan
against the previous keyframe scan. The keyframe registration is initialized
from the integrated odometry and accepted only under correction / RMSE gates.

Build and 60-frame benchmark:

```bash
cmake --build build --target kiss_keyframe_odometry -j2
python3 evaluation/scripts/run_kiss_keyframe_window_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/kiss_keyframe_i10/kiss_keyframe_summary.md
```

12-window aggregate:

| Method | Mean ATE [m] | Median ATE [m] | Max ATE [m] | Mean step RMSE [m] |
| --- | ---: | ---: | ---: | ---: |
| KISS pair | 1.071 | 1.097 | 2.246 | - |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - |
| KISS keyframe i10 | 0.731 | 0.403 | 1.537 | 0.121 |

Key observations:

- This is the first trajectory-level correction that beats the best pairwise
  selector on 60-frame mean ATE and max ATE.
- It improves the worst seq08 start0090 window from 1.582 m to 1.304 m and
  gives large wins on seq02 start0000/0030/0060.
- It is not production-ready: only 6/12 windows improve over the stability
  selector, seq02 start0090 and seq08 start0030 regress, and step RMSE worsens.
- Tightening `max_keyframe_correction` from 1.0 m to 0.3 m was too conservative
  and regressed aggregate mean ATE to 0.888 m.

Correction-gate sweep:

```bash
python3 evaluation/scripts/run_kiss_keyframe_window_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/kiss_keyframe_i10_c0p5 \
  --max-keyframe-correction 0.5

python3 evaluation/scripts/run_kiss_keyframe_window_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/kiss_keyframe_i10_c0p7 \
  --max-keyframe-correction 0.7

python3 evaluation/scripts/summarize_kiss_keyframe_gate_sweep.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/kiss_keyframe_gate_sweep_summary.md
```

| Method / gate | Mean ATE [m] | Median ATE [m] | Max ATE [m] | Mean step RMSE [m] | Improved vs stability |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | 0.060 | - |
| Keyframe correction <=0.3 m | 0.888 | 0.784 | 1.780 | 0.079 | 5/12 |
| Keyframe correction <=0.5 m | 0.784 | 0.749 | 1.295 | 0.089 | 8/12 |
| Keyframe correction <=0.7 m | 0.720 | 0.520 | 1.537 | 0.109 | 7/12 |
| Keyframe correction <=1.0 m | 0.731 | 0.403 | 1.537 | 0.121 | 6/12 |

Interpretation:

- `0.7 m` is the best mean-ATE gate.
- `0.5 m` is the better product-risk gate: it has the best max ATE, improves
  8/12 windows over the stability selector, and keeps step RMSE lower than the
  0.7/1.0 m gates.
- The keyframe correction path is now materially useful, but still not final:
  it needs a learned or calibrated quality gate before being promoted over the
  pairwise stability selector.

Next gate to add: score/consistency calibration for keyframe corrections before
applying them, likely using correction magnitude, RMSE, correspondence count,
and agreement with the stability-motion prior.

## Stability Prior + Keyframe Correction Replay

The previous keyframe executable used KISS pair odometry as its local motion
model. That is not the best product candidate, because the 60-frame pairwise
benchmark already showed `Stability motion 0.0025` is a better local prior.

Updated `kiss_keyframe_odometry` to log the actual keyframe correction
transform:

- `keyframe_correction_dx_m`
- `keyframe_correction_dy_m`
- `keyframe_correction_transform_yaw_deg`

Then replayed accepted keyframe corrections on top of the
`Stability motion 0.0025` prior:

```bash
cmake --build build --target kiss_keyframe_odometry -j2

python3 evaluation/scripts/run_kiss_keyframe_window_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/kiss_keyframe_i10_c0p5_relog \
  --max-keyframe-correction 0.5

python3 evaluation/scripts/run_keyframe_corrected_prior_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --anchor-dir-name kiss_keyframe_i10_c0p5_relog \
  --max-keyframe-correction 0.5
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/stability_motion_keyframe_c0p5/keyframe_corrected_prior_summary.md
```

12-window aggregate:

| Method | Mean ATE [m] | Median ATE [m] | Max ATE [m] | Mean step RMSE [m] | Improved windows |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | 0.060 | - |
| KISS keyframe i10, c<=0.5, KISS local | 0.784 | 0.749 | 1.295 | 0.089 | 8/12 |
| Stability motion + keyframe c<=0.5 | 0.626 | 0.517 | 1.059 | 0.084 | 9/12 |
| Stability motion + keyframe c<=0.7 | 0.668 | 0.619 | 1.437 | 0.107 | 8/12 |

Window-level notes for the c<=0.5 replay:

- seq08 start0090, the previous worst 60-frame tail, improves from 1.582 m to
  0.628 m.
- seq08 start0060 improves from 1.309 m to 1.059 m.
- seq02 windows mostly improve, but seq02 start0060 remains near 0.96 m.
- seq05 start0000/start0030/start0090 regress from already-low baselines, so
  keyframe corrections still need a quality gate or mode switch for easy
  low-drift windows.

Added a correspondence-count gate sweep because the seq05 false-positive
corrections had much lower keyframe correspondence counts than most useful seq08
corrections:

```bash
python3 evaluation/scripts/run_keyframe_corrected_prior_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --anchor-dir-name kiss_keyframe_i10_c0p5_relog \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/stability_motion_keyframe_c0p5_corr6000 \
  --max-keyframe-correction 0.5 \
  --min-keyframe-correspondences 6000

python3 evaluation/scripts/summarize_keyframe_replay_gate_sweep.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60
```

Full table:

```text
experiments/results/intensity_odometry/window_benchmark_60/keyframe_replay_gate_sweep_summary.md
```

| Replay gate | Mean ATE [m] | Median ATE [m] | Max ATE [m] | Mean step RMSE [m] | Improved | Regressions |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | 0.060 | - | - |
| c<=0.5, corr>=80 | 0.626 | 0.517 | 1.059 | 0.084 | 9/12 | 3/12 |
| c<=0.5, corr>=5000 | 0.620 | 0.528 | 1.059 | 0.077 | 8/12 | 2/12 |
| c<=0.5, corr>=6000 | 0.605 | 0.528 | 1.059 | 0.071 | 7/12 | 0/12 |
| c<=0.5, corr>=7000 | 0.639 | 0.558 | 1.119 | 0.070 | 6/12 | 1/12 |
| c<=0.7, corr>=80 | 0.668 | 0.615 | 1.437 | 0.107 | 8/12 | 4/12 |

Updated interpretation:

- Best current product-risk recipe is `Stability motion + KISS keyframe replay`
  with `max_keyframe_correction=0.5 m` and
  `min_keyframe_correspondences=6000`.
- It improves the 60-frame mean ATE from 0.800 m to 0.605 m and max ATE from
  1.582 m to 1.059 m with no windows regressing above the stability-motion
  baseline.
- This is now the leading recipe in the intensity / pair-prior track.

## Keyframe Correction Component Contract

The keyframe replay path has been factored into a reusable component:

```text
evaluation/scripts/keyframe_correction_component.py
evaluation/scripts/KEYFRAME_CORRECTION_COMPONENT.md
```

Pinned recipe:

```text
experiments/results/intensity_odometry/recipes/stability_motion_kiss_keyframe_corr6000.json
```

Recipe-driven replay reproduces the corr>=6000 result exactly:

```bash
python3 evaluation/scripts/run_keyframe_corrected_prior_benchmark.py \
  --benchmark-dir experiments/results/intensity_odometry/window_benchmark_60 \
  --anchor-dir-name kiss_keyframe_i10_c0p5_relog \
  --output-dir experiments/results/intensity_odometry/window_benchmark_60/stability_motion_keyframe_component_recipe \
  --recipe-json experiments/results/intensity_odometry/recipes/stability_motion_kiss_keyframe_corr6000.json
```

Component output:

```text
experiments/results/intensity_odometry/window_benchmark_60/stability_motion_keyframe_component_recipe/keyframe_corrected_prior_summary.md
```

The component contract is:

- Local prior input: selected pair-prior JSON.
- Correction input: KISS keyframe JSON with correction transform fields.
- Gate output: accepted/rejected correction with reason.
- Trajectory output: corrected poses plus correction diagnostics.
- GT usage: metrics only; the gate itself is GT-free.

The C++ `kiss_keyframe_odometry` runtime now emits the same decision contract:

- `keyframe_anchor_accepted`: registration candidate validity.
- `keyframe_accepted`: final gate decision.
- `keyframe_decision_reason`: one of the component gate reasons.
- `min_keyframe_correspondences`: runtime gate parameter, default `6000`.

Interpretation:

- The right architecture is now clearer: use the stability-motion pair selector
  as the local motion model, and use KISS keyframe registration as a bounded
  correction source.
- This is the strongest 60-frame result in the intensity / pair-prior track so
  far: mean ATE 0.605 m and max ATE 1.059 m.
- Remaining product gap is generalization: validate this correction gate on
  longer windows / held-out starts, then stress it on degeneracy datasets where
  high correspondence count may still represent a false lock.

## Degeneracy Dataset Candidate

`ntnu-arl/lidar_degeneracy_datasets` is a strong stress-test candidate for the
product-grade benchmark track. The repository describes LiDAR, IMU, and FMCW
radar measurements from an aerial robot in LiDAR-challenging environments:

- a long straight biking tunnel with geometric self-similarity,
- a fog-filled university corridor with obscurants,
- sensor extrinsics with respect to the IMU.

Use it for degeneracy / false-lock / failure-detection experiments rather than
as a first-pass algorithm tuning dataset.
