#if defined(CONFIG_DEBUG_UART)
// #include <zephyr/drivers/uart.h>
// #include <zephyr/kernel.h>

// #include "main.h"

// #define USER_NODE DT_PATH(zephyr_user)
// const struct device* uart_dev = DEVICE_DT_GET(DT_PROP(USER_NODE, debug_uart));

// void debug_uart_thread(void)
// {
//     if (!device_is_ready(uart_dev)) {
//         printk("uart not ready\n");
//         return;
//     }

//     for (;;) {
//         k_msleep(10);
//     }
// }
// K_THREAD_DEFINE(debug_uart_thread_id, 1024, debug_uart_thread, NULL, NULL, NULL, DEBUG_UART_PRIORITY, 0, 0);


/*
 * 基于 RTIO 的 UART 异步读取示例
 */

 #include <zephyr/kernel.h>
 #include <zephyr/rtio/rtio.h>
//  #include <zephyr/logging/log.h>
 #include <zephyr/drivers/uart.h>
 
//  LOG_MODULE_REGISTER(uart_rtio_main, LOG_LEVEL_DBG);
 
 #define N           (8)           // 队列深度
 #define M           (N/2)         // 每次处理的批次数
 #define SQ_SZ       (N)
 #define CQ_SZ       (N)
 
 /* 获取串口节点：这里假设使用 alias 里的 "uart-dev" 或者指定 compatible */
 #define UART_NODE   DT_ALIAS(uart_dev) 
 #if !DT_NODE_EXISTS(UART_NODE)
 #define UART_NODE   DT_CHOSEN(zephyr_console) // 退而求其次使用控制台串口
 #endif
 
 /* 串口不像传感器有 sample_period，我们手动定义 */
 #define READ_SIZE       32    // 每次读取的字节数
 #define PROCESS_TIME    K_MSEC(100)
 
 // 定义 RTIO 及其内存池（用于存放串口读取的数据）
 RTIO_DEFINE_WITH_MEMPOOL(ez_io, SQ_SZ, CQ_SZ, N, READ_SIZE, 4);
 
 int main(void)
 {
     const struct device *const uart_dev = DEVICE_DT_GET(UART_NODE);
     
     if (!device_is_ready(uart_dev)) {
         LOG_ERR("UART device not ready");
         return -1;
     }
 
     /* 在 Zephyr RTIO 中，驱动通常将 iodev 存储在 config 或 data 中 */
     /* 注意：并非所有串口驱动都支持 RTIO。这里假设使用支持 RTIO 的虚拟或真实驱动 */
     struct rtio_iodev *iodev = (struct rtio_iodev *)uart_dev->data;
 
    //  LOG_INF("UART RTIO demo starting on %s", uart_dev->name);
 
     /* 1. 预填充提交队列 (SQ) */
     for (int n = 0; n < N; n++) {
         struct rtio_sqe *sqe = rtio_sqe_acquire(&ez_io);
         // 准备从内存池自动分配缓冲区的读取请求
         rtio_sqe_prep_read_with_pool(sqe, iodev, RTIO_PRIO_HIGH, NULL);
     }
 
     while (true) {
         int m = 0;
         uint8_t *userdata[M] = {0};
         uint32_t data_len[M] = {0};
 
         /* 2. 提交 M 个读取任务到硬件 */
        //  LOG_INF("Submitting %d UART read requests", M);
         rtio_submit(&ez_io, M);
 
         /* 3. 消费完成队列 (CQE) */
         while (m < M) {
             struct rtio_cqe *cqe = rtio_cqe_consume(&ez_io);
 
             if (cqe == NULL) {
                 k_yield(); // 串口可能还没数据，让出 CPU
                 continue;
             }
 
             if (cqe->result < 0) {
                //  LOG_ERR("UART Read failed with error: %d", cqe->result);
             } else {
                 // 获取从内存池分配的缓冲区指针和实际读到的长度
                 rtio_cqe_get_mempool_buffer(&ez_io, cqe, &userdata[m], &data_len[m]);
                //  LOG_DBG("Received %d bytes from UART", data_len[m]);
             }
 
             rtio_cqe_release(&ez_io, cqe);
             m++;
         }
 
         /* 4. 批处理数据（例如解析串口协议包） */
        //  LOG_INF("--- Processing Batch ---");
         for (m = 0; m < M; m++) {
             if (userdata[m]) {
                //  LOG_HEXDUMP_DBG(userdata[m], data_len[m], "UART Data:");
             }
         }
         
         // 模拟复杂处理耗时
         k_sleep(PROCESS_TIME);
 
         /* 5. 回收缓冲区并重新填充 SQ，准备下一次读取 */
         for (m = 0; m < M; m++) {
             if (userdata[m]) {
                 struct rtio_sqe *sqe = rtio_sqe_acquire(&ez_io);
                 rtio_release_buffer(&ez_io, userdata[m], data_len[m]);
                 rtio_sqe_prep_read_with_pool(sqe, iodev, RTIO_PRIO_HIGH, NULL);
             }
         }
     }
 
     return 0;
 }
#endif
