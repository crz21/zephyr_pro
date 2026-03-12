#ifdef(CONFIG_CUSTOM_SPI)
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>

#include "main.h"

#define CUSTOM_SPI_NODE DT_ALIAS(custom_spi)
const struct device* custom_spi_dev = DEVICE_DT_GET(CUSTOM_SPI_NODE);

void custom_spi_thread(void)
{
    if (!device_is_ready(custom_spi_dev)) {
        return;
    }

    for (;;) {
    }
}

K_THREAD_DEFINE(custom_spi_thread_id, 512, custom_spi_thread, NULL, NULL, NULL, 5, 0, 0);

#endif
