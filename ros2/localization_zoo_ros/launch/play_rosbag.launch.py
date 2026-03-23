"""rosbag再生 + 任意のlocalization_zooノードを同時起動するlaunch"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, TimerAction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('bag_path', description='Path to rosbag2 directory'),
        DeclareLaunchArgument('algorithm', default_value='litamin2',
                              description='litamin2 / aloam / ct_icp / relead / xicp'),
        DeclareLaunchArgument('points_topic', default_value='/points_raw',
                              description='PointCloud2 topic name'),
        DeclareLaunchArgument('imu_topic', default_value='/imu_raw',
                              description='IMU topic name (for relead)'),

        # LiTAMIN2
        Node(
            package='localization_zoo_ros',
            executable='litamin2_node',
            name='litamin2_node',
            output='screen',
            parameters=[{'voxel_resolution': 3.0, 'use_cov_cost': True}],
            remappings=[('points_raw', LaunchConfiguration('points_topic'))],
        ),

        # rosbag再生 (2秒遅延して開始)
        TimerAction(
            period=2.0,
            actions=[
                ExecuteProcess(
                    cmd=['ros2', 'bag', 'play',
                         LaunchConfiguration('bag_path'),
                         '--clock', '--rate', '1.0'],
                    output='screen',
                ),
            ],
        ),
    ])
