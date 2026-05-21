# LiDAR Degeneracy Dataset Benchmark Setup

Source: <https://github.com/ntnu-arl/lidar_degeneracy_datasets>

The dataset is useful for the product-grade robustness track rather than for
first-pass tuning. It targets LiDAR-degenerative environments with an aerial
robot carrying LiDAR, IMU, and FMCW radar.

## Why It Fits This Repo

- Long straight tunnel: geometric self-similarity and weak lateral constraints.
- Fog-filled corridor: obscurants and degraded LiDAR returns.
- Sensor stack: Ouster OS0-128 LiDAR, VectorNav VN100 IMU, TI IWR6843AOP radar.
- The README provides CAD extrinsics with respect to the IMU.

Use it to test:

- degeneracy detection,
- false-lock rate,
- confidence/error calibration,
- outlier rejection under obscurants,
- fallback behavior for LiDAR-only vs LiDAR-inertial vs radar-inertial variants.

## Data Conversion Plan

1. Download the bag files linked from the upstream README.
2. Export LiDAR and IMU into dogfooding layout:

```bash
python3 evaluation/scripts/extract_ros1_lidar_imu.py \
  --pointcloud-bag /path/to/lidar.bag \
  --pointcloud-topic /os_cloud_node/points \
  --imu-bag /path/to/imu.bag \
  --imu-topic /vectornav_node/uncomp_imu \
  --output-dir dogfooding_results/lidar_degeneracy_tunnel_200 \
  --max-frames 200 \
  --time-mode azimuth
```

If the bags are ROS2 sqlite bags, use:

```bash
python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag /path/to/rosbag2_dir \
  --pointcloud-topic /os_cloud_node/points \
  --imu-topic /vectornav_node/uncomp_imu \
  --output-dir dogfooding_results/lidar_degeneracy_tunnel_200 \
  --max-frames 200
```

Topic names should be checked with `rosbag info`, `ros2 bag info`, or the bag
metadata. The upstream README also mentions packet topics; if only packets are
available, first replay them through the Ouster driver referenced upstream.

## First Benchmark Slice

Start with short windows before full trajectories:

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/lidar_degeneracy_tunnel_200 \
  experiments/reference_data/lidar_degeneracy_tunnel_gt.csv \
  200 \
  --methods ct_icp,fast_lio2,point_lio,kiss_icp
```

If ground truth is unavailable, still run odometry-only diagnostics and compare:

- trajectory smoothness,
- scan matching score,
- correspondence counts,
- runtime P95/P99,
- divergence / non-finite pose count,
- visual inspection of projected trajectory.

## Stop Line

Do not use this dataset to tune NDT/GICP precision recipes first. Use it after
KITTI recipes are stable, specifically to measure whether the method reports
degraded/unknown instead of silently producing confident wrong poses.
