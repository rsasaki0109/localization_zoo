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

### Extraction

```bash
/root/lz-venv/bin/python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag data/hard_pcl_localization/indoor_easy_01 \
  --pointcloud-topic /points2/decompressed --imu-topic /imu \
  --output-dir dogfooding_results/hard_pcl_localization/indoor_easy_01

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
```

`extract_ros2_lidar_imu.py` writes the dogfooding layout
(`NNNNNNNN/cloud.pcd`, `frame_timestamps.csv`, `imu.csv`). Inspect outdoor bags
separately before extraction: their Livox MID360 custom-message path is not
established by this indoor inspection.

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
- [x] indoor_easy_01 downloaded, CRC-checked, and extracted; indoor_hard_01 resumable download in progress
- [x] indoor bag inspection: topics, message types, and clock domain vs GT stamps
- [x] ROS2 PointCloud2 + IMU extraction path into dogfooding PCD layout
- [x] TUM -> dogfooding GT CSV converter
- [ ] first odometry slice: indoor_easy_01 full baseline complete; indoor_hard_01 download pending
- [ ] fixed-map false-lock slice (kidnap sequences)
- [ ] remaining sequence downloads (indoor_easy_02, indoor_kidnap_01/02, outdoor parts)
