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
| DegenSense | T1 evidence candidate | Degeneracy sensing is competitive as a no-IMU KITTI fallback once compensation is gated to real IMU packets. | Validate the IMU compensation path on a synchronized LiDAR-IMU benchmark before making full LIO claims. |
| D2-LIO | T1 evidence candidate | Directional degeneracy diagnostics are competitive as a no-IMU KITTI fallback once the IMU prior is gated to real IMU packets. | Validate directional IMU regularization on a synchronized LiDAR-IMU benchmark before making full LIO claims. |
| M-GCLO | T1+ evidence candidate | Ground-factor off ablation is committed on KITTI, and synthetic rolling-ground stress shows ground on improves ATE/RPE versus ground off. | Add public non-flat dataset validation before promoting to T0. |
| Quadric-LO | T1+ evidence candidate | Plane-fallback ablation is committed on KITTI seq00/07, synthetic curved-object stress confirms quadric-path dominance, and public residential KITTI seq02 shows >99% quadric correspondences though rare plane fallback carries more weight than on highway sequences. | Add dedicated orchard or non-urban multi-beam benchmarks before promoting to T0. |
| LiDAR-IBA | T1 evidence candidate | Stereographic plane front-end is competitive on KITTI; committed no-BA profile beats BA-on for RPE on IMU-free seq00/07 with paired ablation JSON. | Validate BA + IMU on a synchronized LiDAR-IMU benchmark before full LIO claims. |
| TrICP-LO | T1 evidence candidate | LTS trimmed point-to-plane odometry is near KISS-ICP on seq00/07; paired overlap ablation shows FRMSD auto sticks to ξ=0.800 on clean KITTI. | Add high-outlier/non-overlap stress before promoting FRMSD overlap claims beyond clean KITTI. |
| LiDAR-visual adapters | T2 | They show pseudo-visual residuals are stable but auxiliary on KITTI PCD. | Do not call these paper-grade until real RGB / camera synchronization is used or the paper claim is reframed as a KITTI-PCD adapter study. |
| RF-LIO / ID-LIO | T1/T2 | Dynamic filtering mechanisms are active on KITTI and in the committed synthetic dynamic-object stress. RF-LIO reduces the synthetic failure severity versus ID-LIO, but both remain far from the KISS sanity reference. | Add a public high-dynamic dataset before making manuscript-level dynamic-scene claims. |

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
synthetic non-flat ground stress, and Quadric-LO synthetic curved-object stress
summaries as supporting, non-paper-grade evidence. LiDAR-IBA and TrICP-LO now
include committed BA on/off and auto vs fixed-overlap ablations respectively.
RF-LIO/ID-LIO public high-dynamic validation, M-GCLO public non-flat validation,
and Quadric-LO public curved-object/non-urban validation remain open before a
full 12-method table.

## Immediate Roadmap

1. **Freeze the core paper table** around 8-12 representative methods instead
   of all 101 methods.
2. **Promote only T0/T1 methods** into the main paper claim; keep T2/T3 in an
   appendix-style catalog.
3. **Add public dynamic/IMU validation** for RF-LIO/ID-LIO and the LIO
   compensation paths; the synthetic stress is committed, while I-LOAM, KC-LO,
   DegenSense, D2-LIO, M-GCLO, Quadric-LO, LiDAR-IBA, and TrICP-LO now have seq00/07 artifacts.
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
