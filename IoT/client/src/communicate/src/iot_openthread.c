#include <zephyr/kernel.h>

#include "main.h"

LOG_MODULE_REGISTER(coap_client, CONFIG_COAP_CLIENT_LOG_LEVEL);

#define OT_CONNECTION_LED DK_LED1
static void on_ot_connect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_on(OT_CONNECTION_LED);
}

static void on_ot_disconnect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_off(OT_CONNECTION_LED);
}

void openthread_thread(void)
{
    coap_client_utils_init(on_ot_connect, on_ot_disconnect, on_mtd_mode_toggle);

    for (;;) {
        k_msleep(1);
    }
}
K_THREAD_DEFINE(openthread_thread_id, 1024, openthread_thread, NULL, NULL, NULL, OPENTHREAD_PRIORITY, 0, 0);


