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

The current upstream bags are published on Hugging Face:

- `tunnel.bag` (~3.47 GB)
- `fog.bag` (~1.85 GB)

The upstream README lists Ouster packet topics, not a guaranteed ready-made
`sensor_msgs/PointCloud2` topic. Treat conversion as a two-step process:

1. inspect the bag and write a manifest,
2. extract directly only if a `PointCloud2` topic is present; otherwise replay
   `/os_cloud_node/lidar_packets` through the Ouster driver first.

### 1. Download + inspect

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --download \
  --inspect \
  --download-tool parallel \
  --download-connections 16
```

This writes:

- `data/lidar_degeneracy_datasets/tunnel.bag`
- `data/lidar_degeneracy_datasets/fog.bag`
- `data/lidar_degeneracy_datasets/tunnel_topics.json`
- `data/lidar_degeneracy_datasets/fog_topics.json`
- `data/lidar_degeneracy_datasets/*_manifest.json`

For a metadata-only dry run before downloading:

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --manifest-only
```

### 2. Extract when PointCloud2 is available

If inspection reports a LiDAR PointCloud2 topic:

```bash
python3 evaluation/scripts/prepare_lidar_degeneracy_inputs.py \
  --sequence tunnel \
  --inspect \
  --extract \
  --pointcloud-topic /os_cloud_node/points \
  --imu-topic /vectornav_node/uncomp_imu \
  --max-frames 200 \
  --time-mode azimuth
```

This writes `dogfooding_results/lidar_degeneracy_tunnel_200/` with
`00000000/cloud.pcd`, `frame_timestamps.csv`, and `imu.csv`.

### 3. Packet-topic fallback

If inspection reports only packet topics such as:

- `/os_cloud_node/lidar_packets`
- `/os_cloud_node/imu_packets`
- `/os_cloud_node/metadata`

first replay the bag through the Ouster driver referenced by the upstream README
to produce a PointCloud2 topic. Then run the existing extractor on that converted
bag:

```bash
mkdir -p ~/catkin_ouster_ws/src
cd ~/catkin_ouster_ws/src
git clone -b dev/sensor_sync_replay --recurse-submodules \
  https://github.com/ntnu-arl/ouster-ros.git
cd ~/catkin_ouster_ws
source /opt/ros/noetic/setup.bash
catkin_make --cmake-args -DCMAKE_BUILD_TYPE=Release
```

Generate and validate the replay script:

```bash
python3 evaluation/scripts/replay_ouster_packets.py \
  --sequence fog \
  --ros-setup /opt/ros/noetic/setup.bash \
  --ouster-workspace ~/catkin_ouster_ws
```

Run the conversion when ROS1 and `ouster_ros` are sourced:

```bash
python3 evaluation/scripts/replay_ouster_packets.py \
  --sequence fog \
  --ros-setup /opt/ros/noetic/setup.bash \
  --ouster-workspace ~/catkin_ouster_ws \
  --run
```

The helper records the decoded `/ouster/points` topic into
`data/lidar_degeneracy_datasets/fog_ouster_points.bag` and prints the follow-up
extract command.

```bash
python3 evaluation/scripts/extract_ros1_lidar_imu.py \
  --pointcloud-bag data/lidar_degeneracy_datasets/fog_ouster_points.bag \
  --pointcloud-topic /ouster/points \
  --imu-bag data/lidar_degeneracy_datasets/fog.bag \
  --imu-topic /vectornav_node/uncomp_imu \
  --output-dir dogfooding_results/lidar_degeneracy_fog_200 \
  --max-frames 200 \
  --time-mode azimuth
```

Topic names should be checked with `rosbag info`, the generated
`*_topics.json`, or the bag metadata.

Observed `fog.bag` inspection:

- LiDAR is packet-only: `/os_cloud_node/lidar_packets`.
- IMU is available: `/vectornav_node/uncomp_imu`.
- `/radar/cloud` is `sensor_msgs/PointCloud2`, but it is radar, not LiDAR.

Do not use `/radar/cloud` as the LiDAR odometry input. Use it only for a
radar-aware baseline or after adding a radar-specific adapter.

## GT-Free Degeneracy Diagnostics

The upstream README does not advertise a ground-truth trajectory, so the first
benchmark layer should be diagnostic rather than ATE-based:

```bash
python3 evaluation/scripts/analyze_lidar_degeneracy_sequence.py \
  dogfooding_results/lidar_degeneracy_tunnel_200 \
  experiments/results/lidar_degeneracy/tunnel_200
```

Outputs:

- `frame_degeneracy.csv`
- `summary.json`
- `summary.md`

The key fields are:

- `linearity`: high in long tunnel-like geometry.
- `planarity`: high when points mostly occupy a plane.
- `scattering`: low when the 3D covariance has a weak third component.
- `degeneracy_score`: `max(linearity, planarity)`.
- intensity summary: useful for fog / obscurant windows.

## First Benchmark Slice

After PCD export and degeneracy-window selection, run short windows before full
trajectories. If you have an external reference trajectory, use it here:

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

Do not fake GT just to get ATE. Keep the first pass as GT-free degeneracy
diagnostics plus method health metrics; add ATE only if a trustworthy external
pose source is available.

## Stop Line

Do not use this dataset to tune NDT/GICP precision recipes first. Use it after
KITTI recipes are stable, specifically to measure whether the method reports
degraded/unknown instead of silently producing confident wrong poses.
