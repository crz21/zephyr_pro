#ifdef(CONFIG_OPENTHREAD_IOT)

#include <coap_utils.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "main.h"

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
        return ret;
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
