#ifdef CONFIG_OPEN_THREAD
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "main.h"
#ifdef CONFIG_OT_COAP_SAMPLE_SW
#include <zephyr/drivers/gpio.h>

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	coap_led_set_state("ff03::1", 0, LED_MSG_STATE_TOGGLE);
}
#endif /* CONFIG_OT_COAP_SAMPLE_SW */

void openthread_thread(void)
{
    int ret = 0;
#ifdef CONFIG_OT_COAP_SAMPLE_SERVER
#ifdef CONFIG_OT_COAP_SAMPLE_LED
    coap_led_reg_rsc();
#endif /* CONFIG_OT_COAP_SAMPLE_LED */
#ifdef CONFIG_OT_COAP_SAMPLE_SW
    coap_btn_reg_rsc();
#endif /* CONFIG_OT_COAP_SAMPLE_SW */
#endif /* CONFIG_OT_COAP_SAMPLE_SERVER */

    ret = coap_init();
    if (ret) {
        return;
    }

#ifdef CONFIG_OT_COAP_SAMPLE_SW
    button_init(&button);

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback_dt(&button, &button_cb_data);
#endif /*CONFIG_OT_COAP_SAMPLE_SW */

    for (;;) {
        k_msleep(1);
    }
}
K_THREAD_DEFINE(openthread_thread_id, 1024, openthread_thread, NULL, NULL, NULL, OPENTHREAD_PRIORITY, 0, 0);

#endif
