"""全アルゴリズムを個別に実行して比較するベンチマーク用launch

使い方:
  ros2 launch localization_zoo_ros benchmark.launch.py \
    algorithm:=litamin2 \
    bag_path:=/path/to/bag \
    points_topic:=/velodyne_points \
    output_dir:=/tmp/benchmark
"""

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument, ExecuteProcess, GroupAction,
    LogInfo, OpaqueFunction, TimerAction,
)
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


ALGORITHMS = {
    'litamin2': {
        'executable': 'litamin2_node',
        'params': {'voxel_resolution': 3.0, 'use_cov_cost': True},
    },
    'aloam': {
        'executable': 'aloam_node',
        'params': {'n_scans': 64, 'line_resolution': 0.4},
    },
    'ct_icp': {
        'executable': 'ct_icp_node',
        'params': {'voxel_resolution': 1.0, 'max_iterations': 30},
    },
    'relead': {
        'executable': 'relead_node',
        'params': {'max_iterations': 5},
    },
    'xicp': {
        'executable': 'xicp_node',
        'params': {'max_iterations': 30, 'kappa_1': 250.0},
    },
}


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('algorithm', default_value='litamin2'),
        DeclareLaunchArgument('bag_path', description='rosbag2 path'),
        DeclareLaunchArgument('points_topic', default_value='/points_raw'),
        DeclareLaunchArgument('imu_topic', default_value='/imu_raw'),

        LogInfo(msg=['Running algorithm: ', LaunchConfiguration('algorithm')]),
        LogInfo(msg=['Bag: ', LaunchConfiguration('bag_path')]),

        # アルゴリズムノードと odom を bag に記録
        ExecuteProcess(
            cmd=['ros2', 'bag', 'record', '/odom', '/map_odom', '/degeneracy',
                 '-o', '/tmp/localization_zoo_result'],
            output='screen',
        ),
    ])
