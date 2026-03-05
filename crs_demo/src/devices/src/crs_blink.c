#if defined(CONFIG_BLINK)
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/printk.h>

#include "main.h"

#define LED0_NODE DT_ALIAS(led0)
// #define LED1_NODE DT_ALIAS(led1)

struct led {
    struct gpio_dt_spec spec;
    uint8_t num;
};

static const struct led led0 = {
    .spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
    .num = 0,
};

// static const struct led led1 = {
//     .spec = GPIO_DT_SPEC_GET_OR(LED1_NODE, gpios, {0}),
//     .num = 1,
// };

void blink(const struct led* led, uint32_t sleep_ms)
{
    const struct gpio_dt_spec* spec = &led->spec;
    int ret;

    if (!device_is_ready(spec->port)) {
        printk("Error: %s device is not ready\n", spec->port->name);
        return;
    }

    ret = gpio_pin_configure_dt(spec, GPIO_OUTPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure pin %d (LED '%d')\n", ret, spec->pin, led->num);
        return;
    }

    for (;;) {
        gpio_pin_toggle(spec->port, spec->pin);
        k_msleep(sleep_ms);
    }
}

void blink0_thread(void) { blink(&led0, 1000); }

// void blink1_thread(void) { blink(&led1, 200); }
K_THREAD_DEFINE(blink0_thread_id, 512, blink0_thread, NULL, NULL, NULL, BLINK0_PRIORITY, 0, 0);
#endif
