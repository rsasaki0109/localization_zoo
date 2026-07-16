# NTNU Cross-Dataset Triage Transferability

Policy: `lidar_degeneracy_triage_v4`
(`evaluation/config/lidar_degeneracy_triage_policy.json`), **unmodified** --
no thresholds were retuned for this analysis.

## What this is

`experiments/results/hard_pcl_localization/risk_gt_calibration/risk_gt_calibration.md`
calibrated the `lidar_degeneracy_triage_v4` policy's reason flags
(`all_pairs_failed`, `low_acceptance`, `low_convergence`,
`cross_method_suspicious` / `path_disagrees_with_all_method_median`, etc.)
against real GT error (ATE) on the Koide Hard PCL dataset, which ships
reference maps and ground truth. The NTNU LiDAR degeneracy bags
(`tunnel.bag`, `fog.bag`) have no such reference (no pose/odom/tf topic in
either bag's topic audit; see `risk_gt_calibration.md` in this directory),
so a GT-backed calibration is not possible here.

This report instead asks a narrower, honest question: **applied as-is, with
no re-tuning, does the Hard-PCL-calibrated policy's existing reason
vocabulary flag anything on the NTNU `tunnel_full` / `fog_full` full-
trajectory runs -- and does what it flags (or fails to flag) line up with
the qualitative failure this repo already documented independently** (both
`full_trajectory_comparison.md` files: every tunnel baseline accumulates
large net-displacement/yaw error while every per-pair health gate stays
green; fog is messier, with baseline-to-baseline acceptance rates spread
from 1% to 100%)? This is transferability evidence, not an accuracy claim
-- there is no GT here to say any flag is "correct."

## How it was built

1. `evaluation/scripts/build_lidar_degeneracy_full_trajectory_method_health.py`
   -- a new script that reads the eight `*_full_trajectory.json` result
   files already produced for `tunnel_full` and `fog_full` (`kiss_keyframe`,
   `litamin2`, `ct_icp`, `xicp`, `degen_sense_imu`, `degen_sense_no_imu`,
   `relead_imu`, `relead_no_imu`) and turns them into the same
   `method_health_comparison.json` schema
   `calibrate_lidar_degeneracy_risk.py` already consumes. It reuses
   `build_hard_pcl_method_health_comparison.py`'s exact flag thresholds
   line for line (`accepted_rate == 0` -> `all_pairs_failed`,
   `accepted_rate < 0.5` -> `low_acceptance`, `converged_rate < 0.5` ->
   `low_convergence`; `health_state` `ok`/`diagnostic_watch`/`fail` at the
   0.9/0.5 `accepted_rate` breakpoints; cross-method path ratio outside
   `[0.5, 2.0]` -> `cross_method_suspicious` /
   `path_disagrees_with_all_method_median`). Both `tunnel_full` and
   `fog_full` are recorded as `expected_stress="chronic_degeneracy"`
   (matching each sequence's own full-trajectory finding), so every row
   below is a stress row.
   Output: `ntnu_full_trajectory_method_health.json` (this directory).
2. `calibrate_lidar_degeneracy_risk.py --comparison-json
   ntnu_full_trajectory_method_health.json --output-dir
   ntnu_full_trajectory/` -- the **same, unmodified** calibration script
   already used for the Hard PCL report and the short-window NTNU report,
   pointed at the new comparison file. No GT CSV was passed (none exists).
   Output: `ntnu_full_trajectory/risk_gt_calibration.json` and `.md` (this
   directory).

Repro:

```bash
python3 evaluation/scripts/build_lidar_degeneracy_full_trajectory_method_health.py
python3 evaluation/scripts/calibrate_lidar_degeneracy_risk.py \
  --comparison-json experiments/results/lidar_degeneracy/risk_gt_calibration/ntnu_full_trajectory_method_health.json \
  --output-dir experiments/results/lidar_degeneracy/risk_gt_calibration/ntnu_full_trajectory
```

## Per-method flag rates (16 rows: 8 methods x 2 sequences)

| Method | `tunnel_full` decision | `fog_full` decision | Active reasons |
| --- | --- | --- | --- |
| `ct_icp` | `watch` | `fail` | tunnel: `low_convergence` (internal-convergence-bit diagnostic). fog: `low_acceptance`, `low_convergence`, `cross_method_suspicious` |
| `relead_imu` | `fail` | `fail` | both: `low_acceptance`, `low_convergence`, `cross_method_suspicious` (near-total rejection on both sequences) |
| `xicp` | `pass` | `fail` | tunnel: `ok_no_risk` (not flagged). fog: `low_acceptance`, `low_convergence` |
| `relead_no_imu` | `investigate` | `pass` | tunnel: `cross_method_suspicious` (path is the longest of any method, 826.9 m). fog: `ok_no_risk` |
| `kiss_keyframe` | `investigate` | `pass` | tunnel: `cross_method_suspicious` (path is short relative to the all-method median, which `relead_no_imu`'s outlier path inflates). fog: `ok_no_risk` |
| `degen_sense_no_imu` | `pass` | `investigate` | tunnel: `ok_no_risk`. fog: `cross_method_suspicious` (longest fog path, 322.9 m) |
| `degen_sense_imu` | `pass` | `pass` | both: `ok_no_risk` |
| `litamin2` | `pass` | `pass` | both: `ok_no_risk` |

Aggregate policy decision counts (both sequences combined): `pass` 8,
`watch` 1, `investigate` 3, `fail` 4. Full per-row detail:
`ntnu_full_trajectory/risk_gt_calibration.md`.

## Does it correlate with the known qualitative failure?

**Partially, and the gap is informative.** The `tunnel_full` write-up's
headline finding is that every baseline (`kiss_keyframe`, `litamin2`,
`ct_icp`, `xicp`) accumulates hundreds of meters of unrecovered net
displacement against a known ~500 m straight tunnel while every per-pair
health gate (accepted, converged, step gate) stays 100% green -- that is
precisely the scenario this triage layer was built to eventually catch.
Checked against that:

- **Caught, for the "wrong" reason:** `kiss_keyframe` on `tunnel_full` is
  flagged `investigate` -- but via `cross_method_suspicious`
  (its path length disagrees with the all-method median), not because
  anything about its own health metrics looks bad (its own accepted/
  converged rate is 100%/100%). The flag exists only because
  `relead_no_imu`'s anomalously long path (826.9 m, a near-random-walk
  degeneracy artifact of its own) pulls the all-method median around
  enough to make `kiss_keyframe`'s comparatively short 106.2 m path look
  like an outlier by ratio. Remove `relead_no_imu` from the comparison set
  and this flag would very likely disappear -- it is not a robust,
  method-independent signal.
- **Not caught at all:** `litamin2` and `xicp` are both flagged `pass`
  / `ok_no_risk` on `tunnel_full`, despite `litamin2` being one of the
  three baselines with the exact same "clean local health, catastrophic
  accumulated drift" failure (63.0 m net vs. a true ~500 m path, 27.1 deg
  yaw drift) and `xicp` showing the detection-without-recovery pattern
  (99.8% of frames flagged internally degenerate by X-ICP's own
  localizability classifier, yet still only 33.1 m net). Neither
  `accepted_rate` nor `converged_rate` -- the two signals this policy's
  flag vocabulary is built from at the full-trajectory level -- drops for
  either method, because the tunnel's failure mode is specifically that
  local registration keeps succeeding while the *trajectory shape* is
  wrong. The current reason vocabulary has no trajectory-shape signal
  (e.g. net-displacement-to-path-length ratio, or accumulated yaw-rate
  variance) at all, so it structurally cannot see this failure mode no
  matter how it is tuned at the per-pair-acceptance level.
- **`ct_icp`'s `low_convergence` (tunnel) is exactly the existing
  diagnostic-watch signal**, unchanged from the short-window NTNU
  calibration -- CT-ICP's internal convergence bit stays a `watch`-tier
  signal here too, consistent rather than newly revealing.
- **On `fog_full`, the policy performs much better** -- `ct_icp`,
  `xicp`, and `relead_imu` are all correctly flagged `fail` via genuinely
  depressed `accepted_rate`/`converged_rate` (21.1%, 47.3%, and 1.1%
  accepted respectively). This is not a coincidence: fog's stress
  mechanism (obscurant point-count/intensity/range collapse) actually
  degrades per-pair registration health directly, so the accepted/
  converged-rate signals this policy already tracks are the *right*
  signals for fog. Tunnel's stress mechanism (weak longitudinal geometric
  constraint) does not degrade per-pair health at all -- only the
  accumulated trajectory shape -- which is exactly the case this
  vocabulary misses.

## Verdict

The Hard-PCL-calibrated `lidar_degeneracy_triage_v4` policy **transfers
usefully to fog-type (sensing-degradation) stress** on NTNU data: its
existing `low_acceptance`/`low_convergence` flags correctly separate
`ct_icp`/`xicp`/`relead_imu`'s genuine fog struggles from the methods that
hold up. It **does not transfer to tunnel-type (geometric self-similarity)
stress**: the one flag that fires on a genuinely bad tunnel baseline
(`kiss_keyframe`, via `cross_method_suspicious`) does so for a fragile,
comparison-set-dependent reason, and the two other baselines with the same
documented failure (`litamin2`, `xicp`) are marked `pass`. This is
consistent with, and sharpens, the `tunnel_full` doc's own conclusion: *"A
GT-free per-pair health gate is not sufficient for degeneracy detection ...
consider a trajectory-level check (net displacement vs. path length ratio,
or accumulated yaw-rate variance) as an additional triage signal."* This
report shows concretely that the currently-shipped, GT-calibrated policy
lacks exactly that signal, and that its absence is not visible on Hard PCL
or on fog -- only on the tunnel's specific chronic-degeneracy failure mode.
Adding a trajectory-level reason (and calibrating it, ideally against a
dataset with real GT that exhibits this same failure mode) remains the
concrete next step, not a re-tuning of the existing per-pair thresholds.

Cross-linked from `risk_gt_calibration.md` in this directory.
