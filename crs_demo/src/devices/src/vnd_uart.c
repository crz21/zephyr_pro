#define DT_DRV_COMPAT vnd_uart

#include <zephyr/kernel.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(vnd_uart, LOG_LEVEL_DBG);

struct vnd_uart_data {
    struct rtio_iodev iodev;
    struct mpsc io_q;
    const struct device *dev;
    // 模拟硬件：在真实驱动中，这里会处理 UART 中断状态
};

// 模拟串口接收到数据的处理函数（对应原代码中的 vnd_sensor_handle_int）
static void vnd_uart_rx_complete(const struct device *dev, uint8_t *hw_rx_data, size_t len)
{
    struct vnd_uart_data *data = dev->data;
    struct mpsc_node *node = mpsc_pop(&data->io_q);

    if (node != NULL) {
        struct rtio_iodev_sqe *iodev_sqe = CONTAINER_OF(node, struct rtio_iodev_sqe, q);
        uint8_t *dest_buf;
        uint32_t dest_len;

        // 从 RTIO 内存池获取缓冲区
        int res = rtio_sqe_rx_buf(iodev_sqe, len, len, &dest_buf, &dest_len);
        if (res == 0) {
            memcpy(dest_buf, hw_rx_data, len);
            rtio_iodev_sqe_ok(iodev_sqe, 0);
        } else {
            rtio_iodev_sqe_err(iodev_sqe, res);
        }
    }
}

static void vnd_uart_iodev_submit(struct rtio_iodev_sqe *iodev_sqe)
{
    struct vnd_uart_data *data = (struct vnd_uart_data *) iodev_sqe->sqe.iodev;
    // 将读取请求放入队列，等待硬件中断触发
    mpsc_push(&data->io_q, &iodev_sqe->q);
    
    // 在真实硬件驱动中，此处可能需要检查是否需要开启 UART 中断
}

static const struct rtio_iodev_api vnd_uart_iodev_api = {
    .submit = vnd_uart_iodev_submit,
};

static int vnd_uart_init(const struct device *dev)
{
    struct vnd_uart_data *data = dev->data;
    data->dev = dev;
    mpsc_init(&data->io_q);
    return 0;
}

#define VND_UART_INIT(n)                                        \
    static struct vnd_uart_data vnd_uart_data_##n = {           \
        .iodev = { .api = &vnd_uart_iodev_api },                \
    };                                                          \
    DEVICE_DT_INST_DEFINE(n, vnd_uart_init, NULL,               \
                          &vnd_uart_data_##n, NULL,             \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

DT_INST_FOREACH_STATUS_OKAY(VND_UART_INIT)
