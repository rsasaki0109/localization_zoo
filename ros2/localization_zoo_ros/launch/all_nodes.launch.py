from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
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
    'gicp': {
        'executable': 'gicp_node',
        'params': {'max_correspondence_distance': 3.0, 'max_iterations': 30},
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
    'fast_lio2': {
        'executable': 'fast_lio2_node',
        'params': {
            'voxel_resolution': 1.0,
            'max_iterations': 8,
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
        PythonExpression([
            "'",
            algorithm,
            "' == '",
            name,
            "' or '",
            algorithm,
            "' == 'all'",
        ])
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

    actions = [
        DeclareLaunchArgument(
            'algorithm',
            default_value='all',
            description='Algorithm to use: all, litamin2, aloam, ct_icp, gicp, ct_lio, fast_lio2, relead, xicp',
        ),
        DeclareLaunchArgument('points_topic', default_value='points_raw'),
        DeclareLaunchArgument('imu_topic', default_value='imu_raw'),
        DeclareLaunchArgument('use_sim_time', default_value='false'),
    ]

    for name, config in ALGORITHMS.items():
        actions.append(
            _make_node(name, config, algorithm, points_topic, imu_topic, use_sim_time)
        )

    return LaunchDescription(actions)
