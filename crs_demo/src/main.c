#include "main.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/logging/log.h>

// #include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
// #include <zephyr/mgmt/mcumgr/transport/smp_bt.h>
// #include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
// #include <zephyr/mgmt/mcumgr/grp/os_mgmt/os_mgmt.h>

#define MSG_QUEUE_SIZE (100)

struct k_mutex i2c_mutex;

typedef struct {
    uint16_t data_len;
    uint8_t data[];
} p_characteristic_data_t;

typedef struct {
    void* p_msg;
    uint32_t event;
    void* p_data;
} p_msg_t;

K_MSGQ_DEFINE(g_msg_queue, sizeof(p_msg_t*), MSG_QUEUE_SIZE, 4);
K_EVENT_DEFINE(g_event_group);
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static void project_zero_process_application_message(void)
{
    p_msg_t* rev_msg = NULL;
    static uint8_t tx_buf[109] = {0};
    uint16_t buf_len = 0;

    if (k_msgq_get(&g_msg_queue, &rev_msg, K_FOREVER) == 0) {
        if (rev_msg != NULL) {
            p_characteristic_data_t* rev_data = (p_characteristic_data_t*)rev_msg->p_data;

            switch (rev_msg->event) {
#if defined(CONFIG_HRBO)
            case HRBO_EVENT:
                tx_buf[0] = 0x55;
                tx_buf[1] = 0xaa;
                buf_len = sizeof(tx_buf) - 4;
                tx_buf[2] = buf_len >> 8;
                tx_buf[3] = (uint8_t)buf_len;

                for (uint16_t i = 0; i < rev_data->data_len; i++) {
                    tx_buf[8 + i] = rev_data->data[i];
                }

                tx_buf[108] = 0xa5;
                crs_notify(tx_buf, sizeof(tx_buf));
                break;
#endif

#if defined(CONFIG_AHT20_TS)
            case AHT20_TS_EVENT:
                tx_buf[4] = rev_data->data[0];
                tx_buf[5] = rev_data->data[1];
                break;
#endif

#if defined(CONFIG_BMP280_PS)
            case BMP280_PS_EVENT:
                tx_buf[6] = rev_data->data[0];
                tx_buf[7] = rev_data->data[1];
                break;
#endif

#if defined(CONFIG_AS201_IMU)
            case AS201_IMU_EVENT:
                for (uint16_t i = 0; i < rev_data->data_len; i++) {
                    tx_buf[8 + i] = rev_data->data[i];
                }
                break;
#endif

            default:
                break;
            }

            if (rev_msg->p_data != NULL) {
                k_free(rev_msg->p_data);
            }

            k_free(rev_msg);
        }
    }
}

static void producer_thread(void* rec, void* p2, void* p3)
{
    for (;;) {
        uint32_t event_bits = k_event_wait(&g_event_group,
                                           0
#if defined(CONFIG_AHT20_TS)
                                               | AHT20_TS_EVENT
#endif

#if defined(CONFIG_BMP280_PS)
                                               | BMP280_PS_EVENT
#endif

#if defined(CONFIG_HRBO)
                                               | HRBO_EVENT
#endif

#if defined(CONFIG_LD2410C_DDRS)
                                               | LD2410C_DDRS_EVENT
#endif

#if defined(CONFIG_AS201_IMU)
                                               | AS201_IMU_EVENT
#endif
                                           ,
                                           true, K_FOREVER);
        if (event_bits != 0) {
            while (k_msgq_num_used_get(&g_msg_queue) > 0) {
                project_zero_process_application_message();
            }
        }
    }
}

void create_event_payload(uint8_t* data, uint16_t len, uint32_t event)
{
    p_msg_t* g_msg = (p_msg_t*)k_malloc(sizeof(p_msg_t));
    p_characteristic_data_t* g_data = (p_characteristic_data_t*)k_malloc(sizeof(p_characteristic_data_t) + len);
    if (g_msg == NULL || g_data == NULL) {
        k_free(g_msg);
        k_free(g_data);
        LOG_ERR("Memory allocation failed!");
        return;
    }

    g_data->data_len = len;
    memcpy(g_data->data, data, len);

    g_msg->event = event;
    g_msg->p_data = g_data;
    g_msg->p_msg = g_msg;

    if (k_msgq_put(&g_msg_queue, &g_msg, K_NO_WAIT) != 0) {
        k_free(g_data);
        k_free(g_msg);
        LOG_WRN("Queue full, dropping packet");
    } else
        k_event_post(&g_event_group, event);
}

int main(void)
{
    printk("Address of sample %p\n", (void*)__rom_region_start);

    k_mutex_init(&i2c_mutex);

    // // 1. 注册 OS 管理组（用于重启等）
    // os_mgmt_register_group();

    // // 2. 注册镜像管理组（用于 DFU 固件传输）
    // img_mgmt_register_group();


    while (1) {
        k_msleep(1);
    }
    return 0;
}

K_THREAD_DEFINE(producer_thread_id, 4096, producer_thread, NULL, NULL, NULL, PRODUCER_PRIORITY, 0, 0);
