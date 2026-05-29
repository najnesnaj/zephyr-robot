#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from control_msgs.msg import JointTrajectoryControllerState
from sensor_msgs.msg import JointState

class JointCommandBridge(Node):
    def __init__(self):
        super().__init__('joint_command_bridge')
        
        # Voeg hier EXACT de joint-namen toe zoals ze in je URDF staan
        self.joint_names = ['joint_1', 'joint_2', 'joint_3', 'joint_4', 'joint_6']
        
        # We luisteren naar de actuele/gewenste status van de actieve controller
        self.subscription = self.create_subscription(
            JointTrajectoryControllerState,
            '/ebamk2_controller/controller_state',
            self.listener_callback,
            10)
            
        # We publiceren naar jouw echte micro-ROS topic
        self.publisher = self.create_publisher(
            JointState, 
            '/joint_commands', 
            10)
        
        self.get_logger().info("Bridge tussen ros2_control en micro-ROS succesvol gestart!")

    def listener_callback(self, msg):
        # Gebruik de gewenste positie (reference) van de controller
        if not msg.reference.positions:
            return
            
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = self.joint_names
        joint_state.position = list(msg.reference.positions)

        self.publisher.publish(joint_state)

def main(args=None):
    rclpy.init(args=args)
    node = JointCommandBridge()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()

