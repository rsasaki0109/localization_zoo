# Implementations

Main method families. Each `papers/<method>/` directory holds the
headers, sources, tests and a README with the paper reference, deviations and
results. Labels: **paper reimplementation**, **derived variant** or
**compact baseline**; a benchmark result does not by itself prove paper
fidelity (see [reproduction status](reproduction_status.md) and
[paper-ready criteria](paper_ready_reproducibility.md)). The interactive
[method explorer](https://rsasaki0109.github.io/localization_zoo/) lists all
107 methods with filters.

## Point Cloud Registration

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[LiTAMIN2](../papers/litamin2/)** | ICRA 2021 | KL-divergence ICP with aggressive point reduction for faster registration | [arXiv](https://arxiv.org/abs/2103.00784) |
| **[GICP](../papers/gicp/)** | RSS 2009 | Plane-to-plane ICP with local covariance modeling and Mahalanobis distance | [Paper](https://www.roboticsproceedings.org/rss05/p31.html) |
| **[Voxel-GICP](../papers/voxel_gicp/)** | RA-L 2021 | GICP accelerated with voxel representatives and voxel-level covariance | [Paper](https://arxiv.org/abs/2109.07082) |
| **[L-LO](../papers/l_lo/)** | arXiv 2023 | Landmark convex hulls matched by turning-function + Hausdorff similarity, registered by maximising hull overlap; no author code | [arXiv](https://arxiv.org/abs/2312.16787) |
| **[small_gicp](../papers/small_gicp/)** | Derived | Compact GICP with voxel downsampling and capped correspondences | [GitHub](https://github.com/koide3/small_gicp) |
| **[VGICP-SLAM](../papers/vgicp_slam/)** | Derived | Voxel-GICP front-end with Scan Context and loop-graph back-end | - |
| **[NDT](../papers/ndt/)** | IROS 2003 | NDT-style registration against voxel Gaussian models with a compact optimizer | [Paper](https://ieeexplore.ieee.org/document/1249285) |
| **[KISS-ICP](../papers/kiss_icp/)** | RA-L 2023 | Compact KISS-ICP-style pipeline with voxel hashing, adaptive thresholds, and robust ICP | [Paper](https://arxiv.org/abs/2209.15397) |
| **[A-LOAM](../papers/aloam/)** | RSS 2014 | Curvature-based feature extraction with a three-stage odom-to-map pipeline | [GitHub (ROS1)](https://github.com/HKUST-Aerial-Robotics/A-LOAM) |
| **[F-LOAM](../papers/floam/)** | Derived | Lightweight LOAM pipeline with input thinning and sparse map updates | - |
| **[ISC-LOAM](../papers/isc_loam/)** | Derived | Lightweight LOAM with intensity descriptors and F-LOAM/GICP loop graph | - |
| **[LOAM Livox](../papers/loam_livox/)** | Derived | LOAM variant for solid-state LiDAR using pseudo scan lines from azimuth sectors | [Reference](https://github.com/hku-mars/loam_livox) |
| **[LeGO-LOAM](../papers/lego_loam/)** | IROS 2018 | Ground-aware feature extraction for vehicle-oriented LOAM | [Paper](https://ieeexplore.ieee.org/document/8594299) |
| **[MULLS](../papers/mulls/)** | Paper + derived | MULLS-LO (`--mulls-lo`): ground filter, PCA pillar/facade classes, multi-metric linear least-squares ICP; the default is a lighter A-LOAM-feature variant | - |
| **[BALM2](../papers/balm2/)** | T-RO 2022 | Local bundle adjustment over recent keyframes with line and plane residuals | [arXiv](https://arxiv.org/abs/2209.08854) |
| **[SuMa](../papers/suma/)** | RSS 2018 | Dense surfel-based point-to-plane odometry from range-image maps | [GitHub](https://github.com/jbehley/SuMa) |
| **[DLO](../papers/dlo/)** | Derived | Direct keyframe odometry that aligns dense scans to a local GICP map | [GitHub](https://github.com/vectr-ucla/direct_lidar_odometry) |
| **[HDL Graph SLAM](../papers/hdl_graph_slam/)** | Derived | NDT front-end with floor priors, Scan Context, and GICP loop closures | [GitHub](https://github.com/koide3/hdl_graph_slam) |
| **[CT-ICP](../papers/ct_icp/)** | ICRA 2022 | Continuous-time registration with two poses per frame and SLERP motion compensation | [GitHub (ROS1)](https://github.com/jedeschaud/ct_icp) |
| **[X-ICP](../papers/xicp/)** | T-RO 2024 | Constrained ICP using Hessian SVD to classify observable directions | [arXiv](https://arxiv.org/abs/2211.16335) |

## Degeneracy-Aware Methods

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[RELEAD](../papers/relead/)** | ICRA 2024 | Constrained ESIKF with projection-based suppression along degenerate directions | [arXiv](https://arxiv.org/abs/2402.18934) |
| **[CT-ICP + RELEAD](../papers/ct_icp_relead/)** | Hybrid | Continuous-time CT-ICP interpolation combined with RELEAD degeneracy handling | - |
| **[LF-GICP](../papers/lf_gicp/)** | arXiv 2026 | Voxel-normal localizability field gates soft Fisher-information reweighting of GICP; no author code | [arXiv](https://arxiv.org/abs/2608.19522) |

## Foundations

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[IMU Preintegration](../papers/imu_preintegration/)** | T-RO 2017 | Preintegration on SO(3) with first-order bias correction for LIO pipelines | [Paper](https://arxiv.org/abs/1512.02363) |

## LIO / Continuous-Time Fusion

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[CT-LIO](../papers/ct_lio/)** | Hybrid | Lightweight LIO that combines CT-ICP interpolation with IMU preintegration constraints | [CLINS](https://arxiv.org/abs/2109.04687) |
| **[DLIO](../papers/dlio/)** | ICRA 2023 | Compact DLIO-style LIO built on DLO-style registration plus IMU preintegration | [GitHub](https://github.com/vectr-ucla/direct_lidar_inertial_odometry) |
| **[LINS](../papers/lins/)** | Derived | Lightweight LiDAR-inertial estimator with iterated filtering and point-to-plane updates | - |
| **[Point-LIO](../papers/point_lio/)** | Derived | Compact direct LIO with raw-point planarity correspondences and iterated filtering | - |
| **[CLINS](../papers/clins/)** | Derived | Sequence pipeline version of CT-LIO-style continuous-time registration | - |
| **[VILENS](../papers/vilens/)** | Derived | Compact visual-lidar-inertial smoother with Point-LIO-style local maps and reprojection fusion | - |
| **[LIO-SAM](../papers/lio_sam/)** | IROS 2020 | Lightweight pose graph with A-LOAM front-end, Scan Context, GICP, and IMU rotation priors | [Paper](https://arxiv.org/abs/2007.00258) |
| **[LVI-SAM](../papers/lvi_sam/)** | Derived | Compact visual-lidar-inertial SLAM built on top of a LIO-SAM-style pose graph | - |
| **[VINS-Fusion](../papers/vins_fusion/)** | Derived | Compact visual-inertial odometry with landmark reprojection and IMU preintegration | - |
| **[OKVIS](../papers/okvis/)** | Derived | Fixed-window VIO with landmark reprojection and IMU preintegration | - |
| **[ORB-SLAM3](../papers/orb_slam3/)** | Derived | Compact visual-inertial SLAM with keyframe graph and overlap-based loop closure | - |
| **[FAST-LIO2](../papers/fast_lio2/)** | T-RO 2022 | Lightweight direct LIO with raw-point scan-to-map alignment and IMU prediction | [Paper](https://ieeexplore.ieee.org/document/9858003) |
| **[FAST-LIVO2](../papers/fast_livo2/)** | Derived | Compact local visual-lidar-inertial odometry built on FAST-LIO2 poses and reprojection residuals | - |
| **[R2LIVE](../papers/r2live/)** | Derived | Compact visual-lidar-inertial SLAM combining FAST-LIO2 odometry and visual landmark factors | - |
| **[FAST-LIO-SLAM](../papers/fast_lio_slam/)** | Derived | Lightweight graph SLAM with FAST-LIO2 front-end, Scan Context, and GICP loop closures | - |

## IMU Dead Reckoning / Aided INS

IMU-only methods (no LiDAR registration); evaluated on NCLT 2013-01-10 and
KITTI Raw drive 0009 with `imu.csv` inputs, not on the KITTI LiDAR leaderboard.

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[IMU Dead Reckoning](../papers/imu_dead_reckoning/)** | Baseline | Strapdown INS lower-bound for the LIO family, with opt-in classical aids (ZUPT / NHC / attitude leveling / ZARU), a motion-gated static-init quality gate, and a 15-state error-state Kalman filter (`--imu-dr-eskf`) mode | - |
| **[OdoNet](../papers/odonet/)** | IEEE Sens. J. 2022 | 1D-CNN pseudo-odometer speed aiding for strapdown INS with NHC/ZUPT | [Paper](https://arxiv.org/abs/2109.03091) |
| **[NN-ZUPT](../papers/nn_zupt/)** | Meas. Sci. Technol. 2023 | CNN zero-velocity detection driving ZUPT corrections for vehicle INS | [Paper](https://doi.org/10.1088/1361-6501/acabde) |
| **[NHC-Net](../papers/nhc_net/)** | GPS Solutions 2023 | Motion-state CNN with adaptive non-holonomic constraints for vehicle dead reckoning | - |

## IMU Motion & Health SDK

[`imu_motion_health`](../papers/imu_motion_health/) is a LiDAR-free streaming C++
SDK and CSV replay CLI for 6-axis IMUs. It diagnoses startup calibration,
bias, timestamp gaps, non-finite values, and saturation; classifies stationary,
moving, impact, fall/tilt, and vibration states; and emits JSON diagnostics plus
short-term relative attitude, velocity, and position. The attitude comes from a
VQF filter by default: 0.70° inclination RMSE on the BROAD benchmark. Its
standalone build only requires Eigen3 and GTest, and the included
deterministic demo needs no sensor download.

## IMU Noise (Allan Variance)

[`allan_variance`](../papers/allan_variance/) computes the overlapping Allan
deviation of a static IMU log (CSV or `.npy`) and extracts the white noise
density, bias random walk, and bias instability. It also writes them as a
Kalibr-style noise file for LIO/VIO configs. On TUM VI's 111 h static BMI160
recording, the paper's own fit protocol reproduces its published values within
0.96-1.01x ([`docs/allan_variance.md`](allan_variance.md)):

| | Repo | TUM VI paper |
|---|---:|---:|
| Gyro noise density [rad/s/√Hz] | 8.05e-5 | 8.0e-5 |
| Gyro random walk [rad/s²/√Hz] | 2.17e-6 | 2.2e-6 |
| Accel noise density [m/s²/√Hz] | 1.34e-3 | 1.4e-3 |
| Accel random walk [m/s³/√Hz] | 8.50e-5 | 8.6e-5 |

## IMU Attitude Estimation (BROAD)

[`attitude_estimation`](../papers/attitude_estimation/) holds the Madgwick, Mahony,
VQF, Valenti, and Seel orientation filters, evaluated on the public BROAD benchmark (39 trials
with motion-capture ground truth, CC BY 4.0) using the dataset's own error
metric. The published BROAD per-trial errors are reproduced within 0.001 deg,
and the VQF paper's values within 1 %. VQF, written from the paper, matches the
authors' code within 0.0002 deg per trial. All-trial RMSE in degrees:

| Variant | Mode | Repo | Published |
|---|---|---:|---:|
| Madgwick (x-io code, beta 0.12) | 9D total | 4.96 | 4.96 (BROAD) |
| Madgwick (field-scale bug fixed, beta 0.08) | 9D total | 4.69 | 4.69 (BROAD #1) |
| Mahony (Kp 0.74, Ki 0.0012) | 9D total | 7.49 | 7.49 (BROAD) |
| Madgwick / Mahony (VQF parameters) | 6D inclination | 5.04 / 5.16 | 5.0 / 5.2 (VQF) |
| VQF (defaults) | 9D total | 2.30 | 2.3 (VQF) |
| VQF (defaults) | 6D inclination | 0.70 | 0.7 (VQF) |
| Valenti / Seel (VQF parameters) | 9D total | 6.07 / 5.11 | 6.1 / 5.1 (VQF) |

Full table: [`docs/attitude_benchmark.md`](attitude_benchmark.md).

## Place Recognition / Loop Closure

| Paper | Venue | Key Idea | Reference |
|-------|-------|----------|-----------|
| **[Scan Context](../papers/scan_context/)** | IROS 2018 | Lightweight place recognition with polar ring-sector descriptors and yaw-shift search | [Paper](https://ieeexplore.ieee.org/document/8593953) |

The [KITTI place-recognition benchmark](place_recognition_benchmark.md)
scores these descriptors on full KITTI Odometry 00/02/05/07/08 (top-1 retrieval,
4 m revisit radius, 50-frame exclusion). Scan Context is best on four of five
sequences (F1max 0.935 on 00, 0.920 on 05); the intensity variant from
ISC-LOAM trails by 0.03-0.09 except on reverse-direction KITTI 08. The DTD port
fails on real scans (F1max ≤ 0.035); see [its README](../papers/dtd/README.md).

| F1max | 00 | 02 | 05 | 07 | 08 |
|---|---:|---:|---:|---:|---:|
| Scan Context | **0.935** | **0.837** | **0.920** | **0.588** | 0.592 |
| ISC (intensity) | 0.879 | 0.746 | 0.872 | 0.562 | **0.597** |
| DTD | 0.000 | 0.000 | 0.000 | 0.035 | 0.013 |

## Degeneracy detection demo

RELEAD and X-ICP report underconstrained geometry:

```
=== Tunnel Environment ===
Has degeneracy: yes
Degenerate translation dirs: 1
  dir: [1, 0, 0]              # x direction (tunnel axis) is degenerate

=== Normal Environment ===
Has degeneracy: no             # walls and ground constrain all directions
```

ROS 2 publishes the status on `/degeneracy`.
