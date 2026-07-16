# Hard Point Cloud Localization Dataset Benchmark Setup

Source: <https://zenodo.org/records/10122133> (Kenji Koide, CC-BY 4.0, DOI
10.5281/zenodo.10122133)

"A dataset to evaluate the robustness of point-cloud-based localization
algorithms in extremely severe situations including aggressive sensor motion,
point cloud degeneration, and data interruptions."

## Why It Fits This Repo

This dataset complements the NTNU LiDAR degeneracy bags
(`SETUP_LIDAR_DEGENERACY_BENCHMARK.md`) by filling exactly the gaps that
investigation hit:

- **Ground truth included** (`gt.zip`, TUM format: `stamp tx ty tz qx qy qz qw`,
  epoch-second stamps, 8 trajectories). The NTNU tunnel work had to stay
  GT-free; here ATE/RPE for degeneracy methods is finally measurable.
- **Environment maps included** (4 `.ply` files, 5-24 MB). This unblocks the
  fixed-map localization false-lock track without first building maps from an
  odometry pass.
- **Kidnap sequences** (indoor x2, outdoor x1) directly test
  relocalization/false-lock behavior — the `fixed_map_ndt` publish-guard work
  needs exactly this.
- Different sensor characteristics from KITTI and NTNU: indoor sequences use an
  Azure Kinect (dense RGB-D-style clouds, short range), outdoor sequences a
  Livox MID360 (non-repetitive scan pattern).

## Files (17.2 GB total)

| File | Size | Notes |
|---|---:|---|
| `gt.zip` | 1 MB | 8 TUM trajectories, `traj_lidar_<sequence>.txt` |
| `map_indoor_easy.ply` | 5.2 MB | shared by indoor_easy_01/02 |
| `map_indoor_hard.ply` | 6.0 MB | indoor_hard_01, indoor_kidnap_01/02 (verify) |
| `map_outdoor_hard.ply` | 23.8 MB | outdoor_hard_01/02 |
| `map_outdoor_kidnap.ply` | 23.8 MB | outdoor_kidnap |
| `indoor_easy_01.zip` | 1.77 GB | Azure Kinect, baseline |
| `indoor_easy_02.zip` | 1.72 GB | Azure Kinect, baseline |
| `indoor_hard_01.zip` | 1.95 GB | aggressive motion + degradation |
| `indoor_kidnap_01.zip` | 2.35 GB | occlusion/relocation |
| `indoor_kidnap_02.zip` | 1.69 GB | occlusion/relocation |
| `outdoor_hard_01a.zip` + `01b.zip` | 1.64+1.35 GB | Livox MID360, split parts |
| `outdoor_hard_02a.zip` + `02b.zip` | 1.51+1.30 GB | Livox MID360, split parts |
| `outdoor_kidnap_a.zip` + `_b.zip` | 0.65+1.25 GB | Livox MID360, split parts |

GT pose counts: indoor 1.6k-2.4k poses/sequence, outdoor 4.0k-5.1k.

Download (Zenodo direct, no auth):

```bash
BASE=https://zenodo.org/api/records/10122133/files
wget -O gt.zip "$BASE/gt.zip/content"
# ... same pattern for every file above
```

Local layout used so far: `data/hard_pcl_localization/` with `gt/gt/*.txt`
unzipped alongside the map `.ply` files.

## Data Format

`indoor_easy_01.zip` was CRC-checked and inspected on 2026-07-16. It contains a
ROS2 sqlite bag (`metadata.yaml` + one 4.39 GB `.db3`) with 139.320 s duration:

| Topic | Type | Messages | Approx. rate | Observed payload |
|---|---|---:|---:|---|
| `/points2/decompressed` | `sensor_msgs/msg/PointCloud2` | 2,027 | 14.5 Hz | 167,488 points/frame; float32 `x/y/z` at offsets 0/4/8; `point_step=12`; no intensity or per-point time |
| `/points2/compressed` | `sensor_msgs/msg/PointCloud2` | 2,027 | 14.5 Hz | Same advertised fields and frame shape; use the decompressed topic |
| `/imu` | `sensor_msgs/msg/Imu` | 28,981 | 208 Hz | Standard angular velocity and linear acceleration |
| `/tf_static` | `tf2_msgs/msg/TFMessage` | 2 | - | Static sensor transforms |
| `/tf` | `tf2_msgs/msg/TFMessage` | 0 | - | Empty |

The indoor cloud is an Azure Kinect depth frame rather than a rotating scan, so
the missing per-point time is expected. The ROS2 extractor adds zero intensity
for PointXYZI consumer compatibility but does not invent per-point time;
CT-ICP's runner then uses its documented index-time fallback.

### Clock-domain check

The PointCloud2 header stamps and TUM GT stamps share Unix epoch seconds and are
exactly one-to-one for `indoor_easy_01`: both have 2,027 entries, with matching
first (`1693718238.937876`) and last (`1693718378.314670`) stamps. No clock
offset or nearest-neighbor resampling is needed. The sqlite receive timestamp
for the first cloud is about 0.216 s later than its header stamp and must not be
used for GT association. IMU header stamps cover `1693718239.113426` through
`1693718378.318311`.

`indoor_hard_01` has the same schema and exact association: 2,379 LiDAR frames
and 2,379 GT poses, from `1690354429.463785` through `1690354593.400955`.
Its first cloud has 165,714 XYZ points, and the bag contains 44,398 IMU samples
(about 271 Hz). Unlike the easy bag, its metadata advertises only the two cloud
topics and IMU; it has no `/tf_static` topic.

The outdoor bags use standard PointCloud2 rather than a Livox custom message.
Their `/livox/points` records contain float32 `x/y/z`, uint32 `t` (nanoseconds
within the scan), float32 `intensity`, uint8 `tag`, and uint8 `line`.
`extract_ros2_lidar_imu.py` converts `t` to a float32 PCD `time` field in
seconds. Each outdoor trajectory is split across two bag directories; repeat
`--bag` in chronological order to concatenate them. All three outdoor
trajectories also have exact one-to-one cloud-header/GT stamp association:

| Sequence | LiDAR / GT frames | Cloud topic |
|---|---:|---|
| `outdoor_hard_01` | 5,147 | `/livox/points` |
| `outdoor_hard_02` | 5,127 | `/livox/points` |
| `outdoor_kidnap` | 4,017 | `/livox/points` |

### Extraction

```bash
/root/lz-venv/bin/python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag data/hard_pcl_localization/indoor_easy_01 \
  --pointcloud-topic /points2/decompressed --imu-topic /imu \
  --output-dir dogfooding_results/hard_pcl_localization/indoor_easy_01

# Split outdoor bags are concatenated by repeating --bag. The extractor keeps
# Livox's per-point uint32 t field as PCD float time in seconds.
/root/lz-venv/bin/python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag data/hard_pcl_localization/outdoor_hard_01a \
  --bag data/hard_pcl_localization/outdoor_hard_01b \
  --pointcloud-topic /livox/points --imu-topic /livox/imu \
  --output-dir dogfooding_results/hard_pcl_localization/outdoor_hard_01

/root/lz-venv/bin/python3 evaluation/scripts/tum_to_gt_csv.py \
  --input data/hard_pcl_localization/gt/gt/traj_lidar_indoor_easy_01.txt \
  --output experiments/reference_data/hard_pcl_indoor_easy_01_gt.csv

# The X-ICP/DegenSense and older window runner JSON writers leave ATE null.
# Apply the same GT-backed planar ATE/RPE evaluation to every result JSON:
/root/lz-venv/bin/python3 evaluation/scripts/evaluate_window_odometry_gt.py \
  --result experiments/results/hard_pcl_localization/indoor_easy_01/result.json

# Or run all six default configurations and attach GT metrics in one command:
/root/lz-venv/bin/python3 evaluation/scripts/run_hard_pcl_odometry_benchmark.py \
  --pcd-dir dogfooding_results/hard_pcl_localization/indoor_easy_01 \
  --gt-csv experiments/reference_data/hard_pcl_indoor_easy_01_gt.csv \
  --output-dir experiments/results/hard_pcl_localization/indoor_easy_01/full \
  --jobs 6

# The outdoor DegenSense runs have much larger maps. Run the remaining matrix
# sequentially to avoid WSL memory pressure. Set WINDOWS_REPO_ROOT only when a
# separate WSL build clone should copy completed JSONs back to the host clone.
WINDOWS_REPO_ROOT=/mnt/c/path/to/localization_zoo \
  evaluation/scripts/run_remaining_hard_pcl_outdoor_degensense.sh

# Provided-map localization (the explicit PLY option avoids rebuilding the map
# from the evaluated sequence's GT poses):
build/evaluation/pcd_dogfooding \
  dogfooding_results/hard_pcl_localization/indoor_kidnap_01 \
  experiments/reference_data/hard_pcl_indoor_kidnap_01_gt.csv 2154 \
  --methods fixed_map_ndt \
  --fixed-map-ndt-map-ply data/hard_pcl_localization/map_indoor_hard.ply \
  --fixed-map-ndt-seed-source ct_icp \
  --fixed-map-ndt-trace-json \
    experiments/results/hard_pcl_localization/fixed_map_ndt/traces/indoor_kidnap_01_trace.json
```

`extract_ros2_lidar_imu.py` writes the dogfooding layout
(`NNNNNNNN/cloud.pcd`, `frame_timestamps.csv`, `imu.csv`). All eight sequences
have been extracted: indoor frame counts are 2,027, 1,967, 2,379, 2,154, and
1,609; outdoor frame counts are 5,147, 5,127, and 4,017.

## BIEVR-LIO Follow-up

[BIEVR-LIO](https://github.com/ethz-asl/BIEVR-LIO) (inspected at commit
`21121698f273d6fbfffca57546b940edb1de2ff0`) is a strong next baseline for this
dataset. Unlike the current publish-gate methods, it attempts to recover the
weak geometric constraints themselves: a 0.5 m voxel stores a 0.05 m
voxel-oriented bump image, and map-informed sampling concentrates registration
on voxels with informative fine geometry. Its core is ROS-independent and its
ROS2 `process_bag` entry point already accepts PointCloud2 + Imu bags.

Two integration requirements must be handled explicitly:

1. The upstream PointCloud2 conversion rejects clouds without a `t`, `time`, or
   `timestamp` field. Koide indoor clouds are Azure Kinect depth frames with
   only float32 `x/y/z`; an adapter should treat the whole depth frame as
   simultaneous (`time=0`) rather than fabricate rotating-LiDAR firing order.
2. Upstream installs Ceres 2.2.0, while the current WSL benchmark clone uses
   Ceres 2.0.0. Build BIEVR-LIO in a separate colcon/Docker workspace; do not
   replace the working Localization Zoo Ceres installation.

The indoor bag provides the required calibration through `/tf_static`.
PointCloud2 uses `depth_camera_link`, IMU uses `imu_link`, and inversion of the
recorded `T_depth_camera_link_imu_link` gives this candidate
`T_IMU_LIDAR` (verify the TF convention in an initial stationary run):

```yaml
topics:
  pointcloud: "/points2/decompressed"
  imu: "/imu"
calibration:
  translation: [-0.05089043, 0.00331340, 0.00087367]
  rotation: [ 0.00293580,  0.09906371, -0.99507676,
             -0.99999529, -0.00059868, -0.00300991,
             -0.00089391,  0.99508091,  0.09906149]
lidar:
  min_range_m: 0.3
  max_range_m: 10.0
```

Upstream can write TUM directly via `debug.trajectory_path`, so its output can
be evaluated against the same converted GT. BIEVR-LIO is a recovery baseline,
not merely another degeneracy signal: compare it against the unchanged six-run
matrix before considering integration into the triage policy.

## Completion Target

Run all 8 sequences through the benchmark harness:

1. **Odometry track (GT-backed)**: baselines (KISS, LiTAMIN2, CT-ICP) and
   degeneracy-aware methods (X-ICP, DegenSense) via the window-odometry
   runners; report ATE/RPE against the TUM GT, not just GT-free health gates.
2. **Fixed-map localization track**: `fixed_map_ndt` (and Scan Context
   hypotheses) against the provided `.ply` maps; evaluate the publish-guard /
   false-lock policy with real GT error labels, especially on the kidnap
   sequences.
3. **Degeneracy calibration**: use GT error to calibrate the GT-free triage
   policy (`lidar_degeneracy_triage_v4`) — the NTNU work explicitly staged
   this in `risk_gt_calibration/` waiting for a GT-backed dataset.

## Status

- [x] gt.zip + 4 maps downloaded and inspected (TUM format confirmed)
- [x] indoor_easy_01 and indoor_hard_01 downloaded, checksum/CRC-checked, and extracted
- [x] indoor bag inspection: topics, message types, and clock domain vs GT stamps
- [x] ROS2 PointCloud2 + IMU extraction path into dogfooding PCD layout
- [x] TUM -> dogfooding GT CSV converter
- [x] first odometry slice: indoor_easy_01 and indoor_hard_01 full trajectories, six unchanged configurations, GT-backed ATE/RPE
- [x] fixed-map false-lock slice: both indoor kidnap sequences, provided PLY, CT-ICP seed, GT-labeled trace/verifier/replay
- [x] `lidar_degeneracy_triage_v4` GT calibration populated from easy/hard full results
- [x] remaining sequence downloads (all Zenodo archives size + MD5 verified)
- [x] all eight bags extracted; outdoor split-bag PointCloud2 `t` conversion verified
- [x] indoor_easy_02 full six-method GT benchmark
- [x] outdoor hard/kidnap fast-method GT benchmarks
- [x] outdoor_kidnap provided-map false-lock trace and three-sequence guard replay
- [ ] remaining long-running outdoor DegenSense configurations
