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

IMU timestamps are taken from `header.stamp`. A zero timestamp is rejected
with a throttled warning by default. For drivers that omit timestamps, use
`zero_stamp_policy:=receive_time` (or `receive_time_fallback:=true`) to use
the node clock. The receive-time option should only be used when the driver
cannot provide a monotonic sensor clock.
