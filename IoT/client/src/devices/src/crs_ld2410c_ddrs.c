#ifdef(CONFIG_LD2410C_DDR)
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "main.h"

#define LD2410C_UART_NODE DT_PATH(zephyr_user)
const struct device* ld2410c_uart_dev = DEVICE_DT_GET(DT_PROP(LD2410C_UART_NODE, ld2410c_uart));

void ld2410c_ddrs_thread(void)
{
    if (!device_is_ready(ld2410c_uart_dev)) {
        printk("ld2410c uart not ready\n");
        return;
    }

    for (;;) {
        // uart_tx(ld2410c_uart_dev, test_buf, sizeof(test_buf), 0);
        k_msleep(1000);
    }
}

K_THREAD_DEFINE(ld2410c_ddrs_thread_id, 1024, ld2410c_ddrs_thread, NULL, NULL, NULL, LD2410C_DDRS_PRIORITY, 0, 0);
#endif
