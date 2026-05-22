#include <version.h>

#if ZEPHYR_VERSION_CODE >= ZEPHYR_VERSION(3,1,0)
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/posix/sys/time.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/uart/cdc_acm.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(microros_main, LOG_LEVEL_DBG);
#include <zephyr/usb/usb_device.h>
#else
#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <drivers/sensor.h>
#include <posix/sys/time.h>
#endif

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

#include <std_msgs/msg/float32.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); for(;;){} }}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

#define AS5600_1 DT_NODELABEL(as5600_1)
#define AS5600_2 DT_NODELABEL(as5600_2)
#define AS5600_3 DT_NODELABEL(as5600_3)

static struct sensor_value encoder1_angle;
static struct sensor_value encoder2_angle;
static struct sensor_value encoder3_angle;

rcl_publisher_t angle_publisher[3];
std_msgs__msg__Float32 angle_msg[3];

void angle_timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	(void) last_call_time;

	if (timer != NULL) {
		const struct device *as5600_1_dev = DEVICE_DT_GET(AS5600_1);
		const struct device *as5600_2_dev = DEVICE_DT_GET(AS5600_2);
		const struct device *as5600_3_dev = DEVICE_DT_GET(AS5600_3);

		sensor_sample_fetch(as5600_1_dev);
		sensor_channel_get(as5600_1_dev, SENSOR_CHAN_ROTATION, &encoder1_angle);

		sensor_sample_fetch(as5600_2_dev);
		sensor_channel_get(as5600_2_dev, SENSOR_CHAN_ROTATION, &encoder2_angle);

		sensor_sample_fetch(as5600_3_dev);
		sensor_channel_get(as5600_3_dev, SENSOR_CHAN_ROTATION, &encoder3_angle);

		float enc1 = (float)encoder1_angle.val1 + (float)encoder1_angle.val2 / 1000000.0f;
		float enc2 = (float)encoder2_angle.val1 + (float)encoder2_angle.val2 / 1000000.0f;
		float enc3 = (float)encoder3_angle.val1 + (float)encoder3_angle.val2 / 1000000.0f;

		angle_msg[0].data = enc1;
		angle_msg[1].data = enc2;
		angle_msg[2].data = enc3;

		RCSOFTCHECK(rcl_publish(&angle_publisher[0], (const void*)&angle_msg[0], NULL));
		RCSOFTCHECK(rcl_publish(&angle_publisher[1], (const void*)&angle_msg[1], NULL));
		RCSOFTCHECK(rcl_publish(&angle_publisher[2], (const void*)&angle_msg[2], NULL));
		printf("Published encoder1 angle: %.2f\n", enc1);
		printf("Published encoder2 angle: %.2f\n", enc2);
		printf("Published encoder3 angle: %.2f\n", enc3);
	}
}

int main(void)
{
	rmw_uros_set_custom_transport(
		MICRO_ROS_FRAMING_REQUIRED,
		(void *) &default_params,
		zephyr_transport_open,
		zephyr_transport_close,
		zephyr_transport_write,
		zephyr_transport_read
	);

	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "as5600_publisher_node", "", &support));

	RCCHECK(rclc_publisher_init_default(&angle_publisher[0], &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder1/angle"));
	RCCHECK(rclc_publisher_init_default(&angle_publisher[1], &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder2/angle"));
	RCCHECK(rclc_publisher_init_default(&angle_publisher[2], &node, ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder3/angle"));
	LOG_INF("Starting AS5600 angle publisher application...");

	rcl_timer_t timer;
	RCCHECK(rclc_timer_init_default(&timer, &support, RCL_MS_TO_NS(1000), angle_timer_callback));

	rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	rclc_executor_spin(&executor);

	for (int i = 0; i < 3; i++) {
		RCCHECK(rcl_publisher_fini(&angle_publisher[i], &node));
	}
	RCCHECK(rcl_node_fini(&node));

	return 0;
}
