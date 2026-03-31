"""rosbag再生 + 任意のlocalization_zooノードを同時起動するlaunch"""

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, TimerAction
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node


ALGORITHMS = {
    'litamin2': {
        'executable': 'litamin2_node',
        'params': {'voxel_resolution': 3.0, 'use_cov_cost': True},
        'needs_imu': False,
    },
    'aloam': {
        'executable': 'aloam_node',
        'params': {'n_scans': 64, 'line_resolution': 0.4},
        'needs_imu': False,
    },
    'ct_icp': {
        'executable': 'ct_icp_node',
        'params': {'voxel_resolution': 1.0, 'max_iterations': 30},
        'needs_imu': False,
    },
    'ct_lio': {
        'executable': 'ct_lio_node',
        'params': {
            'voxel_resolution': 1.0,
            'max_iterations': 20,
            'scan_period': 0.1,
        },
        'needs_imu': True,
    },
    'relead': {
        'executable': 'relead_node',
        'params': {'max_iterations': 5},
        'needs_imu': True,
    },
    'xicp': {
        'executable': 'xicp_node',
        'params': {'max_iterations': 30, 'kappa_1': 250.0},
        'needs_imu': False,
    },
}


def _algorithm_condition(algorithm, name):
    return IfCondition(
        PythonExpression(["'", algorithm, "' == '", name, "'"])
    )


def _make_node(name, config, algorithm, points_topic, imu_topic, use_sim_time):
    remappings = [('points_raw', points_topic)]
    if config['needs_imu']:
        remappings.append(('imu_raw', imu_topic))

    return Node(
        package='localization_zoo_ros',
        executable=config['executable'],
        name=config['executable'],
        output='screen',
        parameters=[config['params'], {'use_sim_time': use_sim_time}],
        remappings=remappings,
        condition=_algorithm_condition(algorithm, name),
    )


def generate_launch_description():
    algorithm = LaunchConfiguration('algorithm')
    points_topic = LaunchConfiguration('points_topic')
    imu_topic = LaunchConfiguration('imu_topic')
    use_sim_time = LaunchConfiguration('use_sim_time')
    bag_path = LaunchConfiguration('bag_path')
    playback_rate = LaunchConfiguration('playback_rate')

    actions = [
        DeclareLaunchArgument('bag_path', description='Path to rosbag2 directory'),
        DeclareLaunchArgument(
            'algorithm',
            default_value='litamin2',
            description='litamin2 / aloam / ct_icp / ct_lio / relead / xicp',
        ),
        DeclareLaunchArgument(
            'points_topic',
            default_value='/points_raw',
            description='PointCloud2 topic name',
        ),
        DeclareLaunchArgument(
            'imu_topic',
            default_value='/imu_raw',
            description='IMU topic name (for ct_lio / relead)',
        ),
        DeclareLaunchArgument('playback_rate', default_value='1.0'),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
    ]

    for name, config in ALGORITHMS.items():
        actions.append(
            _make_node(name, config, algorithm, points_topic, imu_topic, use_sim_time)
        )

    actions.append(
        TimerAction(
            period=2.0,
            actions=[
                ExecuteProcess(
                    cmd=[
                        'ros2', 'bag', 'play',
                        bag_path,
                        '--clock',
                        '--rate', playback_rate,
                    ],
                    output='screen',
                ),
            ],
        )
    )

    return LaunchDescription(actions)
