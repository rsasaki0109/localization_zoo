<div align="center">
  <h1 align="center">Localization Zoo</h1>
  <p align="center">
    <b>C++ implementations, derived variants, and compact baselines for localization papers</b>
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

---

## Why Localization Zoo?

Many localization papers do not ship reusable reference implementations.
This repository collects paper-oriented C++ implementations behind a unified API, with ROS 2 nodes and evaluation tools that are ready to run.

- **Pure C++**: ROS-independent core libraries for research, education, and embedded use
- **ROS 2 Humble**: Ready-to-run nodes via `ros2 run`
- **Built-in benchmarking**: Compare methods immediately after build
- **Degeneracy-aware methods**: Includes RELEAD and X-ICP for constrained environments

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

Papers with **no public author code**, run as pure odometry on KITTI full
sequences (first-pose anchor, `--no-gt-seed`, uniform `--*-dense-profile`; no IMU,
so LIO methods use constant-velocity fallback). RPE is drift %/100 m; ATE in parens.

| Method | Seq 00 _(4541 fr)_ | Seq 07 _(1101 fr)_ | Paper |
|---|---:|---:|---|
| M-GCLO | **0.835%** <sub>(19 m)</sub> | 0.671% <sub>(2 m)</sub> | ISPRS Ann. 2024 |
| KC-LO | 0.842% <sub>(14 m)</sub> | **0.514%** <sub>(1 m)</sub> | ECCV 2004 |
| LODESTAR | 0.848% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2511.09142 |
| Terrain-RBF-LIO | 0.849% <sub>(8 m)</sub> | 0.587% <sub>(1 m)</sub> | arXiv:2509.26222 |
| DALI-SLAM | 0.849% <sub>(8 m)</sub> | 0.600% <sub>(1 m)</sub> | ISPRS JPRS 2025 |
| DAMM-LOAM | 0.851% <sub>(7 m)</sub> | 0.598% <sub>(1 m)</sub> | arXiv:2510.13287 |
| CUBE-LIO | 0.851% <sub>(9 m)</sub> | 0.608% <sub>(1 m)</sub> | ROBOMECH 2026 |
| Intensity-Flow | 0.856% <sub>(8 m)</sub> | 0.616% <sub>(1 m)</sub> | Measurement 2026 |
| Quadric-LO | 0.867% <sub>(15 m)</sub> | 0.598% <sub>(2 m)</sub> | arXiv:2304.14190 |
| Adaptive-ICP | 0.870% <sub>(11 m)</sub> | 0.569% <sub>(1 m)</sub> | arXiv:2509.22058 |
| MCC-LO | 0.892% <sub>(13 m)</sub> | 0.611% <sub>(2 m)</sub> | PLOS ONE 2018 |
| NHC-LIO | 0.902% <sub>(18 m)</sub> | 0.608% <sub>(3 m)</sub> | IEEE Sens. J. 2023 |
| SVN-ICP | 0.912% <sub>(14 m)</sub> | 0.607% <sub>(3 m)</sub> | arXiv:2509.08069 |
| TrICP-LO | 0.931% <sub>(10 m)</sub> | 0.662% <sub>(2 m)</sub> | IVC 2005 |
| GMM-LO | 0.941% <sub>(14 m)</sub> | 0.657% <sub>(1 m)</sub> | arXiv:1807.02587 |
| Student-T-LO | 0.952% <sub>(15 m)</sub> | 0.696% <sub>(2 m)</sub> | PMC11314997 2024 |
| Small-but-Mighty | 0.961% <sub>(15 m)</sub> | 0.897% <sub>(3 m)</sub> | Remote Sens. 2025 |
| GNC-LO | 0.986% <sub>(18 m)</sub> | 0.722% <sub>(2 m)</sub> | arXiv:1909.08605 |
| IMLS-SLAM | 1.000% <sub>(18 m)</sub> | 0.700% <sub>(3 m)</sub> | ICRA 2018 |
| CT-VoxelMap | 1.046% <sub>(21 m)</sub> | 0.800% <sub>(3 m)</sub> | arXiv:2604.03747 |
| Vibration-LIO | 1.082% <sub>(15 m)</sub> | 0.781% <sub>(3 m)</sub> | arXiv:2507.04311 |
| BIEVR-LIO | 1.063% <sub>(25 m)</sub> | 0.873% <sub>(4 m)</sub> | arXiv:2604.14421 |
| R-VoxelMap | 1.076% <sub>(20 m)</sub> | _diverges_ | arXiv:2601.12377 |
| PCR-DAT | 1.239% <sub>(11 m)</sub> | 1.040% <sub>(4 m)</sub> | ISR 2024 |
| LiDAR-IBA | 2.001% <sub>(8 m)</sub> | 1.474% <sub>(1 m)</sub> | arXiv:2602.06380 |
| D2-LIO | 5.794% <sub>(106 m)</sub> | 0.804% <sub>(2 m)</sub> | arXiv:2508.14355 |
| DegenSense | 9.931% <sub>(417 m)</sub> | 9.940% <sub>(39 m)</sub> | arXiv:2412.07513 |
| Spectral-LO | 12.029% <sub>(113 m)</sub> | 13.671% <sub>(47 m)</sub> | arXiv:2005.02042 |
| DiLO | 18.305% <sub>(226 m)</sub> | 18.966% <sub>(159 m)</sub> | ETRI J. 2021 |
| UA-LIO | _diverges_ | _diverges_ | IEEE TIM 2025 |
| _KISS-ICP (same profile, ref)_ | _0.872%_ <sub>(12 m)</sub> | _0.618%_ <sub>(2 m)</sub> | — |
| _CT-ICP (same profile, ref)_ | _2.577%_ <sub>(17 m)</sub> | _2.500%_ <sub>(4 m)</sub> | — |

The top ten (M-GCLO through Adaptive-ICP) **match or beat KISS-ICP on both
sequences**, well clear of CT-ICP. **M-GCLO** leads seq-00 drift (0.835%) via
multiple-ground-plane constraints (higher ATE — an honest RPE/ATE split).
**KC-LO** (correspondence-free kernel correlation) leads seq-07 drift (0.514%)
and beats KISS-ICP on both sequences — at a heavy throughput cost (~1.4 FPS).

Recurring honest finding: on geometry-rich, IMU-free KITTI most robust/soft
mechanisms go near-redundant and the front-end reduces to a ~KISS-ICP
point-to-plane core. Honest negatives: DiLO (direct, 18–19% drift), Spectral-LO
(ICP-free BEV phase-correlation, fastest at ~14 FPS but coarse ~12–14%),
R-VoxelMap (diverges seq 07), UA-LIO/DegenSense. Per-method caveats live in the
module READMEs; raw JSON:
[`docs/benchmarks/kitti_full_new_methods/`](docs/benchmarks/kitti_full_new_methods/).
<!-- LEADERBOARD:END -->

### Trajectory gallery — KITTI seq07, all methods, one figure

Top-down 2D trajectories on KITTI seq07 (1101 frames, `--no-gt-seed`), each
method (color) over ground truth (gray), on a shared scale and ordered by RPE
drift. **KC-LO** leads; the divergent tails of NDT / SuMa are honest no-GT-seed
failures, not cropping. Regenerate with
[`evaluation/scripts/plot_trajectory_grid.py`](evaluation/scripts/plot_trajectory_grid.py).

<p align="center">
  <img src="docs/assets/grid_seq07.png" alt="KITTI seq07 2D trajectory gallery: 15 localization methods over ground truth, no GT seed, ordered by RPE drift" width="960">
</p>

## Scope Note

This repository mixes three levels of implementation scope:

- **Paper reimplementation**: intended to track the published method closely
- **Derived variant**: built from the paper idea, but adapted to this repository's shared components
- **Compact baseline**: a smaller approximation that keeps the interface and core intuition, but not the full paper pipeline

Methods already labeled `Derived` or `Hybrid` are intentionally adapted versions.
Some paper-named entries still use compact or simplified internals today, especially `NDT`, `KISS-ICP`, and `DLIO`. Check each method README for current scope and deviations.
The tracked claim boundary for original-paper reproduction is summarized separately in [`docs/reproduction_status.md`](docs/reproduction_status.md); benchmark usefulness and paper-result reproducibility are not treated as the same thing in this repository.

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

Each `papers/*/` directory is self-contained; the core libraries are ROS-independent. See [Implementations](#implementations) for the full catalog.

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

## License

MIT
