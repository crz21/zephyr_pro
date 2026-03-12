#ifdef(CONFIG_RTIO_UART)
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/rtio/rtio.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define BUF_COUNT (8)     // 总缓冲区数量
#define BATCH_SIZE (4)    // 每批处理的数量
#define UART_PKT_SZ (32)  // 每个串口包的大小

// 定义包含内存池的 RTIO 实例
RTIO_DEFINE_WITH_MEMPOOL(uart_io, BUF_COUNT, BUF_COUNT, BUF_COUNT, UART_PKT_SZ, 4);

int main(void)
{
    const struct device* const uart_dev = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(vnd_uart));
    struct rtio_iodev* iodev = uart_dev->data;

    /* 预先填充提交队列：告知驱动我们有 8 个空闲缓冲区可以接收数据 */
    for (int n = 0; n < BUF_COUNT; n++) {
        struct rtio_sqe* sqe = rtio_sqe_acquire(&uart_io);
        rtio_sqe_prep_read_with_pool(sqe, iodev, RTIO_PRIO_HIGH, NULL);
    }

    while (true) {
        uint8_t* bufs[BATCH_SIZE];
        uint32_t lens[BATCH_SIZE];

        LOG_INF("Waiting for %d UART packets...", BATCH_SIZE);

        /* 1. 提交请求（如果驱动层支持批处理）*/
        rtio_submit(&uart_io, 0);

        /* 2. 阻塞直到收集齐 BATCH_SIZE 个数据包 */
        for (int i = 0; i < BATCH_SIZE; i++) {
            struct rtio_cqe* cqe = rtio_cqe_consume(&uart_io);

            // 获取数据并保存指针
            rtio_cqe_get_mempool_buffer(&uart_io, cqe, &bufs[i], &lens[i]);
            rtio_cqe_release(&uart_io, cqe);
        }

        /* 3. 处理数据（模拟耗时的业务逻辑，如协议解析） */
        LOG_INF("Processing batch...");
        for (int i = 0; i < BATCH_SIZE; i++) {
            LOG_HEXDUMP_DBG(bufs[i], lens[i], "UART Data:");
        }
        k_msleep(500);  // 模拟处理延时

        /* 4. 回收缓冲区，重新提交 read 请求，保持 pipeline 流动 */
        for (int i = 0; i < BATCH_SIZE; i++) {
            rtio_release_buffer(&uart_io, bufs[i], lens[i]);
            struct rtio_sqe* sqe = rtio_sqe_acquire(&uart_io);
            rtio_sqe_prep_read_with_pool(sqe, iodev, RTIO_PRIO_HIGH, NULL);
        }
    }
    return 0;
}
#endif
