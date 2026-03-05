#if defined(CONFIG_AS201_IMU)
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "main.h"

#define AS201_IMU_UART_NODE DT_PATH(zephyr_user)
const struct device* as201_imu_uart_dev = DEVICE_DT_GET(DT_PROP(AS201_IMU_UART_NODE, as201_imu_uart));

uint8_t stop_auto_update_cmd[] = {0xFA, 0xFB, 0x03, 0x1A, 0x00, 0x1A, 0xFC, 0xFD};
uint8_t ask_for_data_cmd[] = {0xFA, 0xFB, 0x03, 0x1B, 0x01, 0x1C, 0xFC, 0xFD};
uint8_t tx_buf_flag = 1;
static char rx_buf[100];
static int rx_buf_pos;

static void serial_cb(const struct device* dev, void* user_data)
{
    uint8_t c;

    if (!uart_irq_update(dev)) {
        printk("rx error 1\n");
        return;
    }

    if (!uart_irq_rx_ready(dev)) {
        printk("rx error 2\n");
        return;
    }

    /* read until FIFO empty */
    while (uart_fifo_read(dev, &c, 1) == 1) {
        if ((c == 0xFD) && rx_buf_pos > 0) {
            /* terminate string */
            rx_buf[rx_buf_pos] = 0xFD;

            /* if queue is full, message is silently dropped */
            // k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);
            create_event_payload(rx_buf, rx_buf_pos + 1, AS201_IMU_EVENT);

            tx_buf_flag = 1;
            /* reset the buffer (it was copied to the msgq) */
            rx_buf_pos = 0;
        } else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
            rx_buf[rx_buf_pos++] = c;
        }
        /* else: characters beyond buffer size are dropped */
    }
}

static void send_uart(uint8_t* buf, uint16_t len)
{
    for (int i = 0; i < len; i++) {
        uart_poll_out(as201_imu_uart_dev, buf[i]);
    }
}

void as201_imu_thread(void)
{
    if (!device_is_ready(as201_imu_uart_dev)) {
        printk("ac201 imu uart not ready\n");
        return;
    }

    int ret = uart_irq_callback_user_data_set(as201_imu_uart_dev, serial_cb, NULL);

    if (ret < 0) {
        if (ret == -ENOTSUP) {
            printk("Interrupt-driven UART API support not enabled\n");
        } else if (ret == -ENOSYS) {
            printk("UART device does not support interrupt-driven API\n");
        } else {
            printk("Error setting UART callback: %d\n", ret);
        }
        return;
    }
    uart_irq_rx_enable(as201_imu_uart_dev);
    send_uart(stop_auto_update_cmd, sizeof(stop_auto_update_cmd));
    for (;;) {
        if (tx_buf_flag) {
            send_uart(ask_for_data_cmd, sizeof(ask_for_data_cmd));
            tx_buf_flag = 0;
        }
    }
}

#endif