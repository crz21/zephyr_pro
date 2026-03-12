#ifdef(CONFIG_DEBUG_UART)
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "main.h"

#define USER_NODE DT_PATH(zephyr_user)
const struct device* uart_dev = DEVICE_DT_GET(DT_PROP(USER_NODE, debug_uart));

void debug_uart_thread(void)
{
    if (!device_is_ready(uart_dev)) {
        printk("uart not ready\n");
        return;
    }

    for (;;) {
        k_msleep(10);
    }
}
K_THREAD_DEFINE(debug_uart_thread_id, 1024, debug_uart_thread, NULL, NULL, NULL, DEBUG_UART_PRIORITY, 0, 0);

#endif
