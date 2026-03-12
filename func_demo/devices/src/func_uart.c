#ifdef(CONFIG_CUSTOM_UART)
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "main.h"

#define CUSTOM_UART_NODE DT_ALIAS(custom_uart)
const struct device* custom_uart_dev = DEVICE_DT_GET(CUSTOM_UART_NODE);
uint8_t tx_flag;
void tx_buf_xxx(uint8_t* buf) { uart_tx(custom_uart_dev, buf, 1, 0); }

void custom_uart_thread(void)
{
    uint8_t test_buf[10] = {0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36};
    // uint8_t rec_val = 0;

    if (!device_is_ready(custom_uart_dev)) {
        // printk("custom uart not ready\n");
        return;
    }

    for (;;) {
        // uart_poll_in(custom_uart_dev, &rec_val);
        // if (rec_val == 0x37) {
        //     rec_val = 0;
        //     for (uint8_t i = 0; i < sizeof(test_buf); i++) {
        //         uart_poll_out(custom_uart_dev, test_buf[i]);  // 轮询
        //     }
        // }
        uart_rx_enable(custom_uart_dev, &tx_flag, 1, 0);  // DMA
        if (tx_flag == 0xCC) {
            tx_flag = 0;
            uart_tx(custom_uart_dev, test_buf, sizeof(test_buf), 0);  // DMA
        }
        k_msleep(10);
    }
}

K_THREAD_DEFINE(custom_uart_thread_id, 512, custom_uart_thread, NULL, NULL, NULL, 5, 0, 0);

#endif
