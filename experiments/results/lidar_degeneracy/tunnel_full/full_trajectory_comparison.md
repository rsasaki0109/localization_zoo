# Tunnel Full-Trajectory Drift (3241 frames, ~324 s, ntnu-arl tunnel.bag)

Prior short-window health checks (tunnel_200, tunnel_geom_2700_200, 200 frames
each) reported 100% pair acceptance for geometry ICP, KISS keyframe, LiTAMIN2,
and CT-ICP, and concluded this was a negative result for local odometry on the
tunnel sequence. This run extracts and processes the full tunnel.bag (3241
LiDAR frames, ~5.4 minutes) continuously through the same three window-runner
binaries in GT-free mode, to test the doc's own follow-up hypothesis: that a
longer trajectory would surface degeneracy drift invisible to short-window
per-pair health gates.

The upstream README describes this as an 8 m wide, ~500 m straight tunnel
section, so a healthy odometry estimate should look close to a straight
~500 m line with near-zero net yaw drift.

## Results

Baselines (no degeneracy awareness):

| Method | Pair accepted | Pair converged | Path length (m) | Net displacement (m) | Accumulated yaw drift |
|---|---:|---:|---:|---:|---:|
| KISS keyframe | 3240/3240 (100%) | 3240/3240 (100%) | 106.2 | 24.0 | 56.3 deg |
| LiTAMIN2 | 3240/3240 (100%) | 3240/3240 (100%) | 273.0 | 63.0 | 27.1 deg |
| CT-ICP | 3240/3240 (100%) | 387/3240 (11.9%, refinement-gate 100%) | 474.7 | 79.0 | 15.5 deg |

Degeneracy-aware methods (new `xicp_window_odometry` and
`degen_sense_window_odometry` runners, same sequence, same gates):

| Method | Pair accepted | Frames flagged degenerate | Path length (m) | Net displacement (m) | Accumulated yaw drift | Runtime (s) |
|---|---:|---:|---:|---:|---:|---:|
| X-ICP | 3240/3240 (100%) | 3234/3240 (99.8%) | 332.1 | 33.1 | -22.9 deg | 45 |
| DegenSense (no IMU) | 3240/3240 (100%) | 497/3240 (15.3%) | 457.3 | 105.3 | 26.8 deg | 948 |
| DegenSense (+IMU blend) | 3240/3240 (100%) | 453/3240 (14.0%) | 372.7 | 115.4 | 26.9 deg | 1057 |
| RELEAD (no IMU) | 3240/3240 (100%) | 0/3240 (0.0%) | 826.9 | 6.7 | 16.1 deg | 68 |
| RELEAD (+IMU predict) | 11/3240 (0.3%) | 0/3240 (0.0%) | 7.5 | 1.8 | 48.9 deg | 62 |

(RELEAD's converged rate -- a separate field from "accepted" and from its
own degeneracy flag -- was 94.4% for the no-IMU run and 0.0% for the
IMU-predict run; see the RELEAD section below for the full accepted/
converged/degenerate breakdown, since the table above only has room for
the "flagged degenerate" column the other degeneracy-aware methods use.)

None of the eight configurations reach anywhere close to the true ~500 m
straight path. The best net displacement is DegenSense+IMU at 115.4 m --
still under a quarter of the true traversal. CT-ICP's total path length is
the closest to 500 m, but its net displacement is only 79 m, meaning the
estimated path curls back on itself rather than continuing straight -- the
tunnel's weak longitudinal constraint is being aliased into spurious
yaw/lateral motion, a textbook degeneracy failure mode. KISS keyframe is
worst among the baselines: 106 m of path for 24 m of net displacement with
56 degrees of accumulated yaw error. RELEAD is worse still on both axes (see
below): its no-IMU variant produces the *longest* path of any method
(826.9 m, more than the true tunnel length) for the *smallest* net
displacement (6.7 m), and its IMU variant essentially never moves at all.

### Detection vs. mitigation

The two degeneracy-aware methods split cleanly along the
detection/mitigation axis:

- **X-ICP detects the degeneracy essentially perfectly**: 99.8% of frames
  carry 1-2 degenerate directions in its localizability classification
  (typically the third translation/rotation axis at PARTIAL or NONE),
  exactly matching the tunnel geometry. This is the runtime signal the
  baselines entirely lack -- every baseline health gate stayed green for
  all 3240 pairs. But constraining updates along the unobservable direction
  cannot recover true longitudinal motion (net 33.1 m): X-ICP prevents
  false motion, it does not restore missing information.
- **DegenSense's adaptive spike detector is blind to chronic degeneracy**:
  its median + k*MAD rule over the degeneracy-factor time series adapts to
  the tunnel's persistently elevated condition numbers (S_rot median ~420-620
  across the run) and only flags spikes above that baseline (14-15% of
  frames). The paper designed the detector for transient degradation; a
  whole-run degenerate environment moves the median itself, so most
  degenerate frames read as "normal". Absolute-threshold classification
  (X-ICP) and adaptive outlier detection (DegenSense) give opposite answers
  on the same data.
- **IMU blending helps net displacement modestly** (105.3 -> 115.4 m) but
  does not touch yaw drift (26.8 vs 26.9 deg): the VN100's accelerometer
  cannot observe near-constant-velocity longitudinal motion over ~5 minutes
  any better than the LiDAR can, so the missing constraint stays missing.

The practical conclusion matches the motivation of the dataset's own paper
(radar-inertial fusion) and the degenerate-environment fusion arguments in
the reference talks (wheel odometry / Doppler velocity / intensity
constraints): in a chronically degenerate corridor, detection is achievable
from LiDAR alone (X-ICP's classification), but recovery requires a sensor
that actually observes the degenerate direction.

### RELEAD (constrained-ESIKF)

`relead_window_odometry` (mirrors `xicp_window_odometry`'s map/KdTree
correspondence building and step/refinement gate, but drives RELEAD's
`CESIKF` -- `papers/relead/include/relead/cesikf.h` -- instead of X-ICP)
was run over the same `tunnel_full` sequence in two configurations: fed real
VN100 `imu.csv` samples through `CESIKF::predict()`, and with `--no-imu`
(seeded by the same constant-velocity extrapolation the other runners use,
written directly into `CESIKF`'s mutable `state()`). `--max-source-points`
was reduced from the X-ICP default of 2500 to 300: `CESIKF::update()`
inverts a dense `n x n` Kalman innovation covariance (`n` = correspondence
count, one scalar row per point-to-plane residual), so its per-frame cost is
O(n^3) rather than the O(n) Gauss-Newton normal-equations cost X-ICP/CT-ICP
use; 300 correspondences keeps a full 3241-frame run to ~1 minute, while
2500 was untested but would be roughly 500x more expensive per frame.

Both configurations are clearly worse than every baseline and both
degeneracy-aware methods, in different ways:

- **RELEAD + IMU predict rejects almost every frame (11/3240, 0.3%
  accepted)** and the trajectory is essentially frozen (1.8 m net over the
  whole run, final yaw error 48.9 deg from the handful of accepted steps).
  Diagnosing this required two runner-side fixes before the number was even
  this good (see the header comment in `relead_window_odometry.cpp` for
  full detail): (1) a rejected pair must roll back RELEAD's *entire*
  `State` (rotation, position, velocity, biases, gravity), not just
  rotation/position -- otherwise a poisoned velocity estimate keeps driving
  position drift through `predict()` on every later frame even while
  rotation/position are being reset; (2) a rejected pair must also call
  `CESIKF::reset()` to clear the internal covariance `P_` back to its tight
  prior, because `predict()` has no counterbalancing shrinkage step the way
  an *accepted* `update()` does, so repeated `predict()` calls across
  rejected frames keep inflating `P_` with fresh process noise every IMU
  sample. Before fix (2), single-frame ESIKF corrections routinely swung
  ~20 deg away from the IMU seed -- verified against directly-integrated
  VN100 gyro data, which shows <0.05 deg of real rotation per 0.1 s frame
  in this stretch of the bag, so the swings were a filter-covariance
  artifact, not real motion. After both fixes the runaway divergence is
  gone, but the underlying disagreement between RELEAD's single-shot ESIKF
  correction and its own IMU-seeded prediction remains large enough
  (still exceeding the refinement gate's 0.25 rad / 2 m bounds on most
  frames) that acceptance stays near zero.
- **RELEAD without IMU (velocity-model seeded) accepts every pair (100%)
  and converges on 94.4%,** yet produces the single worst path-length-to-
  net-displacement ratio of any method tested here: 826.9 m of accepted
  path -- longer than the true ~500 m tunnel and longer than every other
  method's path -- for only 6.7 m of net displacement. Where CT-ICP's path
  "curls back on itself" (79 m net from 474.7 m path), RELEAD's path
  effectively cancels out entirely, consistent with a per-frame ESIKF
  correction that is locally self-consistent step-to-step (step/refinement
  gates pass) but has no persistent forward bias -- a near-random-walk
  rather than a curling drift.
- **RELEAD's own degeneracy detector never fires on this data** (`degenerate=0`
  across all 3240 pairs in both configurations, at the shipped
  `none_threshold=0.01` / `partial_threshold=0.1` singular-value-ratio
  cutoffs). Unlike X-ICP, which flags 99.8% of tunnel frames as degenerate
  along the third axis, RELEAD's constrained-update machinery is
  essentially inactive throughout this run: both configurations are really
  exercising RELEAD's *unconstrained* ESIKF core rather than its
  degeneracy-aware constraint. That the constraint never engages while the
  unconstrained update still produces the worst net-displacement result of
  any tested method is itself informative -- it suggests the paper's SVD
  ratio thresholds, tuned presumably against its own reference platform/
  scan characteristics, do not transfer as-is to this repo's 300-point
  correspondence sets on the NTNU tunnel geometry.
- No new algorithm code was written in `papers/relead` to produce these
  numbers -- only the runner (`evaluation/src/relead_window_odometry.cpp`),
  which calls `CESIKF::predict()`, `CESIKF::update()`, `CESIKF::reset()`,
  and the mutable `state()` accessor exactly as the class already exposes
  them.

## Interpretation

This confirms the doc's own prediction: short-window (200-frame, ~20 s) pair
health gates are blind to this failure. Each individual pairwise registration
step stays within the accepted step-size gate (2 m translation / 20 deg yaw)
and is locally "converged"/"accepted", so the health-check layer never
flags a problem -- but the accumulated trajectory over the full tunnel is
wrong by hundreds of meters of unrecovered net displacement. The prior
negative result (tunnel does not visibly break local odometry) was an
artifact of the 200-frame window being too short to let degeneracy drift
accumulate into something a per-pair gate would notice.

This is exactly the kind of failure degeneracy-aware methods (RELEAD's
Hessian-SVD degeneracy detection, X-ICP) target: none of the three baselines
tested here detect or constrain the degenerate direction, so it silently
accumulates into large drift while every local health signal stays green.

## Follow-up

- ~~Run X-ICP (degeneracy-aware) over the same full tunnel_full sequence~~
  Done above.
- ~~Run RELEAD's constrained-ESIKF frontend over tunnel_full~~ Done above
  (`relead_window_odometry`, both with real IMU predict and with a
  velocity-model seed). Headline: both configurations are worse than every
  other method tested here, and RELEAD's own SVD-ratio degeneracy detector
  never fires on this data -- see the RELEAD section above.
- Promote X-ICP's per-frame localizability classification into the triage
  policy as a runtime degeneracy signal: it flags 99.8% of tunnel frames
  while every existing GT-free health gate stays green. DegenSense's
  median+MAD detector should NOT be used for chronic degeneracy (it adapts
  to the elevated baseline; see Detection vs. mitigation above).
- A GT-free per-pair health gate is not sufficient for degeneracy detection;
  consider a trajectory-level check (e.g. net displacement vs. path length
  ratio, or accumulated yaw-rate variance) as an additional triage signal.
  See `experiments/results/lidar_degeneracy/risk_gt_calibration/ntnu_cross_dataset.md`
  for a first attempt at applying the Hard-PCL-GT-calibrated
  `lidar_degeneracy_triage_v4` policy's existing reason flags to this
  full-trajectory data.
- Map-localization false-lock behavior (fixed_map_ndt) on tunnel geometry is
  still untested here; the Koide Hard Point Cloud Localization dataset
  (zenodo 10122133, see SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md) ships
  reference maps + GT and is the designated venue for that track.
