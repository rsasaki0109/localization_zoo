# Paper-Ready Reproducibility Plan

This document defines the bar for turning Localization Zoo from a broad
implementation catalog into evidence strong enough for a workshop, systems, or
replication paper.

The short version: the repository already has useful breadth and honest
negative results. The next quality step is not "add more methods"; it is to make
the claim boundary explicit, harden a smaller set of representative methods,
and publish enough protocol detail that another lab can regenerate the core
tables.

## Claim Tiers

| Tier | Meaning | Allowed claim |
|---|---|---|
| **T0 paper-grade** | Same dataset family, full sequence, matching metric, documented deviations, raw JSON artifact, unit tests for the paper mechanism, and at least one ablation that isolates the claimed mechanism. | "We reproduce or fail to reproduce the paper mechanism under a documented protocol." |
| **T1 mechanism-grade** | Core mechanism is implemented and unit-tested, full-sequence repo benchmark exists, but sensor inputs, backend, or constants differ enough to block a one-to-one paper comparison. | "This mechanism is active and its effect under our shared protocol is..." |
| **T2 adapter-grade** | Paper idea is adapted onto shared infrastructure or missing sensors are approximated, with full benchmark artifacts and clear caveats. | "This is an adapter, not a faithful paper reproduction." |
| **T3 smoke / concept** | Compiles, has tests, and is useful for API or benchmark coverage, but lacks paper-comparable evidence. | "This is a compact baseline or concept port." |

Every public table should keep these tiers separate. A method can be useful
without being paper-grade.

## Paper-Grade Checklist

A method is ready for a paper table only when all items below are true:

- **Source audit**: paper title, venue, authors, dataset claims, metrics, and
  public-code status are recorded in the method README.
- **Deviation sheet**: every missing subsystem, substituted sensor, changed
  backend, or omitted optimization stage is listed.
- **Mechanism tests**: unit tests exercise the paper-specific mechanism, not
  only generic registration.
- **Full-sequence artifacts**: raw JSON artifacts for at least KITTI seq00 and
  seq07 exist under `docs/benchmarks/kitti_full_new_methods/`.
- **Ablation**: one switch or paired run isolates the mechanism being claimed
  when the implementation permits it.
- **Baseline context**: KISS-ICP / CT-ICP or the nearest family baseline is run
  under the same preprocessing profile.
- **Protocol reproducibility**: the README includes the exact command line and
  dataset-prep path using relative repo paths.
- **Claim language**: the README states whether the result is reproduced,
  approximately reproduced, mechanism-grade, adapter-grade, or an honest
  negative.

## Current Paper-Ready Candidates

These are the strongest near-term candidates for a focused paper because they
combine a distinct mechanism, full-sequence artifacts, and interpretable
positive or negative evidence.

| Method | Current tier | Why it matters | Missing work to reach T0 |
|---|---|---|---|
| I-LOAM | T0 evidence candidate | Intensity weighting shows a controlled 18-20% drift reduction on KITTI with committed seq00/07 on/off raw artifacts. | Locked in the frozen evidence bundle; keep paired commands/artifacts stable. |
| KC-LO | T0 evidence candidate | Correspondence-free kernel correlation beats KISS-ICP on seq00/07, and the committed sigma-schedule ablation records the runtime/accuracy trade-off. | Locked in the frozen evidence bundle; keep annealed/fixed-sigma artifacts stable. |
| DegenSense | T1 evidence candidate | Degeneracy sensing is competitive as a no-IMU KITTI fallback; IMU/LiDAR compensation activates on HDL-400 open and public NCLT 2013-01-10 (NCLT ATE ~45% worse without IMU on 120 frames). | Add to manuscript table with KITTI no-IMU fallback + IMU mechanism rows locked. |
| D2-LIO | T1 evidence candidate | Directional degeneracy diagnostics are competitive as a no-IMU KITTI fallback; IMU-prior regularization activates on public HDL-400 open with small RPE delta vs no-`imu.csv` fallback on that window. | Add to manuscript table with KITTI no-IMU fallback + IMU mechanism rows locked. |
| M-GCLO | T1+ evidence candidate | Ground-factor off ablation is committed on KITTI seq00/07, synthetic rolling-ground stress shows ground on wins, public hilly KITTI seq08 shows ATE +149% with ground off at similar RPE, MulRan no-gt-seed diverges (~103% RPE), and GT-seed MulRan stabilizes tracking but ground off slightly improves ATE (oracle-init). | Blind MulRan odometry or IMU-backed protocol before promoting to T0. |
| Quadric-LO | T1+ evidence candidate | Plane-fallback ablation is committed on KITTI seq00/07, synthetic curved-object stress confirms quadric-path dominance, and public residential KITTI seq02 shows >99% quadric correspondences though rare plane fallback carries more weight than on highway sequences. | Add dedicated orchard or non-urban multi-beam benchmarks before promoting to T0. |
| LiDAR-IBA | T1 evidence candidate | Stereographic plane front-end is competitive on KITTI; committed no-BA profile beats BA-on for RPE on IMU-free seq00/07 with paired ablation JSON. | Validate BA + IMU on a synchronized LiDAR-IMU benchmark before full LIO claims. |
| TrICP-LO | T1 evidence candidate | LTS trimmed point-to-plane odometry is near KISS-ICP on seq00/07; paired overlap ablation shows FRMSD auto sticks to ξ=0.800 on clean KITTI. | Add high-outlier/non-overlap stress before promoting FRMSD overlap claims beyond clean KITTI. |
| LiDAR-visual adapters | T2 | They show pseudo-visual residuals are stable but auxiliary on KITTI PCD. | Do not call these paper-grade until real RGB / camera synchronization is used or the paper claim is reframed as a KITTI-PCD adapter study. |
| RF-LIO / ID-LIO | T1/T2 | Dynamic filtering mechanisms are active on KITTI and in synthetic dynamic-object stress; public urban KITTI seq05 dense-profile check shows both paths active but RF-LIO still trails ID-LIO and KISS-ICP; IMU gyro priors activate on public HDL-400 open with small metric deltas vs no-`imu.csv` fallback. | Add dedicated high-dynamic multi-beam benchmarks before manuscript-level dynamic-scene claims. |

## IMU Dead Reckoning Family Comparison (T3)

[`imu_dead_reckoning`](../papers/imu_dead_reckoning/README.md) (102nd method)
is not a paper-ready candidate -- it is **T3 smoke / concept** per the tier
table above: "This is a compact baseline or concept port." It is the unaided
strapdown IMU-only dead-reckoning lower bound for the LIO family, and the
useful evidence it provides is comparative, not a manuscript claim: three
existing from-paper methods -- **OdoNet**, **NHC-Net**, and **NN-ZUPT** --
share the same strapdown core plus a trained CNN aid, so running all four on
the same window shows what a cheap learned aid buys over unaided dead
reckoning. This section promotes that comparison out of the module README
(previously the only place it lived) so the claim boundary is visible at the
top level. None of these four methods are LiDAR/LIO, none are on the KITTI
point-cloud leaderboard, and IMU-DR's own ZUPT/gyro-bias ablations below are
diagnostic, not a recommended configuration -- the method's repository
default stays pure, unaided dead reckoning throughout.

### NCLT 2013-01-10 (ms25 IMU, ground vehicle)

**120-frame window** (11.5 m trajectory, ~24 s). Manifest:
[`experiments/imu_dead_reckoning_nclt_2013_01_10_matrix.json`](../experiments/imu_dead_reckoning_nclt_2013_01_10_matrix.json),
aggregate:
[`experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json`](../experiments/results/imu_dead_reckoning_nclt_2013_01_10_matrix.json).

| Method | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| IMU-DR default (pure DR, midpoint, ZUPT off) | 9.071 | 170.617 | Repository default; zupt_frames=0. |
| IMU-DR + ZUPT | 2.887 | 30.607 | zupt_frames=481/1051 (~46%) -- this short window is mostly stationary, so the ATE/RPE gain is **not representative of continuous driving**; do not overclaim from this row. |
| IMU-DR forward Euler | 10.280 | 198.899 | Integration scheme is a second-order effect next to ZUPT/gyro-bias. |
| IMU-DR no gyro-bias | 24.676 | 482.028 | Largest degradation of any ablation on this window -- static-init gyro-bias estimation is the single most load-bearing component. |
| OdoNet | 138.872 | 1842.230 | KITTI-Raw-OXTS-trained CNN weights, out-of-domain here (NCLT's ms25 IMU vs. the KITTI OXTS sensor it was trained on). |
| NHC-Net | 4.295 | 102.938 | Same strapdown core plus a trained non-holonomic-constraint aid. |
| NN-ZUPT | 3.799 | 96.164 | Same strapdown core plus a trained ZUPT-detector aid. |

**Full session** (5105 frames, ~17 min / 1021.7 s, 1138.8 m). Manifest:
[`experiments/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json`](../experiments/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json),
aggregate:
[`experiments/results/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json`](../experiments/results/imu_dead_reckoning_nclt_2013_01_10_full_matrix.json).

| Method | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| IMU-DR default | 288700.449 | 67435.005 | zupt_frames=0; honest unaided gyro-drift compounding over ~17 min -- not a bug. |
| IMU-DR + ZUPT | 14531.743 | 2859.304 | -94.97%/-95.76% vs. default; zupt_frames=3984/48122 (~8.3%) -- rare resets truncating velocity-error accumulation, not "mostly stationary" as on the 120-frame window. |
| IMU-DR forward Euler | 291892.627 | 68484.744 | +1.11%/+1.56% vs. default -- still a second-order effect over the full session. |
| IMU-DR no gyro-bias | 672302.751 | 139392.868 | +132.9%/+106.7% vs. default -- confirms gyro-bias init as the most load-bearing aid at any timescale tested. |
| OdoNet | 1397.891 | 753.600 | |
| NHC-Net | 279.794 | 84.226 | |
| NN-ZUPT | 257.397 | 82.813 | |

> **Provenance note (full-session family JSON).** The full-session family
> run above (`full_family.json`, OdoNet/NHC-Net/NN-ZUPT) was generated and
> saved only on a now-defunct machine's external SSD (`/media/sasaki/aiueo`)
> and was **never committed to this repository** -- there is no
> `experiments/*_full_family*_matrix.json` in this repo, in git history, or
> anywhere referencing that filename except PLAN.md's prose. A search for it
> (2026-07-16) covered this Windows checkout, the `E:` external drive
> (`E:\datasets`, `E:\old`, `E:\tmp`, and a full recursive scan of `E:\`),
> and the WSL2 Ubuntu-22.04 clone at `/root/localization_zoo` -- no copy was
> found anywhere. The ATE/RPE values above are recorded numbers carried
> forward honestly from PLAN.md and the module README's committed prose,
> **not** a reconstructed or fabricated manifest. Re-export instructions for
> the underlying `pcd_dir` these three methods need are in the module
> README's
> [Limitations](../papers/imu_dead_reckoning/README.md#limitations--scope-notes)
> section; regenerating `full_family.json` itself additionally requires
> re-running `odonet` / `nhc_net` / `nn_zupt` against the re-exported
> `nclt_2013_01_10_full` fixture and the same GT CSV.

### KITTI Raw drive 2011_09_26_0009 (OXTS, ~9.7 Hz effective)

Second dataset. Manifests:
[`experiments/imu_dead_reckoning_kitti_raw_0009_matrix.json`](../experiments/imu_dead_reckoning_kitti_raw_0009_matrix.json)
(200-frame window),
[`experiments/imu_dead_reckoning_kitti_raw_0009_full_matrix.json`](../experiments/imu_dead_reckoning_kitti_raw_0009_full_matrix.json)
(full 443-frame sequence); aggregates:
[`experiments/results/imu_dead_reckoning_kitti_raw_0009_matrix.json`](../experiments/results/imu_dead_reckoning_kitti_raw_0009_matrix.json),
[`experiments/results/imu_dead_reckoning_kitti_raw_0009_full_matrix.json`](../experiments/results/imu_dead_reckoning_kitti_raw_0009_full_matrix.json).

**Unit-mismatch caveat**: the exporter's `imu.csv` stamps live on the same
synthetic per-frame positional-index timeline as `frame_timestamps.csv` (not
real seconds), so the strapdown integrator's `dt` is clamped to `max_dt=0.5 s`
per step vs. the real ~0.103 s inter-frame interval (~4.8x too large). The
2.0 s static-init window is similarly compressed to only ~3 real IMU samples.
**Static-init assumption is broken**: drive 0009 is already cruising at
~10.7 m/s at frame 0 and never drops below 1.339 m/s in the 200-frame window,
so the "static" init is taken while moving. **ZUPT fires almost entirely on
false positives** (200-frame: 174/199, ~87%; full: 344/442, ~78%, vs. an
actual stationary fraction of only ~9.7%) yet is still the single largest
improvement at both window sizes. **The gyro-bias ablation reverses vs.
NCLT**: disabling static-init gyro-bias estimation *improves* results here
(the opposite of NCLT, where it is the worst ablation), because the ~3-sample
static window is contaminated by real angular rate from the moving vehicle
rather than measuring pure sensor bias.

**200-frame window** (196 exported frames after the 2026-07-16 exporter fix
-- native indices 177-180 have no Velodyne scan and are dropped; 186.97 m):

| Method | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| IMU-DR default | 9195.203 | 7519.660 | zupt_frames=0. |
| IMU-DR + ZUPT | 210.224 | 172.596 | -97.71%/-97.70% vs. default; zupt_frames=170/195 (~87%) but OXTS speed never < 1.339 m/s here -- essentially all false positives, still the largest single gain. |
| IMU-DR no gyro-bias | 1170.370 | 1017.516 | -87.27%/-86.47% -- an **improvement**, reversed vs. NCLT (see caveat above). |
| OdoNet | 855.668 | 883.201 | In-domain sensor here (KITTI-OXTS-trained weights on a KITTI-OXTS drive). Pre-fix export value, not re-verified (see footnote). |
| NHC-Net | 121.881 | 99.144 | Pre-fix export value, not re-verified. |
| NN-ZUPT | 122.547 | 99.982 | Pre-fix export value, not re-verified. |

**Full sequence** (443 frames covering native indices {0..176, 181..446}
after the exporter fix; 332.34 m):

| Method | ATE (m) | RPE (%/100m) | Notes |
|---|---|---|---|
| IMU-DR default | 91851.969 | 49762.369 | zupt_frames=0. |
| IMU-DR + ZUPT | 5994.497 | 4432.717 | -93.47%/-91.09% vs. default; zupt_frames=344/442 (~78%) vs. ~9.7% actual stationary fraction. |
| IMU-DR no gyro-bias | 11184.985 | 5914.431 | -87.82%/-88.11% -- confirms the 200-frame window's reversal, not an artifact. |
| OdoNet | 1723.136 | 982.819 | Pre-fix export value, not re-verified (see footnote). |
| NHC-Net | 180.504 | 88.481 | Pre-fix export value, not re-verified. |
| NN-ZUPT | 186.050 | 93.636 | Pre-fix export value, not re-verified. |

All three learned-aid methods beat pure IMU-DR by 1-2 orders of magnitude on
this in-domain sensor (unlike the NCLT cross-sensor transfer, where OdoNet was
an honest negative). However, OdoNet/NHC-Net/NN-ZUPT were trained on KITTI Raw
OXTS data with `--val-drive 2011_09_26_drive_0056_sync` as the documented
held-out drive; whether drive 0009 itself was part of the original training
corpus **is not recorded**, so some train/eval overlap for these three methods
cannot be ruled out here -- recorded honestly rather than presented as a clean
held-out result. IMU-DR itself is never trained, so this caveat does not apply
to its rows.

> **Footnote (KITTI Raw exporter fix, landed 2026-07-16).** The
> frame-indexing bug in `kitti_raw_to_benchmark.py` (GT pose vs. point-cloud
> pairing skew from Velodyne frame position 177 onward on this drive) is
> fixed: the exporter now joins point clouds, GT, and `imu.csv` strictly on
> the native frame number parsed from filenames, dropping frames present in
> only one stream. The IMU-DR rows above are the post-fix numbers -- the fix
> changed `imu.csv`'s frame pairing (one 5-index gap instead of four 1-index
> steps), shifting every IMU-DR variant by a few percent, so the earlier
> "IMU-only rows are unaffected" claim was only half right (GT/OXTS *values*
> were unaffected; the pairing was not). The OdoNet/NHC-Net/NN-ZUPT rows are
> **pre-fix** values carried from the module README's recorded run and were
> not re-verified (no committed `kitti_raw_0009` JSON exists for them).
> Scan-matching manifests on this fixture family were re-run against the
> corrected export for litamin2, kiss_icp, xicp, ndt, gicp, small_gicp,
> voxel_gicp, vgicp_slam, suma, and ct_icp (partial aloam/balm2); the
> SLAM/mapping-heavy set still reflects pre-fix numbers -- see the module
> README's Limitations for the exact inventory.

The module README carries the full ablation surface for `imu_dead_reckoning`
beyond this family comparison -- RK4 integration, hard-aid stacking
(ZUPT+NHC+leveling), the opt-in ESKF (which beats the hand-tuned hard-aid
stack by ~90% ATE on NCLT full and is competitive with the trained CNN family
at zero training cost), and NEES covariance-consistency analysis. See
[`papers/imu_dead_reckoning/README.md`](../papers/imu_dead_reckoning/README.md)
for that detail; this section exists to keep the family-comparison numbers
visible at the paper-ready-reproducibility level rather than buried in a
module README.

## Frozen Evidence Bundle

The current manuscript-facing seed bundle is
[`docs/benchmarks/paper_ready_bundle.json`](benchmarks/paper_ready_bundle.json),
generated by:

```bash
python3 evaluation/scripts/build_paper_ready_bundle.py --check
```

It freezes eight methods for now: I-LOAM and KC-LO as T0 evidence candidates,
LiDAR-IBA and TrICP-LO as T1 evidence candidates, DegenSense and D2-LIO as
competitive no-IMU fallback evidence, plus M-GCLO and Quadric-LO as T1+
evidence. The bundle records the seq00/07 main result rows, paired-ablation
summaries where available, raw artifact paths, and relative reproduce commands.
It also points to the RF-LIO/ID-LIO synthetic dynamic-object stress, M-GCLO
synthetic non-flat ground stress, Quadric-LO synthetic curved-object stress,
M-GCLO KITTI seq08 public validation, M-GCLO MulRan ParkingLot public validation,
Quadric-LO KITTI seq02 public
validation, RF-LIO/ID-LIO KITTI seq05 public validation, LIO synchronized
LiDAR-IMU HDL-400 public validation, and NCLT 2013-01-10 public validation
summaries as supporting, non-paper-grade evidence. LiDAR-IBA and TrICP-LO now include
committed BA on/off and auto vs fixed-overlap ablations respectively. Dedicated
high-dynamic multi-beam benchmarks for RF-LIO/ID-LIO and orchard
multi-beam benchmarks for Quadric-LO remain open before a full
12-method table. MulRan ParkingLot is committed for M-GCLO under both no-gt-seed
(honest divergence) and GT-seed (mixed ground on/off under oracle init).

## Immediate Roadmap

1. **Freeze the core paper table** around 8-12 representative methods instead
   of all 101 methods.
2. **Promote only T0/T1 methods** into the main paper claim; keep T2/T3 in an
   appendix-style catalog.
3. **Add public dynamic/IMU validation** for RF-LIO/ID-LIO and the LIO
   compensation paths; HDL-400 open and NCLT 2013-01-10 IMU-on/off validation are
   committed for DegenSense, D2-LIO, ID-LIO, and RF-LIO (LiDAR-IBA IMU still
   unwired), while I-LOAM, KC-LO, M-GCLO, Quadric-LO, and TrICP-LO have seq00/07
   artifacts.
4. **Regenerate README and reproducibility report** from raw JSON artifacts
   where possible, rather than hand-editing values.
5. **Extend the frozen bundle** from the current 8-method seed to the final
   8-12 method manuscript table.

## README Policy

The README can advertise breadth, but paper-grade claims must be narrower:

- OK: "101 methods behind one API, with 42 no-code paper ports benchmarked under
  one KITTI protocol."
- OK: "The paper-ready subset is tracked separately with claim tiers and
  ablations."
- Not OK: implying every paper-named module is a faithful reproduction.
- Not OK: mixing adapter results with direct paper-result reproduction without
  tier labels.
