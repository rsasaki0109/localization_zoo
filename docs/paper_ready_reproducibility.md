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
| I-LOAM | T0 evidence candidate | Intensity weighting shows a controlled 18-20% drift reduction on KITTI with committed seq00/07 on/off raw artifacts. | Add it to the frozen paper bundle and keep the paired commands/artifacts locked. |
| KC-LO | T0 evidence candidate | Correspondence-free kernel correlation beats KISS-ICP on seq00/07, and the committed sigma-schedule ablation records the runtime/accuracy trade-off. | Add it to the frozen paper bundle and keep the annealed/fixed-sigma artifacts locked. |
| M-GCLO | T1 candidate | Multiple ground constraints produce the expected RPE/ATE trade-off. | Add ground-factor off ablation and one non-flat dataset check. |
| Quadric-LO | T1 candidate | Quadric residuals are competitive but expensive. | Add plane-fallback ablation and report fallback ratio by sequence. |
| LiDAR-visual adapters | T2 | They show pseudo-visual residuals are stable but auxiliary on KITTI PCD. | Do not call these paper-grade until real RGB / camera synchronization is used or the paper claim is reframed as a KITTI-PCD adapter study. |
| RF-LIO / ID-LIO | T1/T2 | Dynamic filtering mechanisms are active but degrade on mostly static KITTI. | Add a truly dynamic dataset or synthetic dynamic-object benchmark before making paper-level claims. |

## Immediate Roadmap

1. **Freeze the core paper table** around 8-12 representative methods instead
   of all 101 methods.
2. **Promote only T0/T1 methods** into the main paper claim; keep T2/T3 in an
   appendix-style catalog.
3. **Add paired ablations** for M-GCLO, Quadric-LO, and RF-LIO; I-LOAM and
   KC-LO now have committed seq00/07 paired artifacts.
4. **Regenerate README and reproducibility report** from raw JSON artifacts
   where possible, rather than hand-editing values.
5. **Add a `paper` benchmark bundle** that runs the frozen table and emits a
   single JSON manifest for the manuscript.

## README Policy

The README can advertise breadth, but paper-grade claims must be narrower:

- OK: "101 methods behind one API, with 42 no-code paper ports benchmarked under
  one KITTI protocol."
- OK: "The paper-ready subset is tracked separately with claim tiers and
  ablations."
- Not OK: implying every paper-named module is a faithful reproduction.
- Not OK: mixing adapter results with direct paper-result reproduction without
  tier labels.
