#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "storage_usr.h"

#define CUSTOM_UART_NODE DT_ALIAS(custom_uart)
const struct device* custom_uart_dev = DEVICE_DT_GET(CUSTOM_UART_NODE);

int main(void)
{
    uint8_t tx_buf[100] = {0};
    uint8_t rx_buf[100] = {0};

#if (CONFIG_FLASH)
    init_storage();
#endif

    if (!device_is_ready(custom_uart_dev)) {
        return;
    }

#if (CONFIG_FLASH)
    load_parameter(tx_buf, 8);
#endif
    uart_tx(custom_uart_dev, tx_buf, 8, 0);

    for (;;) {
        uart_rx_enable(custom_uart_dev, rx_buf, 8, 0);
        if (0xaa == rx_buf[0] && 0x55 == rx_buf[1]) {
#if (CONFIG_FLASH)
            save_parameter(rx_buf, 8);
#endif
        }
    }

    return 0;
}
