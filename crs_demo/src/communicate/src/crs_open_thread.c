#include <coap_utils.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "main.h"

void openthread_thread(void)
{
    ret = coap_init();
    if (ret) {
        return ret;
    }

    for (;;) {
    }
}
K_THREAD_DEFINE(openthread_thread_id, 1024, openthread_thread, NULL, NULL, NULL, OPENTHREAD_PRIORITY, 0, 0);
