"""Launch one stateful paper pipeline with its native parameter defaults."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

METHODS = ('suma', 'imls_slam', 'dilo', 'litamin2')


def launch_node(context):
    method = LaunchConfiguration('method').perform(context)
    params_file = LaunchConfiguration('params_file').perform(context)
    parameters = [params_file] if params_file else []
    parameters.append({
        'map_frame_id': LaunchConfiguration('map_frame_id'),
        'publish_tf': ParameterValue(LaunchConfiguration('publish_tf'), value_type=bool),
        'use_sim_time': ParameterValue(LaunchConfiguration('use_sim_time'), value_type=bool),
    })
    return [Node(package='localization_zoo_ros', executable=f'{method}_paper_node',
                 output='screen', parameters=parameters,
                 remappings=[('points_raw', LaunchConfiguration('points_topic')),
                             ('odom', LaunchConfiguration('odom_topic'))])]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('method', default_value='imls_slam', choices=METHODS),
        DeclareLaunchArgument('points_topic', default_value='points_raw'),
        DeclareLaunchArgument('odom_topic', default_value='odom'),
        DeclareLaunchArgument('map_frame_id', default_value='map'),
        DeclareLaunchArgument('publish_tf', default_value='true'),
        DeclareLaunchArgument('use_sim_time', default_value='false'),
        DeclareLaunchArgument('params_file', default_value=''),
        OpaqueFunction(function=launch_node),
    ])
