#include <zephyr/kernel.h>
#include <zephyr/drivers/stepper/stepper.h>
#include <zephyr/drivers/stepper/stepper_ctrl.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <sensor_msgs/msg/joint_state.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>
#include <rcl_interfaces/msg/log.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){LOG_ERR("Failed status on line %d: %d.",__LINE__,(int)temp_rc); return -1; }}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){LOG_WRN("Failed status on line %d: %d.",__LINE__,(int)temp_rc);}}

#define STEPPER_ENABLE_PIN 13
#define STEPPER0_NODE DT_NODELABEL(stepper0_control)
#define STEPPER1_NODE DT_NODELABEL(stepper1_control)
#define STEPPER2_NODE DT_NODELABEL(stepper2_control)

#define AS5600_1 DT_NODELABEL(as5600_1)
#define AS5600_2 DT_NODELABEL(as5600_2)
#define AS5600_3 DT_NODELABEL(as5600_3)

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

LOG_MODULE_REGISTER(joint_stepper_microros, LOG_LEVEL_INF);

#define MAX_JOINTS 10
#define ANGLE_TO_STEPS(angle_rad)  ((int32_t)((angle_rad) * 509.3))

const struct device *stepper0_dev;
const struct device *stepper1_dev;
const struct device *stepper2_dev;
static int32_t current_microsteps0 = 0;
static int32_t current_microsteps1 = 0;
static int32_t current_microsteps2 = 0;

struct k_sem stepper0_sem;
struct k_sem stepper1_sem;
struct k_sem stepper2_sem;

const struct device *usart1_dev;

rcl_subscription_t joint_commands_subscriber;
static sensor_msgs__msg__JointState joint_state_msg;

rcl_subscription_t gripper_commands_subscriber;
static std_msgs__msg__Int32 gripper_cmd_msg;

rcl_publisher_t log_publisher;
rcl_interfaces__msg__Log log_msg;

static struct sensor_value encoder1_angle;
static struct sensor_value encoder2_angle;
static struct sensor_value encoder3_angle;

rcl_publisher_t angle_publisher[3];
std_msgs__msg__Float32 angle_msg[3];

void micro_ros_log_init(rcl_node_t *node)
{
    log_msg.msg.capacity = 128;
    log_msg.msg.data = (char *)malloc(log_msg.msg.capacity * sizeof(char));
    log_msg.msg.size = 0;

    rcl_publisher_options_t pub_opts = rcl_publisher_get_default_options();
    rcl_ret_t ret = rcl_publisher_init(&log_publisher, node,
                       ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, Log),
                       "/micro_ros/log",
                       &pub_opts);
    if (ret != RCL_RET_OK) {
        printk("micro-ROS log publisher init failed: %d\n", ret);
    }
}

void micro_ros_log_publish(const char *msg, int level)
{
    uint64_t now_ns = rmw_uros_epoch_nanos();
    log_msg.stamp.sec = now_ns / 1000000000ULL;
    log_msg.stamp.nanosec = now_ns % 1000000000ULL;
    log_msg.level = level;

    rosidl_runtime_c__String__assign(&log_msg.msg, msg);

   (void)rcl_publish(&log_publisher, &log_msg, NULL);
}

#define LOG_INF_PUBLISH(...) do { \
    char buf[256]; \
    snprintf(buf, sizeof(buf), __VA_ARGS__); \
    LOG_INF(__VA_ARGS__); \
    micro_ros_log_publish(buf, 20); \
    if (usart1_dev && device_is_ready(usart1_dev)) { \
        for (int i = 0; buf[i] != '\0'; i++) { \
            uart_poll_out(usart1_dev, buf[i]); \
        } \
        uart_poll_out(usart1_dev, '\r'); \
        uart_poll_out(usart1_dev, '\n'); \
    } \
} while(0)

#define LOG_ERR_PUBLISH(...) do { \
    char buf[256]; \
    snprintf(buf, sizeof(buf), __VA_ARGS__); \
    LOG_ERR(__VA_ARGS__); \
    micro_ros_log_publish(buf, 40); \
    if (usart1_dev && device_is_ready(usart1_dev)) { \
        for (int i = 0; buf[i] != '\0'; i++) { \
            uart_poll_out(usart1_dev, buf[i]); \
        } \
        uart_poll_out(usart1_dev, '\r'); \
        uart_poll_out(usart1_dev, '\n'); \
    } \
} while(0)

#define LOG_WRN_PUBLISH(...) do { \
    char buf[256]; \
    snprintf(buf, sizeof(buf), __VA_ARGS__); \
    LOG_WRN(__VA_ARGS__); \
    micro_ros_log_publish(buf, 30); \
} while(0)

static void stepper0_callback(const struct device *dev,
                              enum stepper_ctrl_event event,
                              void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);

    if (event == STEPPER_CTRL_EVENT_STEPS_COMPLETED) {
        k_sem_give(&stepper0_sem);
    }
}

static void stepper1_callback(const struct device *dev,
                              enum stepper_ctrl_event event,
                              void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);

    if (event == STEPPER_CTRL_EVENT_STEPS_COMPLETED) {
        k_sem_give(&stepper1_sem);
    }
}

static void stepper2_callback(const struct device *dev,
                              enum stepper_ctrl_event event,
                              void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);

    if (event == STEPPER_CTRL_EVENT_STEPS_COMPLETED) {
        k_sem_give(&stepper2_sem);
    }
}

void gripper_commands_callback(const void *msgin)
{
    const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;
    uint32_t pulse_width;

    if (msg->data < 0) {
        pulse_width = min_pulse;
    } else if (msg->data > 180) {
        pulse_width = max_pulse;
    } else {
        pulse_width = min_pulse + (msg->data * (max_pulse - min_pulse) / 180);
    }

    int ret = pwm_set_pulse_dt(&servo, pulse_width);
    if (ret != 0) {
        LOG_ERR_PUBLISH("Servo PWM set failed: %d", ret);
    } else {
        LOG_INF_PUBLISH("Gripper set to %d (pulse: %d us)", (int)msg->data, (int)pulse_width);
    }
}

void joint_commands_callback(const void *msgin)
{
    const sensor_msgs__msg__JointState *msg = (const sensor_msgs__msg__JointState *)msgin;

    size_t n = msg->position.size;
    if (n < 3) {
        LOG_WRN_PUBLISH("JointState: need >=3 positions, got %zu", n);
        return;
    }
    if (n > MAX_JOINTS) {
        LOG_WRN_PUBLISH("JointState: %zu positions truncated to %d", n, MAX_JOINTS);
        n = MAX_JOINTS;
    }

    LOG_INF_PUBLISH("joint_commands_callback: %zu joints", n);

    int32_t target_steps0 = ANGLE_TO_STEPS(msg->position.data[0]);
    int32_t delta0 = target_steps0 - current_microsteps0;
    int ret0 = stepper_ctrl_move_by(stepper0_dev, delta0);
    if (ret0 != 0) {
        LOG_ERR_PUBLISH("Stepper0 move failed: %d", ret0);
    } else {
        current_microsteps0 = target_steps0;
    }

    int32_t target_steps1 = ANGLE_TO_STEPS(msg->position.data[1]);
    int32_t delta1 = target_steps1 - current_microsteps1;
    int ret1 = stepper_ctrl_move_by(stepper1_dev, delta1);
    if (ret1 != 0) {
        LOG_ERR_PUBLISH("Stepper1 move failed: %d", ret1);
    } else {
        current_microsteps1 = target_steps1;
    }

    int32_t target_steps2 = ANGLE_TO_STEPS(msg->position.data[2]);
    int32_t delta2 = target_steps2 - current_microsteps2;
    int ret2 = stepper_ctrl_move_by(stepper2_dev, delta2);
    if (ret2 != 0) {
        LOG_ERR_PUBLISH("Stepper2 move failed: %d", ret2);
    } else {
        current_microsteps2 = target_steps2;
    }
}

void angle_timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
    (void)last_call_time;
    (void)timer;

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

    LOG_INF_PUBLISH("Published encoder angles: %.2f, %.2f, %.2f", (double)enc1, (double)enc2, (double)enc3);
}

int main(void)
{
    usart1_dev = DEVICE_DT_GET(DT_NODELABEL(usart1));

    if (usart1_dev && device_is_ready(usart1_dev)) {
        const char *msg = "hello\r\n";
        for (int i = 0; msg[i] != '\0'; i++) {
            uart_poll_out(usart1_dev, msg[i]);
        }
    }

    LOG_INF_PUBLISH("Starting joint+encoder micro-ROS Controller...");

    /* === GPIO and Stepper initialization === */
    const struct device *gpioc_dev = DEVICE_DT_GET(DT_NODELABEL(gpioc));
    if (!device_is_ready(gpioc_dev)) {
        LOG_ERR_PUBLISH("GPIO device not ready");
        return 0;
    }

    gpio_pin_configure(gpioc_dev, STEPPER_ENABLE_PIN, GPIO_OUTPUT_INACTIVE);

    stepper0_dev = DEVICE_DT_GET(STEPPER0_NODE);
    if (!device_is_ready(stepper0_dev)) {
        LOG_ERR_PUBLISH("Stepper0 device not ready");
        return 0;
    }

    stepper1_dev = DEVICE_DT_GET(STEPPER1_NODE);
    if (!device_is_ready(stepper1_dev)) {
        LOG_ERR_PUBLISH("Stepper1 device not ready");
        return 0;
    }

    stepper2_dev = DEVICE_DT_GET(STEPPER2_NODE);
    if (!device_is_ready(stepper2_dev)) {
        LOG_ERR_PUBLISH("Stepper2 device not ready");
        return 0;
    }

    stepper_ctrl_set_event_cb(stepper0_dev, stepper0_callback, NULL);
    stepper_ctrl_set_reference_position(stepper0_dev, 0);
    stepper_ctrl_set_microstep_interval(stepper0_dev, 1500000);

    stepper_ctrl_set_event_cb(stepper1_dev, stepper1_callback, NULL);
    stepper_ctrl_set_reference_position(stepper1_dev, 0);
    stepper_ctrl_set_microstep_interval(stepper1_dev, 1500000);

    stepper_ctrl_set_event_cb(stepper2_dev, stepper2_callback, NULL);
    stepper_ctrl_set_reference_position(stepper2_dev, 0);
    stepper_ctrl_set_microstep_interval(stepper2_dev, 1500000);

    /* === AS5600 sensor initialization === */
    const struct device *as5600_1_dev = DEVICE_DT_GET(AS5600_1);
    if (!device_is_ready(as5600_1_dev)) {
        LOG_ERR_PUBLISH("AS5600_1 device not ready");
        return 0;
    }

    const struct device *as5600_2_dev = DEVICE_DT_GET(AS5600_2);
    if (!device_is_ready(as5600_2_dev)) {
        LOG_ERR_PUBLISH("AS5600_2 device not ready");
        return 0;
    }

    const struct device *as5600_3_dev = DEVICE_DT_GET(AS5600_3);
    if (!device_is_ready(as5600_3_dev)) {
        LOG_ERR_PUBLISH("AS5600_3 device not ready");
        return 0;
    }

    LOG_INF_PUBLISH("All AS5600 encoders ready");

    if (!pwm_is_ready_dt(&servo)) {
        LOG_ERR_PUBLISH("Servo PWM device not ready");
    } else {
        LOG_INF_PUBLISH("Servo PWM device ready");
    }

    /* === micro-ROS transport === */
    static zephyr_transport_params_t transport_params = {0};
    rmw_uros_set_custom_transport(
        MICRO_ROS_FRAMING_REQUIRED,
        (void *)&transport_params,
        zephyr_transport_open,
        zephyr_transport_close,
        zephyr_transport_write,
        zephyr_transport_read
    );

    /* === micro-ROS support with retry === */
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    rcl_ret_t rc = RCL_RET_ERROR;

    for (int i = 0; i < 10; i++) {
        rc = rclc_support_init(&support, 0, NULL, &allocator);
        if (rc == RCL_RET_OK) {
            LOG_INF_PUBLISH("micro-ROS support initialized successfully!");
            break;
        }
        LOG_WRN_PUBLISH("rclc_support_init failed (%d), retry %d/10...", rc, i + 1);
        k_msleep(1000);
    }

    if (rc != RCL_RET_OK) {
        LOG_ERR_PUBLISH("Failed to initialize micro-ROS after 10 attempts. Is the Agent running?");
        return 0;
    }

    /* === Create node === */
    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "joint_encoder_stepper_node", "", &support));

    /* === micro-ROS logging === */
    micro_ros_log_init(&node);

    /* === Publishers (AS5600 angles) === */
    RCCHECK(rclc_publisher_init_default(&angle_publisher[0], &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder1/angle"));
    RCCHECK(rclc_publisher_init_default(&angle_publisher[1], &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder2/angle"));
    RCCHECK(rclc_publisher_init_default(&angle_publisher[2], &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32), "/encoder3/angle"));

    LOG_INF_PUBLISH("AS5600 angle publishers initialized");

    /* === Timer for periodic encoder publishing === */
    rcl_timer_t angle_timer;
    RCCHECK(rclc_timer_init_default2(&angle_timer, &support,
        RCL_MS_TO_NS(1000), angle_timer_callback, true));

    /* === Subscribers === */
    sensor_msgs__msg__JointState__init(&joint_state_msg);
    RCCHECK(rclc_subscription_init_default(
        &joint_commands_subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
        "/joint_commands"));

    std_msgs__msg__Int32__init(&gripper_cmd_msg);
    RCCHECK(rclc_subscription_init_default(
        &gripper_commands_subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "/gripper_cmd"));

    /* === Allocate JointState message memory === */
    rcl_allocator_t alloc = rcl_get_default_allocator();
    joint_state_msg.name.capacity = MAX_JOINTS;
    joint_state_msg.name.size = 0;
    joint_state_msg.name.data = (rosidl_runtime_c__String *)
        alloc.allocate(MAX_JOINTS * sizeof(rosidl_runtime_c__String), alloc.state);
    if (!joint_state_msg.name.data) {
        LOG_ERR_PUBLISH("malloc failed for joint_state_msg.name.data");
        return 0;
    }
    memset(joint_state_msg.name.data, 0,
           MAX_JOINTS * sizeof(rosidl_runtime_c__String));
    for (size_t i = 0; i < MAX_JOINTS; i++) {
        joint_state_msg.name.data[i].capacity = 32;
        joint_state_msg.name.data[i].size = 0;
        joint_state_msg.name.data[i].data = (char *)
            alloc.allocate(32 * sizeof(char), alloc.state);
        if (!joint_state_msg.name.data[i].data) {
            LOG_ERR_PUBLISH("malloc failed for name[%zu].data", i);
            return 0;
        }
    }

    joint_state_msg.position.capacity = MAX_JOINTS;
    joint_state_msg.position.size = 0;
    joint_state_msg.position.data = (double *)
        alloc.allocate(MAX_JOINTS * sizeof(double), alloc.state);
    if (!joint_state_msg.position.data) {
        LOG_ERR_PUBLISH("malloc failed for joint_state_msg.position.data");
        return 0;
    }

    /* === Executor (2 subscriptions + 1 timer = 3 handles) === */
    rclc_executor_t executor = rclc_executor_get_zero_initialized_executor();
    RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));

    RCCHECK(rclc_executor_add_subscription(
        &executor, &joint_commands_subscriber, &joint_state_msg,
        &joint_commands_callback, ON_NEW_DATA));

    RCCHECK(rclc_executor_add_subscription(
        &executor, &gripper_commands_subscriber, &gripper_cmd_msg,
        &gripper_commands_callback, ON_NEW_DATA));

    RCCHECK(rclc_executor_add_timer(&executor, &angle_timer));

    LOG_INF_PUBLISH("Setup complete! Subscribed to /joint_commands and /gripper_cmd");
    LOG_INF_PUBLISH("Publishing to /encoder1/angle, /encoder2/angle, /encoder3/angle");

    /* === Main loop === */
    while (1) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        k_sleep(K_MSEC(10));
    }

    /* Cleanup */
    (void)rcl_subscription_fini(&joint_commands_subscriber, &node);
    (void)rcl_node_fini(&node);
    (void)rclc_support_fini(&support);

    return 0;
}
