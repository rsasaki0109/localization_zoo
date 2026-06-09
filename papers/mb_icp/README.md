# MbICP

From-paper reimplementation of **Metric-Based Scan Matching Algorithms for Mobile Robot
Displacement Estimation** by Minguez, Lamiraux, and Montesano (ICRA 2005 / T-RO 2006).

## Implemented

- Dense 2D `LaserScan` scan-to-scan odometry.
- Metric-based correspondence search against reference scan segments.
- Metric least-squares update using the paper's approximate configuration-space distance:
  translation error is preserved while tangential error at long range is down-weighted by the
  rotation-aware denominator `||p||^2 + L^2`.
- Motion-prior warm start from the previous increment.
- Trimmed correspondence set for outlier robustness.

## Simplifications

- Default harness path: scan-to-scan against the previous scan only.
- Optional rolling local map via `MbICPParams::use_local_map` with grid-indexed correspondence search.
  Offline smoke (Intel 7s / fr079 ~5min): drift improves to ~14% on Intel/fr079 vs ~17%/16.6% scan-to-scan,
  but the canonical harness keeps scan-to-scan for CI latency until robot-frame map caching lands.
- Visibility rejection and range-based outlier tests are approximated by metric distance gating
  plus trimming.
- The exact point-to-segment metric derivation is approximated by projecting onto scan segments
  and evaluating the metric at the projected reference point.

## Benchmark status

Integrated in `scan_dogfooding` as method selector `mb_icp`. See
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md) for the current
public 2D scan leaderboard.
