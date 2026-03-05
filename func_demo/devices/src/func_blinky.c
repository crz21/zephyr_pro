#if defined(CONFIG_CUSTOM_BLINKY)
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "main.h"

#define SLEEP_TIME_MS (2000)
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void func_blinky_thread(void)
{
    int ret;
    bool led_state = true;

    if (!gpio_is_ready_dt(&led)) {
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return;
    }

    for (;;) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            return;
        }

        led_state = !led_state;
        // printf("LED state: %s\n", led_state ? "ON" : "OFF");
        k_msleep(SLEEP_TIME_MS);
    }
}

K_THREAD_DEFINE(func_blinky_thread_id, 512, func_blinky_thread, NULL, NULL, NULL, 5, 0, 0);

#endif
