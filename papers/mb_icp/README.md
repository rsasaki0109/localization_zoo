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

- Optional rolling local map via `MbICPParams::use_local_map`.
- **Robot-frame map cache** (2026-06): the local map is stored in the current robot frame,
  updated by `inv(increment)` after each registration instead of re-transforming world points
  every frame. A grid index is rebuilt once per map update.
- Default harness enables local map (`use_local_map=true`) for drift gains on real logs.
- Visibility rejection and range-based outlier tests are approximated by metric distance gating
  plus trimming.
- The exact point-to-segment metric derivation is approximated by projecting onto scan segments
  and evaluating the metric at the projected reference point.

## Local map benchmark (robot-frame cache, Intel 73f / fr079 384f)

| Mode | Intel drift | fr079 drift | Intel time |
|------|------------:|------------:|-----------:|
| scan-to-scan | 17.1% | 16.6% | ~1s |
| local map + cache | **14.5%** | **15.4%** | ~22s |

Corridor synthetic: **0.05%** drift with local map. fr079 384f takes ~9 min — heavy for full benchmark refresh.

## Benchmark status

Integrated in `scan_dogfooding` as method selector `mb_icp`. See
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md) for the current
public 2D scan leaderboard.
