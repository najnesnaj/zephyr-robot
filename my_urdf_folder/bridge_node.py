#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64
from sensor_msgs.msg import JointState

class AngleToJointStateBridge(Node):
    def __init__(self):
        super().__init__('angle_to_joint_state_bridge')
        self.joint_name = 'joint_3'  # CHANGE THIS TO YOUR URDF JOINT NAME (e.g., 'arm_joint')
        self.subscription = self.create_subscription(
            Float64,
            '/encoder1/angle',
            self.angle_callback,
            10)
        self.publisher = self.create_publisher(JointState, '/joint_states', 10)
        self.get_logger().info('Bridge node started. Listening to /encoder1_angle...')

    def angle_callback(self, msg):
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = [self.joint_name]
        joint_state.position = [msg.data]  # Assuming radians; convert if needed (e.g., msg.data * 3.14159 / 180 for degrees)
        joint_state.velocity = [0.0]  # Optional; set if you have velocity data
        joint_state.effort = [0.0]    # Optional
        self.publisher.publish(joint_state)
        self.get_logger().info(f'Published /joint_states: {joint_state.position[0]}')

def main(args=None):
    rclpy.init(args=args)
    node = AngleToJointStateBridge()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
