# LiDAR Degeneracy Dataset Benchmark Setup

Source: <https://github.com/ntnu-arl/lidar_degeneracy_datasets>

The dataset is useful for the product-grade robustness track rather than for
first-pass tuning. It targets LiDAR-degenerative environments with an aerial
robot carrying LiDAR, IMU, and FMCW radar.

## Why It Fits This Repo

- Long straight tunnel: geometric self-similarity and weak lateral constraints.
- Fog-filled corridor: obscurants and degraded LiDAR returns.
- Sensor stack: Ouster OS0-128 LiDAR, VectorNav VN100 IMU, TI IWR6843AOP radar.
- The README provides CAD extrinsics with respect to the IMU.

Use it to test:

- degeneracy detection,
- false-lock rate,
- confidence/error calibration,
- outlier rejection under obscurants,
- fallback behavior for LiDAR-only vs LiDAR-inertial vs radar-inertial variants.

## Data Conversion Plan

The current upstream bags are published on Hugging Face:

- `tunnel.bag` (~3.47 GB)
- `fog.bag` (~1.85 GB)

The upstream README lists Ouster packet topics, not a guaranteed ready-made
`sensor_msgs/PointCloud2` topic. Treat conversion as a two-step process:

1. inspect the bag and write a manifest,
2. extract directly only if a `PointCloud2` topic is present; otherwise replay
   `/os_cloud_node/lidar_packets` through the Ouster driver first.

### 1. Download + inspect

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --download \
  --inspect \
  --download-tool parallel \
  --download-connections 16
```

This writes:

- `data/lidar_degeneracy_datasets/tunnel.bag`
- `data/lidar_degeneracy_datasets/fog.bag`
- `data/lidar_degeneracy_datasets/tunnel_topics.json`
- `data/lidar_degeneracy_datasets/fog_topics.json`
- `data/lidar_degeneracy_datasets/*_manifest.json`

For a metadata-only dry run before downloading:

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --manifest-only
```

### 2. Extract when PointCloud2 is available

If inspection reports a LiDAR PointCloud2 topic:

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --sequence tunnel \
  --inspect \
  --extract \
  --pointcloud-topic /os_cloud_node/points \
  --imu-topic /vectornav_node/uncomp_imu \
  --max-frames 200 \
  --time-mode azimuth
```

This writes `dogfooding_results/lidar_degeneracy_tunnel_200/` with
`00000000/cloud.pcd`, `frame_timestamps.csv`, and `imu.csv`.

### 3. Packet-topic fallback

If inspection reports only packet topics such as:

- `/os_cloud_node/lidar_packets`
- `/os_cloud_node/imu_packets`
- `/os_cloud_node/metadata`

the fastest path is to decode the packet bag directly with `ouster-sdk`:

```bash
python3 -m pip install ouster-sdk

python3 evaluation/scripts/extract_ouster_packet_lidar_imu.py \
  --bag data/lidar_degeneracy_datasets/fog.bag \
  --output-dir dogfooding_results/lidar_degeneracy_fog_200 \
  --max-frames 200 \
  --skip-incomplete

python3 evaluation/scripts/analyze_lidar_degeneracy_sequence.py \
  dogfooding_results/lidar_degeneracy_fog_200 \
  experiments/results/lidar_degeneracy/fog_200 \
  --max-frames 200 \
  --sample-points 20000

python3 evaluation/scripts/select_lidar_degradation_windows.py \
  experiments/results/lidar_degeneracy/fog_200/frame_degeneracy.csv \
  experiments/results/lidar_degeneracy/fog_200/window_selection \
  --window-size 30 \
  --stride 5

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/fog_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/fog_200/odometry_health \
  --geometry-min-correspondences 40
```

Run the same selected windows through the KISS keyframe backend after building
`kiss_keyframe_odometry`:

```bash
cmake --build build --target kiss_pair_odometry kiss_keyframe_odometry -j2

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/fog_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/fog_200/kiss_keyframe_health \
  --method kiss_keyframe \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20 \
  --kiss-min-correspondences 80 \
  --min-keyframe-correspondences 1000

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/kiss_keyframe_health \
  --method kiss_keyframe \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20 \
  --kiss-min-correspondences 80 \
  --min-keyframe-correspondences 1000
```

For CT-ICP, use the dedicated window runner. It reports the internal CT-ICP
convergence bit separately from the accepted gate, because the dogfooding
pipeline has historically used the refined transform even when the internal
stopping threshold did not fire:

```bash
cmake --build build --target ct_icp_window_odometry -j2

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/fog_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/fog_200/ct_icp_health \
  --method ct_icp \
  --ct-icp-refinement-gate \
  --ct-icp-multi-scale \
  --ct-icp-normal-cholesky \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/ct_icp_health \
  --method ct_icp \
  --ct-icp-refinement-gate \
  --ct-icp-multi-scale \
  --ct-icp-normal-cholesky \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/fog_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/fog_200/intensity_bev_health \
  --method intensity_bev \
  --intensity-bev-preset default \
  --intensity-bev-min-step-translation 0.25 \
  --intensity-bev-zero-motion-score-margin 0.05 \
  --intensity-bev-low-motion-score-margin 0.02 \
  --intensity-bev-low-motion-stress-only \
  --min-used-path-length 0.25 \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20

python3 evaluation/scripts/run_lidar_degradation_health.py \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/window_selection/degradation_windows.json \
  experiments/results/lidar_degeneracy/tunnel_geom_2700_200/intensity_bev_health \
  --method intensity_bev \
  --intensity-bev-preset default \
  --intensity-bev-min-step-translation 0.25 \
  --intensity-bev-zero-motion-score-margin 0.05 \
  --intensity-bev-low-motion-score-margin 0.02 \
  --intensity-bev-low-motion-stress-only \
  --min-used-path-length 0.25 \
  --max-step-translation 2.0 \
  --max-step-yaw-deg 20

python3 evaluation/scripts/summarize_lidar_degeneracy_health.py
```

`extract_ouster_packet_lidar_imu.py` uses `/sensor_sync_node/trigger_1` by
default for LiDAR frame timestamps, because the packet bag time and IMU header
time are offset. This keeps `frame_timestamps.csv` on the same clock as
`/vectornav_node/uncomp_imu`.

The same direct extraction path applies to `tunnel.bag`:

```bash
python3 evaluation/scripts/extract_ouster_packet_lidar_imu.py \
  --bag data/lidar_degeneracy_datasets/tunnel.bag \
  --output-dir dogfooding_results/lidar_degeneracy_tunnel_200 \
  --max-frames 200 \
  --skip-incomplete

python3 evaluation/scripts/analyze_lidar_degeneracy_sequence.py \
  dogfooding_results/lidar_degeneracy_tunnel_200 \
  experiments/results/lidar_degeneracy/tunnel_200 \
  --max-frames 200 \
  --sample-points 20000
```

For full-bag tunnel scouting without exporting every scan, use `--frame-stride`:

```bash
python3 evaluation/scripts/extract_ouster_packet_lidar_imu.py \
  --bag data/lidar_degeneracy_datasets/tunnel.bag \
  --output-dir dogfooding_results/lidar_degeneracy_tunnel_stride5 \
  --max-frames -1 \
  --frame-stride 5 \
  --skip-incomplete

python3 evaluation/scripts/analyze_lidar_degeneracy_sequence.py \
  dogfooding_results/lidar_degeneracy_tunnel_stride5 \
  experiments/results/lidar_degeneracy/tunnel_stride5 \
  --sample-points 20000
```

The stride-5 scan found the strongest linearity near source scan index 2795, so
a high-density follow-up window starts at 2700:

```bash
python3 evaluation/scripts/extract_ouster_packet_lidar_imu.py \
  --bag data/lidar_degeneracy_datasets/tunnel.bag \
  --output-dir dogfooding_results/lidar_degeneracy_tunnel_geom_2700_200 \
  --start-frame 2700 \
  --max-frames 200 \
  --skip-incomplete
```

Alternatively, replay the bag through the Ouster driver referenced by the
upstream README to produce a PointCloud2 topic. Then run the existing extractor
on that converted bag:

```bash
mkdir -p ~/catkin_ouster_ws/src
cd ~/catkin_ouster_ws/src
git clone -b dev/sensor_sync_replay --recurse-submodules \
  https://github.com/ntnu-arl/ouster-ros.git
cd ~/catkin_ouster_ws
source /opt/ros/noetic/setup.bash
catkin_make --cmake-args -DCMAKE_BUILD_TYPE=Release
```

Generate and validate the replay script:

```bash
python3 evaluation/scripts/replay_ouster_packets.py \
  --sequence fog \
  --ros-setup /opt/ros/noetic/setup.bash \
  --ouster-workspace ~/catkin_ouster_ws
```

Run the conversion when ROS1 and `ouster_ros` are sourced:

```bash
python3 evaluation/scripts/replay_ouster_packets.py \
  --sequence fog \
  --ros-setup /opt/ros/noetic/setup.bash \
  --ouster-workspace ~/catkin_ouster_ws \
  --run
```

The helper records the decoded `/ouster/points` topic into
`data/lidar_degeneracy_datasets/fog_ouster_points.bag` and prints the follow-up
extract command.

```bash
python3 evaluation/scripts/extract_ros1_lidar_imu.py \
  --pointcloud-bag data/lidar_degeneracy_datasets/fog_ouster_points.bag \
  --pointcloud-topic /ouster/points \
  --imu-bag data/lidar_degeneracy_datasets/fog.bag \
  --imu-topic /vectornav_node/uncomp_imu \
  --output-dir dogfooding_results/lidar_degeneracy_fog_200 \
  --max-frames 200 \
  --time-mode azimuth
```

Topic names should be checked with `rosbag info`, the generated
`*_topics.json`, or the bag metadata.

Observed `fog.bag` inspection:

- LiDAR is packet-only: `/os_cloud_node/lidar_packets`.
- IMU is available: `/vectornav_node/uncomp_imu`.
- `/radar/cloud` is `sensor_msgs/PointCloud2`, but it is radar, not LiDAR.
- `fog_200` direct extraction produced 200 complete LiDAR frames with a mean of
  31.6k valid points/frame and `degeneracy_score` mean 0.613 / p95 0.692.
- `fog_200` degradation selection identified frames 170-199 as the strongest
  obscurant window: point count drops 42.9%, intensity drops 35.7%, and range
  p95 drops 24.9% against the initial clear-window baseline.
- A GT-free geometry ICP health smoke shows the selected degradation matters:
  baseline window accepted 65.5% of frame pairs, point-count tail accepted
  13.8%, and the strongest fog window accepted 0%.
- `tunnel_200` direct extraction produced 200 complete LiDAR frames with a mean
  of 44.2k valid points/frame. Its scattering mean is 0.095, much lower than
  `fog_200` at 0.198, which captures the tunnel's lower 3D structural richness.
  In the first 200 frames, however, the geometry ICP health smoke accepted 100%
  of frame pairs across baseline, obscurant, point-count, intensity, and
  geometry-degeneracy windows. Treat this as a negative first-window result:
  tunnel geometry is visibly structured differently, but this short slice does
  not yet break the lightweight local odometry baseline.
- `tunnel_stride5` covers the full bag at 5-frame stride and raises the
  degeneracy_score p95 to 0.755, with max 0.773 near source scan index 2795.
  The dense follow-up `tunnel_geom_2700_200` raises degeneracy_score mean/p95 to
  0.719/0.769, but the geometry ICP health smoke still accepts 100% of selected
  windows. This is a stronger negative result: local 2D ICP remains healthy on
  the sampled tunnel geometry windows, so tunnel should next be tested with
  KISS/CT-ICP or map-localization failure modes rather than this lightweight
  pairwise ICP alone.
- KISS keyframe odometry now supports GT-free window runs through `gt_csv=-` and
  `--start-frame`. On `tunnel_geom_2700_200`, the same selected windows remain
  healthy under KISS keyframes: baseline, degraded, point-count tail, and the
  geometry-degeneracy window all report 100% pair convergence/acceptance and
  2/2 accepted keyframe corrections. This keeps the tunnel result negative for
  local LiDAR-only odometry; the next tunnel value is likely in longer
  trajectories, map-localization false locks, or explicit degeneracy confidence,
  not another short-window pair health smoke.
- KISS keyframe odometry fails hard on `fog_200`: baseline, strongest fog, and
  point-count-tail windows all report 0% convergence/acceptance and 0/2 accepted
  keyframe corrections. The strongest fog window also lowers mean correspondence
  overlap from 309.0 to 163.3 against the baseline. This is a backend-specific
  degradation signal that the lightweight geometry ICP already hinted at, but it
  is sharper because the KISS matcher never reaches the convergence gate.
- CT-ICP window odometry gives a useful contrast. On `tunnel_geom_2700_200`, all
  selected windows pass the step/refinement health gate at 100% acceptance, even
  though the internal convergence bit remains low (10.3-20.7%). On `fog_200`,
  baseline and point-count-tail windows stay at 100% acceptance, while the
  strongest fog window drops to 65.5% because 10/29 pairs fail the refinement
  gate. This is a better gradient than KISS: CT-ICP does not collapse on all fog
  windows, but the strongest obscurant slice still creates measurable
  correction instability.
- Intensity BEV odometry is now in the same health contract. It accepts 100% of
  the selected tunnel and fog windows with the default search preset plus
  score-margin preferences. The earlier clear-fog zero-motion false lock is
  removed by preferring a >=0.25 m motion candidate when it is within 0.05
  normalized-correlation score of the zero-motion winner, while stress windows
  get a 0.02 low-motion regularizer so ambiguous reflectance matches are
  downgraded instead of accumulating into accepted drift.
- The combined method-health comparison is written to
  `experiments/results/lidar_degeneracy/method_health_comparison/`. Its current
  aggregate readout is: on `fog_200`, geometry ICP and KISS keyframe now average
  100% acceptance after their fog/crop correspondence gates were relaxed,
  intensity BEV averages 100%, and CT-ICP averages 88.5%; on
  `tunnel_geom_2700_200`, all four methods remain at 100% acceptance across the
  selected short windows. The aggregate now separates local risk, cross-method
  risk, total risk, and `pass`/`watch`/`investigate`/`fail` triage-policy counts.
- The comparison now includes a GT-free failure-awareness layer. Selected
  windows are labeled as nominal or stress, each method row is mapped to
  `ok`/`suspicious`/`degraded`/`failed`, and the failure-awareness table reports
  local flagged, cross-method flagged, residual unflagged, and false alarms.
  Because this layer has no ground truth, residual unflagged means the method
  stayed externally healthy on a stress window and did not trip the current
  cross-method risk gate; it does not prove the estimate is correct.
- The same report includes confidence probes for `stress_unflagged` rows:
  minimum score, minimum overlap, intensity-BEV decision reason distribution,
  and notes such as `motion_margin_dominant` or `overlap_tail`. This turns
  externally healthy stress windows into an explicit follow-up queue for GT or
  cross-method validation.
- The intensity BEV health runner now promotes two GT-free false-confidence
  probes into health flags on non-baseline selected windows: dominant
  score-margin decisions (`motion_margin` or `low_motion_margin`, default
  >=0.5) and a sharp overlap tail (min/mean overlap <0.5). With the current
  settings, the stress-only low-motion regularizer downgrades ambiguous
  intensity BEV stress windows instead of letting weak reflectance-score gains
  accumulate into large accepted drift. Baseline windows stay `ok`.
- Cross-method consistency is also reported for `stress_unflagged` rows. It
  compares each GT-free trajectory path length against healthy-peer and
  all-method path medians. It now promotes a row to `cross_method_suspicious`
  when it disagrees with the healthy-peer median, or when there is no healthy
  peer and it disagrees with the all-method median. With the current settings,
  intensity BEV has no residual stress-unflagged rows after the stress-only
  low-motion regularizer; ambiguous intensity rows are locally downgraded
  before they reach the cross-method acceptance path.
- Risk calibration is staged in
  `experiments/results/lidar_degeneracy/risk_gt_calibration/`. The current
  local NTNU extraction has no GT CSV and the downloaded bag topic audits show
  no pose/odom/tf topics, so the GT-backed rows are intentionally empty. The
  script can be rerun with external references via
  `python3 evaluation/scripts/calibrate_lidar_degeneracy_risk.py --gt-csv fog_200=... --gt-csv tunnel_geom_2700_200=...`.
  Until then, the stress-window proxy calibration separates `local_risk`
  (mean path/healthy 2.05) from `ok` (mean path/healthy 1.00), but this remains
  a triage signal rather than an error label.
- The same calibration report now includes a stress-window reason drilldown.
  Current strongest proxy signals include `low_motion_margin_dominant` and
  `low_used_path` on intensity BEV stress windows, plus all-method-only path
  disagreement on the remaining KISS fog row. In contrast, `ok_no_risk` rows
  stay near the healthy-peer path median. This is still GT-free, but it
  gives a concrete priority order for GT or stronger-reference checks.
- The calibration report also materializes a pre-GT triage policy:
  `fail` for hard local failures (`all_pairs_failed`, `low_acceptance`,
  `nonfinite_pose`), `investigate` for unresolved cross-method disagreement
  (`cross_method_suspicious`, `path_disagrees_with_healthy_median`), `watch` for
  calibrated local confidence downgrades and medium signals
  (`motion_margin_dominant`, `low_motion_margin_dominant`, `overlap_tail`, `low_convergence`,
  `ct_icp_internal_convergence_low`, `partial_acceptance`, all-method-only
  disagreement, no-healthy-peer context),
  and `pass` for `ok_no_risk`. On the current stress windows this yields
  0 `investigate` rows, 0 `fail` rows, 13 `watch` rows, and 15 `pass` rows.
- CT-ICP reports `ct_icp_internal_convergence_low` instead of generic
  `low_convergence` when the refinement gate accepts the update stream and
  `require_convergence=false`. Treat this as a diagnostic watch signal: the
  local update is usable under the configured gate, but the internal optimizer
  convergence rate still deserves runtime and calibration follow-up. When this
  is the only active CT-ICP signal, the comparison reports `diagnostic_watch`
  instead of counting it as product local risk or a nominal false alarm.
- The method-health comparison exposes CT-ICP refinement-gate rate and mean
  optimizer iterations in both the aggregate table and the diagnostic-watch
  rows. A candidate watch-clear condition should require accepted rate 1.0,
  refinement-gate rate 1.0, mean iterations not pinned to the configured max,
  and no extreme path disagreement against healthy peers or all methods. Until
  those checks are calibrated with GT or a stronger reference, keep
  `ct_icp_internal_convergence_low` as policy `watch`.
- The diagnostic-watch rows also include machine-readable
  `watch_clear_candidate`, `watch_action`, and `watch_clear_blockers` fields.
  `watch_action` separates `clear_candidate` rows from `optimizer_retry`
  rows, `reject_or_retry` rows where acceptance/refinement-gate checks fail,
  `fallback_required` rows where the accepted CT-ICP trajectory disagrees
  with healthy or all-method path references, and `reference_missing` rows
  where a path-ratio reference is unavailable. Current blockers include
  `accepted_below_one`, `refinement_gate_below_one`, `iterations_pinned`, and
  path-ratio blockers such as `path_vs_healthy_high` or
  `path_vs_all_high`.
- CT-ICP rows also expose a GT-free production-guard recommendation:
  `ct_icp_guard_decision`, `ct_icp_guard_reasons`, and
  `ct_icp_guard_uses_refined`. The decision keeps healthy or watch-cleared
  rows as `use_refined`, routes pinned diagnostic rows to `retry_optimizer`,
  routes path-reference disagreements to `fallback_to_prior`, and routes
  partial acceptance or hard local failures to `reject_or_retry`. This is an
  offline guard contract for the benchmark dashboard; production wiring still
  needs a live prior/fallback source.
- `simulate_ct_icp_guarded_trajectory.py` turns those guard decisions into an
  offline guarded trajectory using a bounded constant-velocity prior when
  refined CT-ICP should not be trusted. The report includes guarded path
  length, path/reference status, velocity-prior pair rate, hold rate, and step
  continuity. Treat it as a fallback-source sanity check: if
  `fallback_to_prior` stays `path_high` or becomes worse, the guard is doing
  the right thing by not trusting CT-ICP, but a stronger production fallback
  source is still required.
- `compare_ct_icp_fallback_sources.py` compares that self-velocity prior
  against already-computed external peer trajectories under the same GT-free
  path-reference contract. A fallback source is eligible only when its source
  health/risk state is `ok`, acceptance is high, and the path ratio is
  reference-consistent. On the current selected windows, geometry ICP is the
  selected fallback source for all CT-ICP guarded rows, while self velocity is
  reference-consistent on only a minority of rows. Treat this as evidence that
  a production fallback should be an independent odometry/source-health path,
  not just CT-ICP's own velocity extrapolation.
- `build_ct_icp_guarded_composite.py` materializes the current guard contract:
  use CT-ICP refined steps only for `use_refined`, otherwise use the selected
  fallback source from the bakeoff, with self-velocity only as a missing-pair
  backup. On the current selected windows the composite uses geometry ICP for
  every pair and becomes reference-consistent on all rows. That validates the
  wiring contract, but the 100% fallback rate also means this slice is still a
  fallback-system validation, not evidence that CT-ICP refined output should be
  trusted without a stronger guard or GT-backed check.
- The same policy is now integrated into the main method-health comparison:
  each window row includes `Policy` and `Policy reasons`, and each method
  aggregate includes policy counts. The calibration report remains the place
  for GT/proxy validation; the method-health comparison is the daily dashboard.
- The triage policy is versioned in
  `evaluation/config/lidar_degeneracy_triage_policy.json` as
  `lidar_degeneracy_triage_v4`. Both the method-health comparison and risk
  calibration reports load this file and stamp the policy path/version into
  their JSON and Markdown outputs, so future GT-backed precision/recall checks
  can be tied to the exact policy definition.
- The policy contract has a lightweight regression check:
  `python3 evaluation/scripts/test_lidar_degeneracy_triage_policy.py`. It
  validates required policy fields, representative reason decisions, unknown
  reason fallback to `watch`, and agreement between the method-health and
  calibration scripts.
- The failure-aware report layer has a single lightweight check entrypoint:
  `python3 evaluation/scripts/run_lidar_degeneracy_checks.py`. Add
  `--regenerate-reports` when you also want to refresh
  `method_health_comparison` and `risk_gt_calibration` from the current
  committed inputs.
- The check entrypoint is also wired into the repo script tests:
  `python3 -m pytest tests/test_experiment_scripts.py -k lidar_degeneracy_check_runner_smoke`.
- Use `python3 evaluation/scripts/run_lidar_degeneracy_checks.py --enforce-policy`
  for a strict gate. The gate reads the versioned
  `enforcement.strict_gate` budget in
  `evaluation/config/lidar_degeneracy_triage_policy.json` and returns non-zero
  when `method_health_comparison.json` or `risk_gt_calibration.json` has more
  `fail` or `investigate` rows than allowed. This is intentionally stricter
  than the daily dashboard path.
- Fixed-map NDT seed audits can be attached to the same strict gate with
  `--include-fixed-map-ndt-audit`. Keep this flag explicit: the current
  fixed-map audit intentionally contains known blockers, including CT-ICP seed
  rows that accept bad GT error and unfiltered Scan Context top-K rows where
  NDT score selects bad global hypotheses. Use
  `--regenerate-fixed-map-ndt-audit` to refresh
  `experiments/results/fixed_map_ndt/fixed_map_ndt_failure_audit.json` before
  attaching it to the gate.
- Add `--policy-gate-output-dir experiments/results/lidar_degeneracy/policy_gate`
  to write `policy_gate_report.json` and `policy_gate_report.md` with report
  counts, violations, and the top `fail`/`investigate` rows by
  sequence/window/method/reason.
- Convert the gate report into a repair backlog with
  `python3 evaluation/scripts/generate_lidar_degeneracy_action_plan.py`. This
  writes `policy_gate_action_plan.json` and `policy_gate_action_plan.md`, grouping
  offenders by policy decision, reason category, and method.

Do not use `/radar/cloud` as the LiDAR odometry input. Use it only for a
radar-aware baseline or after adding a radar-specific adapter.

## GT-Free Degeneracy Diagnostics

The upstream README does not advertise a ground-truth trajectory, so the first
benchmark layer should be diagnostic rather than ATE-based:

```bash
python3 evaluation/scripts/analyze_lidar_degeneracy_sequence.py \
  dogfooding_results/lidar_degeneracy_tunnel_200 \
  experiments/results/lidar_degeneracy/tunnel_200
```

Outputs:

- `frame_degeneracy.csv`
- `summary.json`
- `summary.md`

The key fields are:

- `linearity`: high in long tunnel-like geometry.
- `planarity`: high when points mostly occupy a plane.
- `scattering`: low when the 3D covariance has a weak third component.
- `degeneracy_score`: `max(linearity, planarity)`.
- intensity summary: useful for fog / obscurant windows.

## First Benchmark Slice

After PCD export and degeneracy-window selection, run short windows before full
trajectories. If you have an external reference trajectory, use it here:

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/lidar_degeneracy_tunnel_200 \
  experiments/reference_data/lidar_degeneracy_tunnel_gt.csv \
  200 \
  --methods ct_icp,fast_lio2,point_lio,kiss_icp
```

If ground truth is unavailable, still run odometry-only diagnostics and compare:

- trajectory smoothness,
- scan matching score,
- correspondence counts,
- runtime P95/P99,
- divergence / non-finite pose count,
- visual inspection of projected trajectory.

Do not fake GT just to get ATE. Keep the first pass as GT-free degeneracy
diagnostics plus method health metrics; add ATE only if a trustworthy external
pose source is available.

## Stop Line

Do not use this dataset to tune NDT/GICP precision recipes first. Use it after
KITTI recipes are stable, specifically to measure whether the method reports
degraded/unknown instead of silently producing confident wrong poses.
