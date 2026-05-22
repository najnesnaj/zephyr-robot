# Logging System

The firmware implements a **triple-output logging architecture**, sending diagnostic information to three independent backends simultaneously.

## The Three Layers

| Layer | Destination | Mechanism | ROS 2 Level |
|-------|-------------|-----------|-------------|
| 1 | Zephyr LOG subsystem | `LOG_INF` / `LOG_ERR` / `LOG_WRN` | — |
| 2 | `/micro_ros/log` topic | `rcl_interfaces/Log` publisher | 20 (INFO) / 30 (WARN) / 40 (ERROR) |
| 3 | USART1 (PA9) direct UART | `uart_poll_out()` polling | — |

## Convenience Macros

Three macros unify all three layers into single-line calls:

### `LOG_INF_PUBLISH(fmt, ...)`
1. Calls `LOG_INF` → Zephyr UART console (CDC ACM)
2. Publishes to `/micro_ros/log` with level 20 (INFO)
3. Echoes to USART1 via `uart_poll_out`

### `LOG_ERR_PUBLISH(fmt, ...)`
1. Calls `LOG_ERR`
2. Publishes to `/micro_ros/log` with level 40 (ERROR)
3. Echoes to USART1

### `LOG_WRN_PUBLISH(fmt, ...)`
1. Calls `LOG_WRN`
2. Publishes to `/micro_ros/log` with level 30 (WARN)
3. Does **not** echo to USART1

## Macro Implementation Example

```c
#define LOG_INF_PUBLISH(...) do {                    \
    char buf[256];                                    \
    snprintf(buf, sizeof(buf), __VA_ARGS__);          \
    LOG_INF(__VA_ARGS__);                             \
    micro_ros_log_publish(buf, 20);                   \
    if (usart1_dev && device_is_ready(usart1_dev)) {  \
        for (int i = 0; buf[i] != '\0'; i++) {        \
            uart_poll_out(usart1_dev, buf[i]);         \
        }                                             \
        uart_poll_out(usart1_dev, '\r');              \
        uart_poll_out(usart1_dev, '\n');              \
    }                                                 \
} while(0)
```

## micro-ROS Log Publisher

The function `micro_ros_log_init()` creates a publisher on `/micro_ros/log`:

```c
void micro_ros_log_init(rcl_node_t *node) {
    log_msg.msg.capacity = 128;
    log_msg.msg.data = (char *)malloc(log_msg.msg.capacity);
    rcl_publisher_init(&log_publisher, node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(rcl_interfaces, msg, Log),
        "/micro_ros/log", &pub_opts);
}
```

Each message contains:
- `stamp` — ROS 2 timestamp from `rmw_uros_epoch_nanos()`
- `level` — 20 (INFO), 30 (WARN), 40 (ERROR)
- `msg` — the formatted log string (up to 128 bytes)

## Data Flow Diagram

```
+-------------------+       +-------------------+
| Application code  | ----> | LOG_INF_PUBLISH   |
| (main.c)          |       | LOG_ERR_PUBLISH   |
+-------------------+       | LOG_WRN_PUBLISH   |
                            +--------+----------+
                                     |
                      +--------------+--------------+
                      |              |              |
                Zephyr LOG      snprintf        snprintf
                subsystem        into buf        into buf
                      |              |              |
                      v              v              v
                +----------+  +-----------+  +------------+
                | LOG_BE-  |  | micro-ROS |  | USART1     |
                | CKEND_   |  | publisher |  | (PA9 TX)   |
                | UART     |  | /micro_   |  | polling    |
                |          |  | ros/log   |  |            |
                +----------+  +-----------+  +------------+
                      |              |              |
                USB CDC ACM    ROS 2 DDS     Debug serial
                (virtual COM)  network        terminal
```

## Viewing Logs

**Zephyr console (USB CDC ACM):**
```bash
screen /dev/ttyACM0 115200
```

**ROS 2:**
```bash
ros2 topic echo /micro_ros/log
```

**USART1 debug:**
Connect a USB-UART adapter to PA9 (TX) and GND. Open at 115200 baud.
