import os
import xacro
import yaml
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def load_yaml(package_name, file_path):
    package_path = get_package_share_directory(package_name)
    full_path = os.path.join(package_path, file_path)
    try:
        with open(full_path, 'r') as f:
            return yaml.safe_load(f)
    except Exception as e:
        print(f"Warning: Could not load {file_path}: {e}")
        return {}

def generate_launch_description():
    pkg_share = get_package_share_directory('easy')

    # Robot Description
    xacro_file = os.path.join(pkg_share, 'config', 'ebamk2.urdf.xacro')
    robot_description_config = xacro.process_file(xacro_file).toxml()

    # SRDF
    srdf_file = os.path.join(pkg_share, 'config', 'ebamk2.srdf')
    with open(srdf_file, 'r') as f:
        robot_description_semantic = f.read()

    kinematics_yaml = load_yaml('easy', 'config/kinematics.yaml')

    # === OMPL CONFIG (this is the critical part) ===
    ompl_config = load_yaml('easy', 'config/ompl_planning.yaml')

    parameters = [
        {'robot_description': robot_description_config},
        {'robot_description_semantic': robot_description_semantic},
        {'robot_description_kinematics': kinematics_yaml},
        {'planning_pipelines': ['ompl']},
        {'default_planning_pipeline': 'ompl'},
        {'publish_planning_scene': True},
        {'use_sim_time': False},
    ]

    # Jazzy fix: explicitly add the ompl dict
    if ompl_config:
        parameters.append({'ompl': ompl_config})

    return LaunchDescription([
        Node(
            package='moveit_ros_move_group',
            executable='move_group',
            name='move_group',          # explicit name helps
            output='screen',
            parameters=parameters,
            arguments=['--ros-args', '--log-level', 'warn'],
        ),
    ])
