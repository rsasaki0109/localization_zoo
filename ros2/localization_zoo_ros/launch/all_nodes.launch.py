from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    algorithm = LaunchConfiguration('algorithm')

    return LaunchDescription([
        DeclareLaunchArgument(
            'algorithm', default_value='litamin2',
            description='Algorithm to use: litamin2, aloam, ct_icp, relead, xicp'
        ),
        DeclareLaunchArgument('points_topic', default_value='points_raw'),

        # LiTAMIN2
        Node(
            package='localization_zoo_ros',
            executable='litamin2_node',
            name='litamin2_node',
            output='screen',
            parameters=[{
                'voxel_resolution': 3.0,
                'use_cov_cost': True,
            }],
            remappings=[('points_raw', LaunchConfiguration('points_topic'))],
            condition=launch.conditions.IfCondition(
                launch.substitutions.PythonExpression(
                    ["'", algorithm, "' == 'litamin2'"]
                )
            ),
        ),
    ])
