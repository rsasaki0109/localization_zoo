# Upstream BIEVR-LIO adapter

This adapter feeds Localization Zoo's extracted PCD/IMU layout directly into
the unmodified ROS-independent core from
<https://github.com/ethz-asl/BIEVR-LIO>.

The upstream source is not vendored. `setup_bievr_lio_upstream.sh` checks out
commit `21121698f273d6fbfffca57546b940edb1de2ff0`, builds Ceres 2.2 into the
isolated prefix `/root/bievr_lio_deps/ceres-2.2`, and links this executable
against that prefix. The system Ceres 2.0 used by Localization Zoo is not
modified.

Input conversion:

- `cloud.pcd`: x/y/z and optional intensity/time are copied into upstream
  `StampedIntensityPointcloud`.
- A missing time field means a simultaneous depth frame (`time=0` for every
  point), matching the Azure Kinect capture model.
- `imu.csv` is converted into upstream `ImuMeasurement`.
- `frame_timestamps.csv` supplies the cloud header time.
- Upstream `Synchronizer` performs the actual LiDAR/IMU association.

The official pipeline logs `T_W_I` in TUM format. The evaluator applies the
configured LiDAR-to-IMU extrinsic as `T_W_L = T_W_I * T_I_L` before comparing
against the LiDAR GT.

```bash
evaluation/scripts/setup_bievr_lio_upstream.sh
evaluation/scripts/run_bievr_lio_hard_pcl.sh indoor_easy_01 100
evaluation/scripts/run_bievr_lio_hard_pcl.sh indoor_easy_01
evaluation/scripts/run_bievr_lio_hard_pcl.sh indoor_hard_01
```
