# Paper Draft: Results and Discussion

Working manuscript text for Sections 5 and 6 of
[`paper_draft_outline.md`](paper_draft_outline.md). Claims are scoped by
[`paper_claim.md`](paper_claim.md); every number below comes from a generated
asset named next to it, so re-running `export_paper_assets.py` and
`generate_paper_ratio_table.py` is the way to refresh them. Counts are as of
2026-10-10.

---

## 5. Results

### 5.1 Default variants do not transfer across datasets

Every method family keeps three or more concrete variants alive under one CLI
contract, and each benchmark problem elects its own default from them by the
shared promotion rule (`docs/decisions.md`). If one configuration were
universally best, every problem of a family would elect the same variant.
That does not happen for any family with broad coverage
(`docs/variant_analysis.md` §2, Table 3,
`docs/assets/paper/default_variant_matrix.csv`):

| Family | Windows with a ready problem | Distinct elected defaults |
|---|---:|---:|
| GICP | 14 | 2 |
| NDT | 17 | 5 |
| KISS-ICP | 19 | 5 |
| LiTAMIN2 | 37 | 11 |
| CT-ICP | 74 | 45 |

The CT-ICP count is inflated by its many single-sequence parameter sweeps on
KITTI Odometry, each of which elects a default by design. The cleaner
comparison is the twelve windows that LiTAMIN2, GICP, NDT, KISS-ICP, and CT-ICP
all share (Istanbul ×3, HDL-400 reference ×2, KITTI Raw ×4, MCD ×3). There,
LiTAMIN2 and CT-ICP each elect three distinct defaults and GICP, NDT, and
KISS-ICP two (Figure 4, `default_variant_instability.png`). Families with
narrower coverage (the LOAM variants, DLO/DLIO, MULLS, Small-GICP) are not
counted, because one or two windows cannot show instability.

### 5.2 The Pareto front on one shared sequence

Figure 1 (`docs/assets/paper/kitti07_pareto.png`) plots every pure-odometry
variant on full KITTI Odometry 07: 152 variants from 10 method families,
excluding GT-seeded runs, ablations, runs on corrected input scans, and
diverged runs. The front has four points: LeGO-LOAM's KITTI profile is the most
accurate (0.528 % 100 m RPE) but runs at 1 FPS on the stored host, while three
LiTAMIN2 variants trade a few hundredths of a percent for 70-107 FPS
(0.530 % at 71 FPS, 0.535 % at 80 FPS, 0.548 % at 107 FPS). No variant of the
eight other families reaches the front. A single-default repository would have
shown one point per family; the front only appears because the cheap variants
were kept.

Two caveats bound this figure. FPS comes from stored aggregates that predate
per-run host records (Table 8), so throughput is comparable within a family but
only indicative across families. And the front is drawn on one 1101-frame
sequence; Section 5.1 is the evidence that it moves on other data.

### 5.3 Pure odometry on KITTI Odometry

The README leaderboard ranks the non-GT-seeded best variant per cell on five
full sequences (00/02/05/07/08, 17 135 frames). Point-to-plane and LOAM-style
front ends cluster at 0.5-1.4 % 100 m RPE: LF-GICP is best on 00, 05, and 08,
LeGO-LOAM on 02 and 07, with A-LOAM, F-LOAM, and KISS-ICP within 0.1-0.2
points. SuMa, CT-ICP, L-LO, and MULLS trail at 1-4 %. GT-seeded registration
(NDT, LiTAMIN2 with seeding, GICP) is reported separately: a GT pose as the
per-frame initial guess measures seed adherence, not tracking, and NDT's
0.02 m seeded ATE becomes 87 % RPE without the seed.

### 5.4 Fidelity to the original papers (Table 6)

Table 6 (`docs/assets/paper/paper_ratio_table.{csv,tex}`) compares the
paper-reported KITTI translational error with the repository on the same full
sequences and the same official metric (100-800 m segments). Paper values are
read from the PDFs with table and row recorded
(`evaluation/data/paper_reported_numbers.json`). The repository value is the
best variant of each sweep, chosen on the evaluated sequence, so every ratio is
an optimistic bound; rows whose variant was chosen on sequence 07 and
transferred unchanged are marked held-out.

The methods split into two groups. A-LOAM (1.01x against LOAM values cited by
later papers), LF-GICP (0.96x), and LiTAMIN2 (1.22x) reach paper-level
accuracy. L-LO (1.58x), KISS-ICP (1.39x, sequence 00 only), SuMa (2.21x),
CT-ICP (4.03x), and MULLS (7.62x) run on the same metric but do not reproduce
the paper numbers. Section 6.5 traces part of the KISS-ICP gap to input
preprocessing and configuration.

### 5.5 Mechanism ablations

Paired ablations isolate single mechanisms on the same sequences:

- **LiDAR intensity (I-LOAM).** Turning reflectance weighting on cuts 100 m
  drift by 18.2 % on KITTI 00 and 19.7 % on KITTI 07 with mapping disabled
  (`docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json`).
- **Ground factors (M-GCLO).** Disabling them keeps RPE similar on hilly
  KITTI 08 but worsens ATE by 149 %.
- **Rare fallbacks matter (Quadric-LO).** The plane fallback fires rarely, yet
  disabling it on KITTI 02 worsens RPE by 55 % and ATE by 84 %.
- **Bundle adjustment (LiDAR-IBA).** It lowers ATE slightly but worsens RPE
  and throughput.

### 5.6 Place recognition

The loop-closure descriptors are scored separately from odometry, with top-1
retrieval against scans at least 50 frames older and a 4 m ground-truth
revisit radius (`docs/place_recognition_benchmark.md`,
`experiments/results/kitti_place_recognition.json`). Scan Context reaches
F1max 0.935 / 0.837 / 0.920 / 0.588 on KITTI 00 / 02 / 05 / 07, and the
intensity variant from ISC-LOAM, which shares its search, trails by
0.03-0.09. On KITTI 08, whose revisits run mostly in the opposite direction,
the two tie near 0.59 and recall at 100 % precision falls below 0.05. The
from-paper DTD port finds almost no correct loops (F1max ≤ 0.035); a direct
check shows its single-SVD verification without outlier rejection fails even
between scans 1 m apart. Like the odometry audits, this is a negative result
that only a real-data benchmark exposes: DTD's unit tests pass on synthetic
scenes.

---

## 6. Discussion

### 6.1 No universal default under broad coverage

Every family with broad window coverage elects more than one default
(Section 5.1). The practical consequence is that "the" configuration of a
method is a property of a method and a dataset together. A repository that
ships one default per method hides the variant that would have won on the
user's data and, as Figure 1 shows, can also hide a variant that is several
times faster at nearly the same accuracy.

### 6.2 Fast profiles are competitive

On KITTI 07, three LiTAMIN2 variants within 0.02 points of the most accurate
result run about 75-110 times faster than it (71-107 FPS against 0.95 FPS on
the stored hosts). The front is dense near the throughput end, which argues
for reporting at least one fast and one dense variant per method rather than a
single number.

### 6.3 Dataset dependency is the norm

Istanbul, the HDL-400 windows, MCD, KITTI Raw, and KITTI Odometry rank the same
methods differently. We treat this as the information variant-first
benchmarking is meant to expose, not as noise to average away; it is also the
reason the Pareto front of Section 5.2 is drawn on one named sequence instead
of an aggregate.

### 6.4 Paper-number audits catch silent errors

Pinning paper values to a table and row exposed two kinds of error that a
benchmark repository can carry unnoticed. First, wrong reference values: until
2026-10-03, every stored paper value for LiTAMIN2, CT-ICP, and KISS-ICP
disagreed with the paper, which had inflated LiTAMIN2's apparent gap (1.35x
claimed, 1.22x measured) and overstated CT-ICP's. Second, silent code drift: a
change to the shared KISS-ICP voxel search moved KITTI 00 from 0.857 % to
1.069 % 100 m RPE without any aggregate being re-run. A CI golden file over
every method's smoke-fixture ATE and RPE now fails on such drift.

### 6.5 Input preprocessing is part of the method

The KITTI Velodyne HDL-64E has a known intrinsic error that makes points
appear about 0.2 deg too low. Upstream KISS-ICP and CT-ICP undo it in their
KITTI loaders (+0.205 deg), so their published KITTI numbers are on corrected
scans, while every repository KITTI run used raw scans. Measuring the
correction separately (`docs/kitti_elevation_correction.md`) changes how the
Table 6 gaps read:

- **It explains most of the KISS-ICP gap.** The official KISS-ICP 1.3.0 code
  scores 0.528 % on corrected KITTI 00, matching the paper's 0.51 %, but
  0.910 % on raw scans, close to the repository's old 0.954 %. With the
  correction and the upstream configuration the repository reaches 0.708 %,
  so the KISS-ICP row falls from 1.87x to 1.39x. The rest is implementation:
  on raw scans the port matches or beats the official code on seq 00, yet it
  gains only 10-16 % from the correction against the official code's
  26-42 %. Raising the shared 80 m loader range to upstream's 100 m does not
  close that difference.
- **It is not a universal fix.** On KITTI 07 the same correction, applied to
  each method's Table 6 variant unchanged, lowers the official RTE of
  LiTAMIN2 by 29 % and KISS-ICP by 10-36 % but raises CT-ICP by 17 %,
  LF-GICP by 18 %, and SuMa by 44 %. Those variants were tuned on raw scans,
  and LF-GICP's paper explicitly used raw scans, so a worse corrected result
  says the tuning absorbed the sensor error, not that the correction is wrong.

Two rules follow. A paper-number comparison has to reproduce the paper's input
preprocessing, not only its metric, so corrected variants enter Table 6 only
for methods whose published pipeline uses them. A cross-method leaderboard has
to give every method the same input, so the README ranking and Figure 1 stay
on raw scans.

The same rerun exposed a second reproducibility limit. CT-ICP's Table 6
variant on KITTI 00 is deterministic on one host (two identical runs) but gives
3.446 % official RTE on a 4-core Xeon cloud container against the stored
1.664 % from an 8-thread i5-1145G7 with the same source, OS release, and
command. Single-host determinism is therefore not enough for long-sequence
CT-ICP numbers; they need the host recorded next to them, which the runner
now does.

### Limitations

- Families integrated after the twelve-window grid (LOAM variants, DLO/DLIO,
  MULLS, Small-GICP) cover fewer windows, so their cross-dataset stability is
  not yet comparable to the five core families.
- CT-LIO GT-backed evaluation remains blocked for lack of aligned open ground
  truth on the HDL-400 LIO window; the reference-based and public ROS1
  synthetic-time results are reported separately and are not exact
  native-time reproductions.
- Historical aggregates do not record their host, and at least one evidence
  row ran on a different machine; FPS is attributable only for runs recorded
  after per-run host provenance was added.
- Table 6 ratios use the best variant selected on the evaluated sequence, so
  they are optimistic bounds, not held-out estimates; and the variants were
  tuned on raw KITTI scans, which matters for methods whose papers used
  corrected scans (Section 6.5).
- Some long-sequence results depend on the host: CT-ICP on KITTI 00 differs
  by 2x in official RTE between two machines (Section 6.5).
- The multimodal KITTI extension covers four KITTI Raw windows with
  known-landmark reprojection inputs; it adds breadth but does not replace the
  LiDAR evidence.
