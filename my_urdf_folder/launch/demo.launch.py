import os
import pprint
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from moveit_configs_utils import MoveItConfigsBuilder
import xacro


def generate_launch_description():
    pkg_share = get_package_share_directory('easy')
    config_dir = os.path.join(pkg_share, 'config')

    print(f"[DEBUG] Config files: {os.listdir(config_dir)}")

    # Build MoveIt config
    moveit_config = (
        MoveItConfigsBuilder("ebamk2", package_name="easy")
        .robot_description(file_path="config/ebamk2.urdf.xacro")
        .robot_description_semantic(file_path="config/ebamk2.srdf")
        .robot_description_kinematics(file_path="config/kinematics.yaml")
        .joint_limits(file_path="config/joint_limits.yaml")
    #    .trajectory_execution(file_path="config/moveit_controllers.yaml")
        .planning_pipelines(pipelines=["ompl"])
        .to_moveit_configs()
    )



    # === CRITICAL FIX: Remove the offending empty controller_list ===
    moveit_dict = moveit_config.to_dict()
    
    # Verwijder lege of conflicterende builder lists
    if 'controller_list' in moveit_dict:
        del moveit_dict['controller_list']

    # Remove 'controller_list' if it's empty (prevents the tuple error)
    if 'controller_list' in moveit_dict and not moveit_dict['controller_list']:
        print("[FIX] Removing empty 'controller_list' to avoid parameter type error")
        del moveit_dict['controller_list']

    print("\n=== CLEANED MOVEIT CONFIG DICT (key parts) ===")
    pprint.pprint({
        k: v for k, v in moveit_dict.items() 
        if k in ['controller_list', 'moveit_controller_manager', 'moveit_manage_controllers']
    }, width=120)
    print("=================================\n")

    # Manual robot_description (more reliable)
    xacro_path = os.path.join(config_dir, 'ebamk2.urdf.xacro')
    robot_description = xacro.process_file(xacro_path).toxml()

    # Nodes
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
       # output='both',
        output='screen',
        #parameters=[{'robot_description': robot_description, 'publish_frequency': 30.0}],
        parameters=[{'robot_description': robot_description}],
    )

    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', 'world', 'base_link'],
        output='log',
    )


    joint_bridge_node = Node(
       package='easy',
       executable='joint_command_bridge.py',
       output='screen'
    )


    moveit_dict = moveit_config.to_dict()

    # Verwijder lege of conflicterende builder lists
    if 'controller_list' in moveit_dict:
        del moveit_dict['controller_list']

    # CRUCIAL: Definieer het pad naar je moveit_controllers.yaml correct
    moveit_controllers_yaml_path = os.path.join(config_dir, "moveit_controllers.yaml")

    # Voeg de simpele controller manager parameters handmatig samen
    move_group_parameters = [
        moveit_dict,                  # Basisinstellingen (URDF, SRDF, OMPL)
        moveit_controllers_yaml_path, # Laadt de ebamk2_controller en joints in
        {
            # Schakel ros2_control integratie binnen MoveIt uit om de C++ service error te voorkomen
            "use_ros2_control": False,
            # Overschrijf de builder en forceer de Simpele Controller Manager
            "moveit_controller_manager": "moveit_simple_controller_manager/MoveItSimpleControllerManager",
            "moveit_manage_controllers": False
        }
    ]

    move_group_node = Node(
        package='moveit_ros_move_group',
        executable='move_group',
        output='screen',
        parameters=move_group_parameters,
        arguments=['--ros-args', '--log-level', 'info'],
        remappings=[('/joint_states', '/joint_states')],
    )



    # RViz with proper parameters (this fixes the SRDF error)
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', os.path.join(config_dir, 'moveit.rviz')],
        parameters=[
            {'robot_description': robot_description},                    # URDF
            {'robot_description_semantic': moveit_dict.get('robot_description_semantic', '')},  # SRDF
            moveit_config.robot_description_kinematics,                  # optional but helpful
            moveit_config.planning_pipelines,
        ],
    )

    # ros2_control
    ros2_controllers_path = os.path.join(config_dir, "ros2_controllers.yaml")

    ros2_control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[{'robot_description': robot_description}, ros2_controllers_path],
        output="both",
    #    remappings=[("/controller_manager/robot_description", "/robot_description")],
        # Zorg dat de controller manager luistert naar jouw echte micro-ROS encoders:
  #       remappings=[
  #       ("~/robot_description", "/robot_description")
  #      ("/joint_states", "/joint_states"),
        
        # Zorg dat de commando's van de controller direct op jouw echte topic belanden:
  #      ("/ebamk2_controller/joint_trajectory", "/joint_commands") 
  #       ],
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
    )

    # Make sure this name matches exactly the controller in ros2_controllers.yaml
    arm_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["ebamk2_controller", "--controller-manager", "/controller_manager"],
    )

    return LaunchDescription([
        robot_state_publisher,
        static_tf,
        move_group_node,
        rviz_node,
        ros2_control_node,
        joint_state_broadcaster_spawner,
        arm_controller_spawner,
    ])
