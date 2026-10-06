# localization_zoo ROS 2 wrappers

## IMU Motion & Health

`imu_motion_health_node` is a LiDAR-free ROS 2 wrapper around the reusable
`imu_motion_health` C++ core. It subscribes to `sensor_msgs/msg/Imu` and
publishes:

| Topic | Type | Contents |
| --- | --- | --- |
| `/imu/odom` | `nav_msgs/msg/Odometry` | short-term relative position, velocity, and attitude |
| `/imu/health_json` | `std_msgs/msg/String` | one core JSON health snapshot per IMU sample |
| `/imu/events_json` | `std_msgs/msg/String` | one `imu_motion_health_event_v1` JSON record per started/ended event |
| TF `imu_world -> base_link` | TF | optional transform matching the odometry |

The default launch uses the built-in `cargo` profile:

```bash
source /opt/ros/humble/setup.bash
source install/setup.bash
ros2 launch localization_zoo_ros imu_motion_health.launch.py
```

Select another built-in profile or remap the input topic with launch
arguments:

```bash
ros2 launch localization_zoo_ros imu_motion_health.launch.py \
  profile:=vehicle imu_topic:=/sensors/imu/data publish_tf:=false
```

Available built-in profiles are `default`, `wearable`, `vehicle`, `machine`,
`cargo`, and `drone`. The optional `profile_file` parameter accepts the
dependency-free strict profile YAML used by the core SDK, for example:

```bash
ros2 launch localization_zoo_ros imu_motion_health.launch.py \
  profile:=default \
  profile_file:=$(ros2 pkg prefix localization_zoo_ros)/share/localization_zoo_ros/config/profiles/vehicle.yaml
```

All core detector and integration parameters are declared ROS parameters,
including `impact_accel_threshold`, `impact_gyro_threshold`,
`fall_freefall_threshold`, `vibration_rms_threshold`,
`stationary_gyro_threshold`, `moving_accel_threshold`, `max_gap_s`, and
`event_hold_duration_s`. Put overrides in a ROS parameter YAML file and pass
it with `params_file:=...`.

The attitude in `/imu/odom` comes from a VQF filter by default. Its
inclination RMSE on the BROAD benchmark is 0.70°.

- `vqf_attitude:=false` restores gyro integration with stationary leveling.
- `mag_topic:=/imu/mag` subscribes to `sensor_msgs/msg/MagneticField`. The
  latest field within `mag_max_age_s` (0.1 s) is attached to each IMU sample.
  The attitude in `/imu/odom` then has a magnetic heading in ENU (BROAD: 2.30°
  total RMSE), with VQF's magnetic disturbance rejection. `use_magnetometer`
  and `vqf_attitude_tau_mag_s` tune it.
- Calibrate the magnetometer first: `magnetometer_calibration_cli
  --profile-yaml mag.yaml` writes the hard/soft-iron keys. Pass that file as
  `profile_file`.
- `posture_confirmation:=true` adds one-shot `fall_confirmed` records to
  `/imu/events_json`. A fall is confirmed when a lasting posture change follows
  an impact.
- Tuning keys are also declared: `vqf_attitude_tau_acc_s`, `posture_*`, and
  the `stationary_bias_*` safeguards.

See the core SDK README (`papers/imu_motion_health`) for their meaning and
evaluation.

IMU timestamps are taken from `header.stamp`. A zero timestamp is rejected
with a throttled warning by default. For drivers that omit timestamps, use
`zero_stamp_policy:=receive_time` (or `receive_time_fallback:=true`) to use
the node clock. The receive-time option should only be used when the driver
cannot provide a monotonic sensor clock.

### Bias and temperature calibration

`imu_calibration_node` subscribes to `/imu_raw` and optional
`sensor_msgs/msg/Temperature` on `/imu/temperature`, applies the generated
per-axis constant/temperature gyro bias and accelerometer offset, and publishes
`/imu/calibrated`. The combined launch connects that output directly to Motion
& Health:

```bash
ros2 launch localization_zoo_ros imu_calibrated_motion_health.launch.py \
  calibration_params_file:=/absolute/path/to/imu_calibration.yaml
```

Generate the parameter file from `imu_calibration_v1` with
`calibration_validation.py export-ros`. If no temperature message has arrived,
the node safely applies the reference-temperature bias. The original message
header, orientation, and covariance are preserved.
