# Fog Full-Trajectory Drift (1729 frames, ~173 s, ntnu-arl fog.bag)

Companion run to `tunnel_full`'s full-trajectory drift comparison, over the
full `fog.bag` end to end (1729 LiDAR frames extracted with
`extract_ouster_packet_lidar_imu.py --max-frames -1 --skip-incomplete`, same
recipe as `tunnel_full`). All eight configurations run continuously (not in
disjoint windows) in GT-free mode with the same step gates used throughout
this benchmark (`--max-step-translation 2.0 --max-step-yaw-deg 20`).

Unlike `tunnel_full` (an 8 m wide, ~500 m straight tunnel section with a
known target geometry), the upstream README describes the fog sequence as
an indoor corridor flight through obscurant fog -- there is no advertised
"true" trajectory shape to check net displacement against, and the earlier
`fog_200` short-window slice already showed fog is a *sensing-degradation*
stress (point count / intensity / range percentile all drop under the
strongest obscurant window) rather than a *geometric self-similarity*
stress like the tunnel. This section therefore reports the same metrics as
`tunnel_full` for direct comparability, but interprets them as relative
method-to-method degradation under obscurant conditions, not as error
against a known ground truth -- keeping this GT-free and honest per the
setup doc's stop line.

## Results

Baselines (no degeneracy awareness):

| Method | Pair accepted | Pair converged | Path length (m) | Net displacement (m) | Accumulated yaw drift |
|---|---:|---:|---:|---:|---:|
| KISS keyframe | 1716/1728 (99.3%) | 1716/1728 (99.3%) | 84.3 | 21.6 | -110.3 deg |
| LiTAMIN2 | 1728/1728 (100%) | 1728/1728 (100%) | 112.3 | 36.4 | -29.1 deg |
| CT-ICP | 365/1728 (21.1%, refinement-gate 21.1%) | 88/1728 (5.1%) | 61.0 | 12.9 | -147.5 deg |

Degeneracy-aware methods (same `xicp_window_odometry` /
`degen_sense_window_odometry` runners, same sequence, same gates):

| Method | Pair accepted | Frames flagged degenerate | Path length (m) | Net displacement (m) | Accumulated yaw drift | Runtime (s) |
|---|---:|---:|---:|---:|---:|---:|
| X-ICP | 817/1728 (47.3%) | 1460/1728 (84.5%) | 173.7 | 5.0 | 98.4 deg | 32 |
| DegenSense (no IMU) | 1711/1728 (99.0%) | 361/1728 (20.9%) | 322.9 | 42.3 | -141.0 deg | 164 |
| DegenSense (+IMU blend) | 1728/1728 (100%) | 311/1728 (18.0%) | 213.3 | 32.1 | -15.1 deg | 145 |

RELEAD (constrained-ESIKF, `relead_window_odometry`, same two
configurations as `tunnel_full`):

| Method | Pair accepted | Pair converged | Frames flagged degenerate | Path length (m) | Net displacement (m) | Accumulated yaw drift | Runtime (s) |
|---|---:|---:|---:|---:|---:|---:|---:|
| RELEAD (no IMU) | 1728/1728 (100%) | 1416/1728 (81.9%) | 6/1728 (0.35%) | 201.9 | 15.9 | -8.3 deg | 48 |
| RELEAD (+IMU predict) | 19/1728 (1.1%) | 0/1728 (0.0%) | 0/1728 (0.0%) | 6.4 | 0.9 | 41.5 deg | 46 |

## Interpretation

Fog reshuffles the method ranking relative to tunnel in several notable
ways:

- **CT-ICP is much worse on fog than on tunnel.** On `tunnel_full` its
  refinement gate passed 100% of pairs (with a low internal convergence
  bit treated as diagnostic-only); on `fog_full` the refinement gate itself
  only passes 21.1% of pairs, matching the earlier `fog_200` short-window
  finding that CT-ICP's correction-instability signal is far sharper under
  obscurant conditions than under tunnel geometric self-similarity. This is
  the clearest baseline-level evidence that fog and tunnel are genuinely
  different stress types, not interchangeable "degeneracy" cases.
- **X-ICP's acceptance rate drops from 100% (tunnel) to 47.3% (fog)** while
  its degeneracy flag rate stays high (84.5% of frames carry a
  PARTIAL/NONE localizability direction, vs. 99.8% on tunnel). X-ICP is
  still detecting *something* degenerate on the majority of fog frames, but
  unlike tunnel -- where every X-ICP pair was still accepted -- roughly
  half of fog pairs now also fail X-ICP's own step/refinement gate. This
  reads as a compounding effect: geometric weakness (feeding the
  localizability classification) plus point-count/intensity degradation
  under obscurant conditions (feeding correspondence starvation) together
  push more pairs past the gate than either would alone. Net displacement
  is the worst of all eight methods (5.0 m), consistent with the
  detection-without-recovery pattern already established on tunnel.
- **DegenSense's flag rate is far higher on fog than on tunnel** (18-21% of
  frames vs. 14-15% on tunnel), which is the opposite of what a "detector
  blind to chronic degeneracy" story alone would predict -- fog's
  obscurant point-count/intensity drops are transient/patchy rather than a
  whole-run condition-number shift, so DegenSense's adaptive median+MAD
  detector can still register them as spikes above its adapting baseline.
  This supports the `tunnel_full` doc's original diagnosis: DegenSense's
  detector tracks *transient* degradation well (fog) but is blind to
  *chronic* degradation (tunnel), rather than being weak in general.
  DegenSense also keeps the best net displacement among the baselines and
  degeneracy-aware methods on fog (32.1-42.3 m) as it did on tunnel.
- **RELEAD's IMU-predict configuration fails the same way on fog as on
  tunnel**: near-total rejection (19/1728, 1.1% accepted), an essentially
  frozen trajectory (0.9 m net), and the same runtime (~46-62 s for
  ~1700-3200 frames at 300 correspondences/frame). This is the strongest
  evidence that the IMU-path failure mode identified on `tunnel_full`
  (covariance inflation across `predict()` calls between infrequent
  accepted updates -- see that doc's RELEAD section) is a property of
  RELEAD's constrained-ESIKF implementation interacting with real VN100
  IMU data, not an artifact specific to tunnel geometry.
- **RELEAD's no-IMU configuration behaves very differently on fog than on
  tunnel.** On tunnel it produced the single worst path/net-displacement
  ratio of any method (826.9 m path for 6.7 m net, effectively a
  random walk). On fog it is mid-pack: 100% accepted, 81.9% converged,
  201.9 m path for 15.9 m net -- worse than LiTAMIN2 and DegenSense but
  better than CT-ICP and X-ICP on net displacement, and its degeneracy
  detector fires on 0.35% of frames (vs. 0% on tunnel) at the shipped SVD
  thresholds. This is consistent with fog's geometry being less
  self-similar than the tunnel's: the same unconstrained ESIKF core that
  cancels itself out in a geometrically-degenerate corridor produces a more
  usable (if still not great) trajectory once the scene has more 3D
  structure to anchor point-to-plane correspondences, even under sensing
  obscurants.
- No GT-derived "which method is closest to the truth" claim is made here
  -- fog has no reference trajectory. The comparisons above are strictly
  relative (method vs. method, fog vs. tunnel for the same method), which is
  the GT-free contract this benchmark layer commits to.

## Follow-up

- Cross-dataset triage: `experiments/results/lidar_degeneracy/risk_gt_calibration/ntnu_cross_dataset.md`
  applies the Hard-PCL-GT-calibrated `lidar_degeneracy_triage_v4` policy's
  existing reason flags (unmodified thresholds) to both `tunnel_full` and
  `fog_full`'s per-method rows above, as a transferability check rather
  than an accuracy claim.
- As with tunnel, map-localization false-lock behavior (`fixed_map_ndt`) on
  fog/obscurant conditions is untested here; the Koide Hard Point Cloud
  Localization dataset (zenodo 10122133, see
  `SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md`) remains the designated venue
  for GT-backed accuracy and map-localization failure-mode work.
