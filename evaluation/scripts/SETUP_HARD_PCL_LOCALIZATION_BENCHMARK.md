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

The sequence zips contain ROS2 bags (per the Zenodo description: PointCloud2,
IMU, and Livox-format messages). Exact topic names/types TBD after first
inspection — record them here once known. The `rosbags` Python package already
used by `extract_ouster_packet_lidar_imu.py` reads ROS2 bags too, so the
extraction path should mirror the NTNU one: bag -> dogfooding PCD layout
(`NNNNNNNN/cloud.pcd`, `frame_timestamps.csv`, `imu.csv`) plus a GT CSV
converted from the TUM file.

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
- [ ] indoor_easy_01 / indoor_hard_01 downloaded (in progress)
- [ ] bag inspection: topics, message types, clock domain vs GT stamps
- [ ] ROS2 extraction path into dogfooding PCD layout
- [ ] TUM -> dogfooding GT CSV converter
- [ ] first odometry slice (indoor_easy_01 baseline vs indoor_hard_01)
- [ ] fixed-map false-lock slice (kidnap sequences)
- [ ] remaining sequence downloads (indoor_easy_02, indoor_kidnap_01/02, outdoor parts)
