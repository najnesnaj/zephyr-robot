#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/uart/cdc_acm.h>

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

static const struct device *cdc_acm_dev;

static void uart_print(const struct device *dev, const char *str)
{
    if (!dev || !device_is_ready(dev)) {
        return;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        uart_poll_out(dev, str[i]);
    }
}

#define STEP PWM_USEC(100)

enum direction {
	DOWN,
	UP,
};

int main(void)
{
	uint32_t pulse_width = min_pulse;
       // uint32_t pulse_width = 700000; 
	enum direction dir = UP;
	int ret;

	cdc_acm_dev = DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart0));

	if (!device_is_ready(cdc_acm_dev)) {
		printk("CDC ACM device not ready\n");
		return 0;
	}

	uart_print(cdc_acm_dev, "Gripper servo control (USB CDC)\r\n");

	printk("Gripper servo control\n");

	if (!pwm_is_ready_dt(&servo)) {
		printk("Error: PWM device %s is not ready\n", servo.dev->name);
		uart_print(cdc_acm_dev, "Error: PWM device not ready\r\n");
		return 0;
	}

	while (1) {
		ret = pwm_set_pulse_dt(&servo, pulse_width);
		if (ret < 0) {
			printk("Error %d: failed to set pulse width\n", ret);
			uart_print(cdc_acm_dev, "Error: failed to set pulse width\r\n");
			return 0;
		}

		printk("Pulse width: %d us\n", pulse_width);
		{
			char buf[64];
			int n = snprintf(buf, sizeof(buf), "Pulse width: %d us\r\n", pulse_width);
			if (n > 0) {
				uart_print(cdc_acm_dev, buf);
			}
		}

		if (dir == DOWN) {
			if (pulse_width <= min_pulse) {
				dir = UP;
				pulse_width = min_pulse;
			} else {
				pulse_width -= STEP;
			}
		} else {
			pulse_width += STEP;
			if (pulse_width >= max_pulse) {
				dir = DOWN;
				pulse_width = max_pulse;
			}
		}
		k_sleep(K_SECONDS(1));
	}
	return 0;
}
