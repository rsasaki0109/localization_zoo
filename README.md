<div align="center">
  <h1 align="center">Localization Zoo</h1>
  <p align="center">
    <b>C++ implementations, derived variants, and compact baselines for localization papers</b>
  </p>
  <p align="center">
    <b>101 methods</b> · <b>73 paper reimplementations</b> · <b>42 papers with no public author code</b> · one C++ API · honest KITTI benchmarks
  </p>
  <p align="center">
    <img src="https://img.shields.io/badge/C%2B%2B-17-blue" alt="C++17">
    <img src="https://img.shields.io/badge/ROS2-Humble-green" alt="ROS2 Humble">
    <img src="https://img.shields.io/badge/Scope-Reimpl%20%2B%20Derived-orange" alt="Reimplementations and Derived Variants">
    <img src="https://img.shields.io/badge/Tests-CTest-brightgreen" alt="CTest">
    <img src="https://img.shields.io/badge/License-MIT-yellow" alt="MIT License">
  </p>
  <p align="center">
    <a href="https://rsasaki0109.github.io/localization_zoo/">
      <img src="docs/assets/hero_seq00.gif" alt="KITTI seq00 top-down trajectories: KC-LO, KISS-ICP, TrICP-LO vs ground truth, no GT seed" width="900">
    </a>
  </p>
  <p align="center">
    <sub>KITTI seq00 odometry, no GT seed — paper reimplementations tracked against ground truth.</sub>
  </p>
  <p align="center">
    <a href="https://rsasaki0109.github.io/localization_zoo/"><b>↪ Open the interactive benchmark and method explorer</b></a>
  </p>
</div>

**Try it in one command** — no build, no dataset; runs the synthetic benchmark plus
committed real-data fixtures and drops `report.html` into `./zoo-demo/`:

```bash
docker run --rm -v "$PWD/zoo-demo:/out" ghcr.io/rsasaki0109/localization_zoo:latest
```

---

## Why Localization Zoo?

Many localization papers do not ship reusable reference implementations.
This repository collects paper-oriented C++ implementations behind a unified
API, with ROS 2 nodes and evaluation tools that are ready to run. The next
research track is a stricter paper-ready subset with claim tiers and ablations:
[paper-ready reproducibility plan](docs/paper_ready_reproducibility.md).

- **Pure C++**: ROS-independent core libraries for research, education, and embedded use
- **ROS 2 Humble**: Ready-to-run nodes via `ros2 run`
- **Built-in benchmarking**: Compare methods immediately after build
- **Degeneracy-aware methods**: Includes RELEAD and X-ICP for constrained environments
- **Claim tiers**: Separate faithful reproduction, mechanism evidence, adapters, and compact baselines

## Paper-Ready Claim Boundary

Localization Zoo is intentionally broad, but manuscript-level claims should use
a narrower evidence set:

- **Breadth catalog**: all 101 methods share the same API, tests, and benchmark
  harness; this includes paper reimplementations, derived variants, hybrids, and
  compact baselines.
- **Paper-ready subset**: only methods that satisfy the tier checklist and
  paired-ablation requirements in
  [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md)
  should be treated as reproduction evidence. The current frozen evidence
  manifest is
  [`docs/benchmarks/paper_ready_bundle.json`](docs/benchmarks/paper_ready_bundle.json).
- **Strongest current candidates**: I-LOAM, KC-LO, M-GCLO, and Quadric-LO.
  LiDAR-visual adapters and dynamic-filtering LIO ports remain
  adapter/mechanism evidence until real RGB, synchronized camera, IMU, or
  dynamic-scene gaps are closed.

<!-- LEADERBOARD:START -->
## Leaderboard — odometry, RPE [drift %/100 m], lower is better

KITTI Odometry full sequences, ranked by relative pose error (the
seed-independent local-accuracy metric). ATE in parens is unbounded drift over
the run. Full matrix: [**explorer**](https://rsasaki0109.github.io/localization_zoo/).

| Method | Seq 00 | Seq 02 | Seq 05 | Seq 07 | Seq 08 |
|---|---:|---:|---:|---:|---:|
|  | _4542 fr_ | _4661 fr_ | _2761 fr_ | _1101 fr_ | _4071 fr_ |
| LeGO-LOAM | **0.84%** <sub>(12 m)</sub> | **0.88%** <sub>(42 m)</sub> | 0.52% <sub>(5 m)</sub> | **0.53%** <sub>(3 m)</sub> | 1.37% <sub>(19 m)</sub> |
| A-LOAM | 0.90% <sub>(19 m)</sub> | 0.93% <sub>(51 m)</sub> | **0.51%** <sub>(5 m)</sub> | 0.61% <sub>(3 m)</sub> | 1.39% <sub>(19 m)</sub> |
| KISS-ICP | 0.86% <sub>(21 m)</sub> | 0.94% <sub>(39 m)</sub> | 0.62% <sub>(6 m)</sub> | 0.61% <sub>(2 m)</sub> | **1.34%** <sub>(19 m)</sub> |
| F-LOAM | 0.99% <sub>(9 m)</sub> | 0.95% <sub>(54 m)</sub> | 0.54% <sub>(6 m)</sub> | 0.59% <sub>(3 m)</sub> | 1.37% <sub>(17 m)</sub> |
| CT-ICP | 1.97% <sub>(19 m)</sub> | 2.64% <sub>(67 m)</sub> | 1.09% <sub>(11 m)</sub> | 1.17% <sub>(3 m)</sub> | 1.91% <sub>(31 m)</sub> |
| MULLS | – | – | – | 2.64% <sub>(8 m)</sub> | – |

_Best variant per cell ([`docs/experiments.md`](docs/experiments.md)). KISS-ICP /
LOAM ~0.5–1.4% drift is competitive — their large ATE is honest drift, not a
broken port._

> **No GT-seeded methods here.** NDT / LiTAMIN2 / GICP use the ground-truth pose
> as the per-frame initial guess, so their ATE is seed adherence, not tracking
> (NDT `--no-gt-seed` → 87% RPE). They need a GT prior and aren't ranked.

### From-paper reimplementations (no public reference code) — KITTI full

> **📋 [Reproducibility Report](docs/reproducibility_report.md)** — what
> happens when you reimplement 42 papers that ship no code: which claims
> reproduce, which mechanisms go silent on KITTI, which diverge, and the
> recurring implementation lessons.

Papers with **no public author code**, run as pure odometry on KITTI full
sequences (first-pose anchor, `--no-gt-seed`, dense profile unless a method README
calls out a CPU profile; no IMU, so LIO methods use constant-velocity fallback).
RPE is drift %/100 m; ATE in parens.

| Method | Seq 00 _(4541 fr)_ | Seq 07 _(1101 fr)_ | Paper |
|---|---:|---:|---|
| M-GCLO | **0.835%** <sub>(19 m)</sub> | 0.671% <sub>(2 m)</sub> | ISPRS Ann. 2024 |
| KC-LO | 0.837% <sub>(13 m)</sub> | **0.510%** <sub>(1 m)</sub> | ECCV 2004 |
| **I-LOAM** | **0.899%** <sub>(13 m)</sub> | **0.575%** <sub>(2 m)</sub> | UR 2020 |
| LODESTAR | 0.848% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2511.09142 |
| Terrain-RBF-LIO | 0.849% <sub>(8 m)</sub> | 0.587% <sub>(1 m)</sub> | arXiv:2509.26222 |
| DALI-SLAM | 0.849% <sub>(8 m)</sub> | 0.600% <sub>(1 m)</sub> | ISPRS JPRS 2025 |
| DAMM-LOAM | 0.851% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2510.13287 |
| CUBE-LIO | 0.851% <sub>(9 m)</sub> | 0.608% <sub>(1 m)</sub> | ROBOMECH 2026 |
| Intensity-Flow | 0.856% <sub>(8 m)</sub> | 0.616% <sub>(1 m)</sub> | Measurement 2026 |
| Quadric-LO | 0.867% <sub>(15 m)</sub> | 0.598% <sub>(2 m)</sub> | arXiv:2304.14190 |
| Adaptive-ICP | 0.870% <sub>(11 m)</sub> | 0.569% <sub>(1 m)</sub> | arXiv:2509.22058 |
| MCC-LO | 0.892% <sub>(13 m)</sub> | 0.611% <sub>(2 m)</sub> | PLOS ONE 2018 |
| Mesh-LOAM | 0.901% <sub>(13 m)</sub> | 0.616% <sub>(1 m)</sub> | IEEE T-IV 2024 |
| NHC-LIO | 0.902% <sub>(18 m)</sub> | 0.608% <sub>(3 m)</sub> | IEEE Sens. J. 2023 |
| SVN-ICP | 0.912% <sub>(14 m)</sub> | 0.607% <sub>(3 m)</sub> | arXiv:2509.08069 |
| ICPSC-LO | 0.912% <sub>(19 m)</sub> | 0.660% <sub>(4 m)</sub> | JAG 2023 |
| TrICP-LO | 0.931% <sub>(10 m)</sub> | 0.662% <sub>(2 m)</sub> | IVC 2005 |
| GMM-LO | 0.941% <sub>(14 m)</sub> | 0.657% <sub>(1 m)</sub> | arXiv:1807.02587 |
| MCGICP-LO | 0.940% <sub>(20 m)</sub> | 0.774% <sub>(5 m)</sub> | RAS 2017 |
| Student-T-LO | 0.952% <sub>(15 m)</sub> | 0.696% <sub>(2 m)</sub> | PMC11314997 2024 |
| Small-but-Mighty | 0.961% <sub>(15 m)</sub> | 0.897% <sub>(3 m)</sub> | Remote Sens. 2025 |
| GNC-LO | 0.986% <sub>(18 m)</sub> | 0.722% <sub>(2 m)</sub> | arXiv:1909.08605 |
| IMLS-SLAM | 1.000% <sub>(18 m)</sub> | 0.700% <sub>(3 m)</sub> | ICRA 2018 |
| CT-VoxelMap | 1.046% <sub>(21 m)</sub> | 0.800% <sub>(3 m)</sub> | arXiv:2604.03747 |
| OPL-LVIO | 1.050% <sub>(15 m)</sub> | 0.902% <sub>(4 m)</sub> | Remote Sens. 2022 |
| AD-VLO | 1.052% <sub>(13 m)</sub> | 0.939% <sub>(3 m)</sub> | ICRA 2019 |
| TC-MVLO | 1.054% <sub>(11 m)</sub> | 0.939% <sub>(3 m)</sub> | ISR 2022 |
| TC-LVGF | 1.055% <sub>(12 m)</sub> | 0.941% <sub>(4 m)</sub> | ROBIO 2023 |
| TC-VLO | 1.060% <sub>(12 m)</sub> | 0.925% <sub>(4 m)</sub> | IV 2019 |
| BIEVR-LIO | 1.063% <sub>(25 m)</sub> | 0.873% <sub>(4 m)</sub> | arXiv:2604.14421 |
| V-LOAM2015 | 1.066% <sub>(14 m)</sub> | 0.910% <sub>(4 m)</sub> | ICRA 2015 |
| R-VoxelMap | 1.076% <sub>(20 m)</sub> | _diverges_ | arXiv:2601.12377 |
| Vibration-LIO | 1.082% <sub>(15 m)</sub> | 0.781% <sub>(3 m)</sub> | arXiv:2507.04311 |
| ID-LIO | 1.111% <sub>(15 m)</sub> | 0.999% <sub>(5 m)</sub> | Sensors 2023 |
| ELO | 1.124% <sub>(23 m)</sub> | 0.981% <sub>(4 m)</sub> | IEEE RA-L 2021 |
| PCR-DAT | 1.239% <sub>(11 m)</sub> | 1.040% <sub>(4 m)</sub> | ISR 2024 |
| RF-LIO | 1.351% <sub>(23 m)</sub> | 1.272% <sub>(5 m)</sub> | IROS 2021 |
| LiDAR-IBA | 2.001% <sub>(8 m)</sub> | 1.474% <sub>(1 m)</sub> | arXiv:2602.06380 |
| D2-LIO | 5.794% <sub>(106 m)</sub> | 0.804% <sub>(2 m)</sub> | arXiv:2508.14355 |
| DegenSense | 9.931% <sub>(417 m)</sub> | 9.940% <sub>(39 m)</sub> | arXiv:2412.07513 |
| Spectral-LO | 12.029% <sub>(113 m)</sub> | 13.671% <sub>(47 m)</sub> | arXiv:2005.02042 |
| DiLO | 18.305% <sub>(226 m)</sub> | 18.966% <sub>(159 m)</sub> | ETRI J. 2021 |
| **PL-LOAM** | **143.211%** <sub>(3016 m)</sub> | **116.899%** <sub>(271 m)</sub> | ICRA 2020 |
| **VLOM** | **91.465%** <sub>(249 m)</sub> | **153.868%** <sub>(439 m)</sub> | arXiv:2304.08978 |
| **InTEn-LOAM** | **52.695%** <sub>(1459 m)</sub> | **67.497%** <sub>(448 m)</sub> | RS 2022/23 |
| UA-LIO | _diverges_ | _diverges_ | IEEE TIM 2025 |
| _KISS-ICP (same profile, ref)_ | _0.872%_ <sub>(12 m)</sub> | _0.618%_ <sub>(2 m)</sub> | — |
| _CT-ICP (same profile, ref)_ | _2.577%_ <sub>(17 m)</sub> | _2.500%_ <sub>(4 m)</sub> | — |

The top ten (M-GCLO through Adaptive-ICP) **match or beat KISS-ICP on seq-00**,
and all but M-GCLO (0.671% seq-07) also beat it on **seq-07** — well clear of
CT-ICP. **M-GCLO** leads seq-00 drift (0.835%) via
multiple-ground-plane constraints (higher ATE — an honest RPE/ATE split). Its
ground-factor ablation keeps translational RPE similar or lower, but disabling
ground more than doubles ATE on seq00/07 and worsens rotational drift; the
paired raw artifacts are committed as
[`m_gclo_ground_factor_ablation.json`](docs/benchmarks/kitti_full_new_methods/m_gclo_ground_factor_ablation.json).
**KC-LO** (correspondence-free kernel correlation) leads seq-07 drift (0.510%)
and beats KISS-ICP on both sequences — at a heavy throughput cost
(~2.6-3.1 FPS for the fixed-sigma profile; ~1.4 FPS with coarse-to-fine
annealing). Its sigma-schedule ablation is committed as
[`kc_lo_sigma_schedule_ablation.json`](docs/benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json).

Recurring honest finding: on geometry-rich, IMU-free KITTI most robust/soft
mechanisms go near-redundant and the front-end reduces to a ~KISS-ICP
point-to-plane core. The newer LiDAR-visual adapter family
(**OPL-LVIO**, **AD-VLO**, **TC-MVLO**, **TC-LVGF**, **TC-VLO**, **V-LOAM2015**)
is stable at ~0.90–1.07% drift and far better than pseudo-image visual front
ends; AD-VLO/TC-MVLO improve ATE within the group, while OPL-LVIO keeps the best
seq07 RPE. Still, pseudo-visual residuals remain secondary to the
point-to-plane core. **RF-LIO** confirms the same KITTI pattern for dynamic
removal: its removal-first range-image filter is active, but on mostly static
KITTI it removes useful foreground structure and trails ID-LIO. Honest negatives:
DiLO (direct, 18–19% drift), Spectral-LO
(ICP-free BEV phase-correlation, fastest at ~14 FPS but coarse ~12–14%),
**PL-LOAM** (LiDAR-visual point+line on pseudo-image without RGB, ~117–143% drift),
**VLOM** (scale-corrected visual bootstrap A-LOAM on pseudo-image, ~91–154% drift),
**InTEn-LOAM** (cylindrical intensity LO without DOR/mapping, ~53–67% drift),
R-VoxelMap (diverges seq 07), UA-LIO/DegenSense. Per-method caveats live in the
module READMEs; raw JSON:
[`docs/benchmarks/kitti_full_new_methods/`](docs/benchmarks/kitti_full_new_methods/).
<!-- LEADERBOARD:END -->

### Does LiDAR intensity actually help? — I-LOAM ablation

[**I-LOAM**](papers/i_loam/) (Intensity Enhanced LOAM, UR 2020 — no public author
code) injects the LiDAR reflectance channel into LOAM's scan-to-scan association
(intensity-augmented correspondence + intensity-similarity residual weighting).
Running the *identical* pipeline with the intensity paths on vs. off
(`--i-loam-no-intensity`) **and mapping disabled** (`--i-loam-no-mapping`)
isolates exactly what reflectance contributes on KITTI:

| Sequence | Geometric baseline (intensity off) | I-LOAM (intensity on) | Δ drift |
|---|---:|---:|---:|
| Seq 00 | 3.186% <sub>(76.0 m)</sub> | **2.606%** <sub>(49.4 m)</sub> | **−18.2%** |
| Seq 07 | 3.806% <sub>(18.5 m)</sub> | **3.055%** <sub>(15.1 m)</sub> | **−19.7%** |

Reflectance consistently cuts drift ~18–20% (and ATE up to ~35%) versus the same
geometry-only pipeline — I-LOAM's central claim **reproduces on KITTI**, even
though KITTI intensity is uncalibrated and coarse. With **mapping enabled**
(default), I-LOAM reaches ~0.58% drift on seq07 and is ranked in the from-paper
table above; the ablation below uses scan-to-scan only to isolate the intensity
channel. Raw artifacts:
[`seq00 on`](docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json),
[`seq00 off`](docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json),
[`seq07 on`](docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json),
[`seq07 off`](docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json),
and the paired summary
[`i_loam_intensity_ablation.json`](docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json).

### Trajectory gallery — KITTI seq07, all methods, one figure

Top-down trajectories on KITTI seq07 (1101 frames, `--no-gt-seed`), each path
**colored by its distance from ground truth** (cool = on-track, warm = drifting,
clipped at 3 m) on a shared scale and ordered by RPE drift. This surfaces *where*
each method drifts — the top tier all hug GT, so the raw paths look identical;
the color reveals the difference. **KC-LO** leads; NDT / SuMa run fully warm —
honest no-GT-seed failures, not cropping. Regenerate with
[`evaluation/scripts/plot_trajectory_diff.py`](evaluation/scripts/plot_trajectory_diff.py).

<p align="center">
  <img src="docs/assets/grid_seq07.png" alt="KITTI seq07 trajectory gallery: 15 localization methods, each path colored by distance from ground truth, no GT seed, ordered by RPE drift" width="960">
</p>

## Scope Note

This repository mixes three levels of implementation scope:

- **Paper reimplementation**: intended to track the published method closely
- **Derived variant**: built from the paper idea, but adapted to this repository's shared components
- **Compact baseline**: a smaller approximation that keeps the interface and core intuition, but not the full paper pipeline

Methods already labeled `Derived` or `Hybrid` are intentionally adapted versions.
Some paper-named entries still use compact or simplified internals today, especially `NDT`, `KISS-ICP`, and `DLIO`. Check each method README for current scope and deviations.
The tracked claim boundary for original-paper reproduction is summarized in
[`docs/reproduction_status.md`](docs/reproduction_status.md), and the stricter
paper-ready promotion criteria live in
[`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md).
Benchmark usefulness and paper-result reproducibility are not treated as the
same thing in this repository.

---

## Experiment-Driven Development

A small stable benchmark core plus a discardable experiment layer. Search state is
externalized in docs ([experiments](docs/experiments.md), [decisions](docs/decisions.md),
[interfaces](docs/interfaces.md), [reproduction status](docs/reproduction_status.md));
variants live under [`experiments/`](experiments/) with a matrix runner over Istanbul,
HDL-400, MCD, and KITTI windows.

### Quick checks (after clone)

```bash
# build + synthetic benchmark + broad real-data fixture suite
bash evaluation/scripts/demo_localization_zoo.sh
```

It writes a `report.html` + `manifest.json` and JSON summaries under
`experiments/results/runs/demo_localization_zoo/`.

CI-equivalent smoke coverage (a committed ~3 MB MCD fixture, also run in GitHub Actions
after `ctest`):

```bash
bash evaluation/scripts/smoke_ci_fixture.sh
bash evaluation/scripts/smoke_multimodal_fixture.sh
```

Refreshing the experiment/publication docs and the matrix runner is documented in
[`evaluation/README.md`](evaluation/README.md).

---

## Benchmark

### Real LiDAR Data

One real-data snapshot from the Autoware Istanbul localization bag (last full
multi-method run; speed-oriented profile, GT-seeded scan-to-map methods fall back to
the seed pose on weak updates). GitHub Pages serves the latest from
[`docs/benchmarks/latest/results.json`](docs/benchmarks/latest/results.json).

- Topic: `/localization/util/downsample/pointcloud`
- Window: frames `10200-10307`
- Sequence length: `108` frames, `302.1 m`, `10.70 s`
- Reference poses: `reference_pose_full.csv`
- Scan density: `940-1380` points per frame, `1128.7` average

| Method | Status | ATE [m] | FPS | Notes |
|--------|--------|---------|-----|-------|
| NDT | OK | 0.109 | 1.2 | GT-seeded scan-to-map init with weak-update fallback; current snapshot uses a recent/local-map profile |
| LiTAMIN2 | OK | 1.213 | 21.0 | GT-seeded scan-to-map init with weak-update fallback; current snapshot uses a recent/local-map profile plus OpenMP parallelism |
| GICP | OK | 0.994 | 3.1 | GT-seeded scan-to-map init with weak-update fallback; current snapshot uses a recent/local-map profile |
| CT-ICP | OK | 75.075 | 1.3 | Odometry-only; ATE is measured after anchoring to the first GT pose |
| KISS-ICP | OK | 183.178 | 3.2 | Odometry-only; ATE is measured after anchoring to the first GT pose |
| CT-LIO | SKIPPED | - | - | The bag window does not contain IMU data, so `imu.csv` was not generated |

![Autoware Istanbul benchmark](docs/benchmarks/latest/trajectory.png)

`./pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] --methods <selector> --summary-json <path>`
evaluates sequential PCD datasets against a shared trajectory CSV. The selector list
and per-method flag families are generated in [`docs/interfaces.md`](docs/interfaces.md).

Dataset-prep helpers (KITTI Odometry, KITTI Raw, ROS 1/2 bag extraction, HDL-400) live
under [`evaluation/scripts/`](evaluation/scripts/). For KITTI Odometry public sequences:

```bash
python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
  --kitti-root /path/to/data_odometry --sequence 00 --sequence 07 \
  --window-size 108 --include-full
```

### Synthetic Urban (30 frames)

```
Method         ATE [m]     FPS
─────────────────────────────────
CT-ICP         0.124       0.1   << best accuracy
A-LOAM         2.059       4.6   << balanced baseline
X-ICP          16.634      5.9
LiTAMIN2       31.155      2.6
```

> Reproduce with `./synthetic_benchmark`. No external dataset is required.

### 2D Laser Scan Odometry

Eight planar matchers (papers 43–50) plus **Karto-Matcher** (map-based CSM extension) in [`scan_dogfooding`](evaluation/src/scan_dogfooding.cpp).
Full leaderboard, fixtures, and reproduction steps:
[**docs/benchmarks/scan2d/README.md**](docs/benchmarks/scan2d/README.md).

| Method | Intel val | fr079 val | MIT val | Synth corridor |
|--------|---:|---:|---:|---:|
| | _73 fr / 378 m_ | _384 fr / 373 m_ | _33 fr / 267 m_ | _120 fr / 9.5 m_ |
| **RF2O** | **14.3%** | 15.4% | 27.6% | 1.3% |
| **Karto-Matcher** | 14.0% | 13.7% | 28.1% | 30.5% |
| **CSM** | 14.0% | 13.7% | 28.1% | 30.5% |
| **NDT-2D** | 14.9% | **14.4%** | 27.8% | **0.8%** |
| **IDC** | 15.3% | 27.7% | 29.5% | 42.6% |
| **MbICP** | 14.5% | 15.4% | 27.5% | **0.05%** |
| **PL-ICP** | 15.0% | 14.1% | 27.2% | **0.01%** |
| **Kinematic-ICP** | 18.4% | 18.9% | **23.4%** | 83.8% |
| **PSM** | 21.8% | **13.9%** | 27.9% | 11.6% |

```bash
cmake --build build --target scan_dogfooding
bash evaluation/scripts/run_scan2d_benchmark.sh
```

Setup: [`evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md`](evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md).

---

## Implementations

### Point Cloud Registration

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[LiTAMIN2](papers/litamin2/)** | ICRA 2021 | KL-divergence ICP with aggressive point reduction for faster registration | [arXiv](https://arxiv.org/abs/2103.00784) |
| **[GICP](papers/gicp/)** | RSS 2009 | Plane-to-plane ICP with local covariance modeling and Mahalanobis distance | [Paper](https://www.roboticsproceedings.org/rss05/p31.html) |
| **[Voxel-GICP](papers/voxel_gicp/)** | RA-L 2021 | GICP accelerated with voxel representatives and voxel-level covariance | [Paper](https://arxiv.org/abs/2109.07082) |
| **[small_gicp](papers/small_gicp/)** | Derived | Compact GICP with voxel downsampling and capped correspondences | [GitHub](https://github.com/koide3/small_gicp) |
| **[VGICP-SLAM](papers/vgicp_slam/)** | Derived | Voxel-GICP front-end with Scan Context and loop-graph back-end | - |
| **[NDT](papers/ndt/)** | IROS 2003 | NDT-style registration against voxel Gaussian models with a compact optimizer | [Paper](https://ieeexplore.ieee.org/document/1249285) |
| **[KISS-ICP](papers/kiss_icp/)** | RA-L 2023 | Compact KISS-ICP-style pipeline with voxel hashing, adaptive thresholds, and robust ICP | [Paper](https://arxiv.org/abs/2209.15397) |
| **[A-LOAM](papers/aloam/)** | RSS 2014 | Curvature-based feature extraction with a three-stage odom-to-map pipeline | [GitHub (ROS1)](https://github.com/HKUST-Aerial-Robotics/A-LOAM) |
| **[F-LOAM](papers/floam/)** | Derived | Lightweight LOAM pipeline with input thinning and sparse map updates | - |
| **[ISC-LOAM](papers/isc_loam/)** | Derived | Lightweight LOAM with intensity descriptors and F-LOAM/GICP loop graph | - |
| **[LOAM Livox](papers/loam_livox/)** | Derived | LOAM variant for solid-state LiDAR using pseudo scan lines from azimuth sectors | [Reference](https://github.com/hku-mars/loam_livox) |
| **[LeGO-LOAM](papers/lego_loam/)** | IROS 2018 | Ground-aware feature extraction for vehicle-oriented LOAM | [Paper](https://ieeexplore.ieee.org/document/8594299) |
| **[MULLS](papers/mulls/)** | Derived | Multi-metric scan-to-map alignment using edge, plane, and point residuals | - |
| **[BALM2](papers/balm2/)** | T-RO 2022 | Local bundle adjustment over recent keyframes with line and plane residuals | [arXiv](https://arxiv.org/abs/2209.08854) |
| **[SuMa](papers/suma/)** | RSS 2018 | Dense surfel-based point-to-plane odometry from range-image maps | [GitHub](https://github.com/jbehley/SuMa) |
| **[DLO](papers/dlo/)** | Derived | Direct keyframe odometry that aligns dense scans to a local GICP map | [GitHub](https://github.com/vectr-ucla/direct_lidar_odometry) |
| **[HDL Graph SLAM](papers/hdl_graph_slam/)** | Derived | NDT front-end with floor priors, Scan Context, and GICP loop closures | [GitHub](https://github.com/koide3/hdl_graph_slam) |
| **[CT-ICP](papers/ct_icp/)** | ICRA 2022 | Continuous-time registration with two poses per frame and SLERP motion compensation | [GitHub (ROS1)](https://github.com/jedeschaud/ct_icp) |
| **[X-ICP](papers/xicp/)** | T-RO 2024 | Constrained ICP using Hessian SVD to classify observable directions | [arXiv](https://arxiv.org/abs/2211.16335) |

### Degeneracy-Aware Methods

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[RELEAD](papers/relead/)** | ICRA 2024 | Constrained ESIKF with projection-based suppression along degenerate directions | [arXiv](https://arxiv.org/abs/2402.18934) |
| **[CT-ICP + RELEAD](papers/ct_icp_relead/)** | Hybrid | Continuous-time CT-ICP interpolation combined with RELEAD degeneracy handling | - |

### Foundations

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[IMU Preintegration](papers/imu_preintegration/)** | T-RO 2017 | Preintegration on SO(3) with first-order bias correction for LIO pipelines | [Paper](https://arxiv.org/abs/1512.02363) |

### LIO / Continuous-Time Fusion

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[CT-LIO](papers/ct_lio/)** | Hybrid | Lightweight LIO that combines CT-ICP interpolation with IMU preintegration constraints | [CLINS](https://arxiv.org/abs/2109.04687) |
| **[DLIO](papers/dlio/)** | ICRA 2023 | Compact DLIO-style LIO built on DLO-style registration plus IMU preintegration | [GitHub](https://github.com/vectr-ucla/direct_lidar_inertial_odometry) |
| **[LINS](papers/lins/)** | Derived | Lightweight LiDAR-inertial estimator with iterated filtering and point-to-plane updates | - |
| **[Point-LIO](papers/point_lio/)** | Derived | Compact direct LIO with raw-point planarity correspondences and iterated filtering | - |
| **[CLINS](papers/clins/)** | Derived | Sequence pipeline version of CT-LIO-style continuous-time registration | - |
| **[VILENS](papers/vilens/)** | Derived | Compact visual-lidar-inertial smoother with Point-LIO-style local maps and reprojection fusion | - |
| **[LIO-SAM](papers/lio_sam/)** | IROS 2020 | Lightweight pose graph with A-LOAM front-end, Scan Context, GICP, and IMU rotation priors | [Paper](https://arxiv.org/abs/2007.00258) |
| **[LVI-SAM](papers/lvi_sam/)** | Derived | Compact visual-lidar-inertial SLAM built on top of a LIO-SAM-style pose graph | - |
| **[VINS-Fusion](papers/vins_fusion/)** | Derived | Compact visual-inertial odometry with landmark reprojection and IMU preintegration | - |
| **[OKVIS](papers/okvis/)** | Derived | Fixed-window VIO with landmark reprojection and IMU preintegration | - |
| **[ORB-SLAM3](papers/orb_slam3/)** | Derived | Compact visual-inertial SLAM with keyframe graph and overlap-based loop closure | - |
| **[FAST-LIO2](papers/fast_lio2/)** | T-RO 2022 | Lightweight direct LIO with raw-point scan-to-map alignment and IMU prediction | [Paper](https://ieeexplore.ieee.org/document/9858003) |
| **[FAST-LIVO2](papers/fast_livo2/)** | Derived | Compact local visual-lidar-inertial odometry built on FAST-LIO2 poses and reprojection residuals | - |
| **[R2LIVE](papers/r2live/)** | Derived | Compact visual-lidar-inertial SLAM combining FAST-LIO2 odometry and visual landmark factors | - |
| **[FAST-LIO-SLAM](papers/fast_lio_slam/)** | Derived | Lightweight graph SLAM with FAST-LIO2 front-end, Scan Context, and GICP loop closures | - |

### Place Recognition / Loop Closure

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[Scan Context](papers/scan_context/)** | IROS 2018 | Lightweight place recognition with polar ring-sector descriptors and yaw-shift search | [Paper](https://ieeexplore.ieee.org/document/8593953) |

---

## Quick Start

### Docker (no local dependencies)

```bash
# Runs the full demo and writes report.html + JSON summaries to ./zoo-demo/
docker run --rm -v "$PWD/zoo-demo:/out" ghcr.io/rsasaki0109/localization_zoo:latest

# Pass demo options after the image name, e.g. a quicker method profile
docker run --rm -v "$PWD/zoo-demo:/out" ghcr.io/rsasaki0109/localization_zoo:latest \
  bash evaluation/scripts/docker_demo_entrypoint.sh --profile quick
```

### Native build

```bash
# Dependencies (Ubuntu 22.04)
sudo apt install libeigen3-dev libpcl-dev libopencv-dev libceres-dev libgtest-dev

# One-command demo: build + synthetic benchmark + broad real-data fixture suite
bash evaluation/scripts/demo_localization_zoo.sh

# Open the generated report
xdg-open experiments/results/runs/demo_localization_zoo/report.html
```

Manual build and test path:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure

# Benchmark (no external data required)
./build/evaluation/synthetic_benchmark
```

### Python bindings (experimental)

A thin pybind11 layer over a representative subset (KISS-ICP odometry,
NDT / GICP / LiTAMIN2 registration). Point clouds are Nx3 numpy arrays:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j"$(nproc)" --target localization_zoo_py
python3 python/example_synthetic.py   # end-to-end check, no dataset needed
```

```python
import sys; sys.path.insert(0, "build/python")
import localization_zoo as lz

odom = lz.KissICP()
for scan in scans:          # Nx3 float64 numpy arrays
    odom.register_frame(scan)
print(odom.pose)            # 4x4 world-frame pose
```

See [`python/README.md`](python/README.md) for the full API.

### ROS 2

```bash
cd ros2 && colcon build
source install/setup.bash

# Launch any algorithm node
ros2 run localization_zoo_ros litamin2_node
ros2 run localization_zoo_ros aloam_node
ros2 run localization_zoo_ros kiss_icp_node
ros2 run localization_zoo_ros ndt_node
ros2 run localization_zoo_ros ct_icp_node
ros2 run localization_zoo_ros gicp_node
ros2 run localization_zoo_ros dlo_node
ros2 run localization_zoo_ros ct_lio_node
ros2 run localization_zoo_ros fast_lio2_node
ros2 run localization_zoo_ros dlio_node
ros2 run localization_zoo_ros relead_node
ros2 run localization_zoo_ros xicp_node

# Play a rosbag
ros2 launch localization_zoo_ros play_rosbag.launch.py \
  bag_path:=/path/to/bag points_topic:=/velodyne_points
```

### Evaluation

```bash
# Compare trajectories on any dataset such as KITTI, MulRan, nuScenes, or TUM
python3 evaluation/scripts/benchmark.py \
  --gt gt_poses.txt \
  --est LiTAMIN2:litamin2_poses.txt A-LOAM:aloam_poses.txt \
  --output_dir results/
```

---

## Architecture

```
localization_zoo/
├── common/      # Shared Eigen/PCL utilities
├── papers/      # One self-contained dir per method (headers, sources, tests)
├── evaluation/  # Benchmark and evaluation tools
├── ros2/        # ROS 2 Humble wrappers
└── .github/workflows/ci.yml
```

Each `papers/*/` directory is a self-contained method (headers, sources, tests); the core libraries are ROS-independent. The full method list lives in the leaderboard and [Implementations](#implementations) tables above.

---

## Degeneracy Detection Demo

RELEAD and X-ICP can detect degenerate geometry such as tunnel-like environments:

```
=== Tunnel Environment ===
Has degeneracy: yes
Degenerate translation dirs: 1
  dir: [1, 0, 0]              # x direction (tunnel axis) is degenerate

=== Normal Environment ===
Has degeneracy: no             # walls and ground constrain all directions
```

ROS 2 nodes publish real-time status on `/degeneracy` with `std_msgs/Bool`.

---

## Adding a New Paper

```bash
mkdir -p papers/your_method/{include/your_method,src,test}
# 1. Write headers, sources, and tests
# 2. Add CMakeLists.txt
# 3. Add add_subdirectory to the top-level CMakeLists.txt
# 4. Run ctest and keep the full suite passing
```

See **[CONTRIBUTING.md](CONTRIBUTING.md)** for the full "one paper unit" checklist
(mechanism tests, KITTI eval, leaderboard row, honesty policy). Want a specific
paper reimplemented — or are you its author? Open a
[paper request](https://github.com/rsasaki0109/localization_zoo/issues/new?template=paper_request.yml).

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Eigen3 | >= 3.3 | Linear algebra |
| PCL | >= 1.10 | Point cloud processing |
| Ceres Solver | >= 2.0 | Nonlinear optimization |
| GTest | >= 1.11 | Unit testing |
| OpenCV | >= 4.0 | I/O utilities |

## Contributing

Contributions are welcome — new paper reimplementations, bug fixes, and
benchmark improvements. See [CONTRIBUTING.md](CONTRIBUTING.md) for scope, the
honesty policy, and the per-paper checklist.

## Citation

If you use this software or its benchmarks, please cite it (see
[CITATION.cff](CITATION.cff)) or use GitHub's "Cite this repository" button.

```bibtex
@software{sasaki_localization_zoo,
  author  = {Sasaki, Ryohei},
  title   = {Localization Zoo: from-paper {C++} reimplementations of {LiDAR}
             localization and odometry methods, honestly benchmarked},
  url     = {https://github.com/rsasaki0109/localization_zoo},
  version = {1.0.0},
  year    = {2026}
}
```

## License

MIT
