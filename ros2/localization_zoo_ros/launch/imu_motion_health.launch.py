"""Run the LiDAR-free IMU Motion & Health node."""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    default_params = PathJoinSubstitution(
        [FindPackageShare("localization_zoo_ros"), "config", "imu_motion_health.yaml"]
    )

    params_file = LaunchConfiguration("params_file")
    profile = LaunchConfiguration("profile")
    profile_file = LaunchConfiguration("profile_file")
    imu_topic = LaunchConfiguration("imu_topic")
    odom_topic = LaunchConfiguration("odom_topic")
    health_topic = LaunchConfiguration("health_topic")
    events_topic = LaunchConfiguration("events_topic")
    world_frame_id = LaunchConfiguration("world_frame_id")
    base_frame_id = LaunchConfiguration("base_frame_id")
    publish_tf = LaunchConfiguration("publish_tf")
    zero_stamp_policy = LaunchConfiguration("zero_stamp_policy")
    receive_time_fallback = LaunchConfiguration("receive_time_fallback")
    use_sim_time = LaunchConfiguration("use_sim_time")

    node = Node(
        package="localization_zoo_ros",
        executable="imu_motion_health_node",
        name="imu_motion_health_node",
        output="screen",
        parameters=[
            params_file,
            {
                "profile": profile,
                "profile_file": profile_file,
                "imu_topic": imu_topic,
                "odom_topic": odom_topic,
                "health_topic": health_topic,
                "events_topic": events_topic,
                "world_frame_id": world_frame_id,
                "base_frame_id": base_frame_id,
                "publish_tf": publish_tf,
                "zero_stamp_policy": zero_stamp_policy,
                "receive_time_fallback": receive_time_fallback,
                "use_sim_time": use_sim_time,
            },
        ],
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "params_file",
                default_value=default_params,
                description="ROS parameter YAML file for the IMU node",
            ),
            DeclareLaunchArgument(
                "profile",
                default_value="cargo",
                description="Built-in profile: default, wearable, vehicle, machine, cargo, or drone",
            ),
            DeclareLaunchArgument(
                "profile_file",
                default_value="",
                description="Optional strict IMU profile YAML loaded by the C++ core",
            ),
            DeclareLaunchArgument("imu_topic", default_value="/imu_raw"),
            DeclareLaunchArgument("odom_topic", default_value="/imu/odom"),
            DeclareLaunchArgument("health_topic", default_value="/imu/health_json"),
            DeclareLaunchArgument("events_topic", default_value="/imu/events_json"),
            DeclareLaunchArgument("world_frame_id", default_value="imu_world"),
            DeclareLaunchArgument("base_frame_id", default_value="base_link"),
            DeclareLaunchArgument("publish_tf", default_value="true"),
            DeclareLaunchArgument(
                "zero_stamp_policy",
                default_value="reject",
                description="reject or receive_time",
            ),
            DeclareLaunchArgument(
                "receive_time_fallback",
                default_value="false",
                description="Compatibility boolean enabling receive-time fallback for zero stamps",
            ),
            DeclareLaunchArgument("use_sim_time", default_value="false"),
            node,
        ]
    )
