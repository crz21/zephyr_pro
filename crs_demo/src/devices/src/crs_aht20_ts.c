#ifdef CONFIG_AHT20_TS
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "main.h"

#define AHT20_SENSOR_NODE DT_NODELABEL(aht20_sensor)
const struct i2c_dt_spec aht20_spec = I2C_DT_SPEC_GET(AHT20_SENSOR_NODE);
enum { AHT20_IDLE, AHT20_MEASURE, AHT20_COMPLETE };

struct s_aht20_op {
    uint16_t cnt;
    uint8_t status;
    uint8_t data_buf[10];
    float temp;  // 温度
    float humi;  // 湿度
};

struct s_aht20_op* p_aht20_op;

static unsigned char check_crc8(unsigned char* pdat, unsigned char lenth)
{
    unsigned char crc = 0xff, i, j;

    for (i = 0; i < lenth; i++) {
        crc = crc ^ *pdat;
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
        pdat++;
    }
    return crc;
}

static void ath20_handle(void)
{
    uint32_t s32x = 0;

    switch (p_aht20_op->status) {
    case AHT20_IDLE:
        if (p_aht20_op->cnt > 10) {
            p_aht20_op->status = AHT20_MEASURE;
            p_aht20_op->data_buf[0] = 0xAC;
            p_aht20_op->data_buf[1] = 0x33;
            p_aht20_op->data_buf[2] = 0;
            i2c_write_dt(&aht20_spec, p_aht20_op->data_buf, 3);
        }
        break;

    case AHT20_MEASURE:
        if (p_aht20_op->cnt > 150) {
            p_aht20_op->status = AHT20_COMPLETE;
            i2c_read_dt(&aht20_spec, p_aht20_op->data_buf, 7);
            if ((check_crc8(p_aht20_op->data_buf, 6) == p_aht20_op->data_buf[6]) &&
                ((p_aht20_op->data_buf[0] & 0x98) == 0x18)) {
                s32x = p_aht20_op->data_buf[1];
                s32x = s32x << 8;
                s32x += p_aht20_op->data_buf[2];
                s32x = s32x << 8;
                s32x += p_aht20_op->data_buf[3];
                s32x = s32x >> 4;
                p_aht20_op->humi = (float)s32x;
                p_aht20_op->humi = p_aht20_op->humi * 100 / 1048576;

                s32x = p_aht20_op->data_buf[3] & 0x0F;
                s32x = s32x << 8;
                s32x += p_aht20_op->data_buf[4];
                s32x = s32x << 8;
                s32x += p_aht20_op->data_buf[5];
                p_aht20_op->temp = (float)s32x;
                p_aht20_op->temp = p_aht20_op->temp * 200 / 1048576 - 50;

                create_event_payload(p_aht20_op->data_buf, 6, AHT20_TS_EVENT);
            }
        }
        break;

    case AHT20_COMPLETE:
        if (p_aht20_op->cnt > 2000) {
            p_aht20_op->status = AHT20_IDLE;
            p_aht20_op->cnt = 0;
        }
        break;

    default:
        break;
    }
}

static void aht20_ts_thread(void)
{
    if (!i2c_is_ready_dt(&aht20_spec)) {
        printk("aht20 not ready\n");
        return;
    }

    p_aht20_op->status = AHT20_IDLE;

    for (;;) {
        k_mutex_lock(&i2c_mutex, K_FOREVER);
        p_aht20_op->cnt += 10;
        ath20_handle();
        k_mutex_unlock(&i2c_mutex);
        k_msleep(10);
    }
}

K_THREAD_DEFINE(aht20_ts_thread_id, 1024, aht20_ts_thread, NULL, NULL, NULL, AHT20_TS_PRIORITY, 0, 0);
#endif
