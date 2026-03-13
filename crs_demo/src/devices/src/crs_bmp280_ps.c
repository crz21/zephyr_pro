#ifdef CONFIG_BMP280_PS
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "main.h"

#define BMP280_REG_TEMP_XLSB (0xFC)
#define BMP280_REG_TEMP_LSB (0xFB)
#define BMP280_REG_TEMP_MSB (0xFA)
#define BMP280_REG_PRESS_XLSB (0xF9)
#define BMP280_REG_PRESS_LSB (0xF8)
#define BMP280_REG_PRESS_MSB (0xF7)
#define BMP280_REG_CONFIG (0xF5)
#define BMP280_REG_CTRL_MEAS (0xF4)
#define BMP280_REG_STATUS (0xF3)
#define BMP280_REG_RESET (0xE0)
#define BMP280_REG_ID (0xD0)

#define BMP280_SENSOR_NODE DT_NODELABEL(bmp280_sensor)
const struct i2c_dt_spec bmp280_spec = I2C_DT_SPEC_GET(BMP280_SENSOR_NODE);

enum BMP280_STATUS {
    READ_ID_STEP,
    READ_DATA_STEP,
};

struct bmp280_op {
    uint16_t cnt;
    uint8_t status;
    uint8_t data_buf[10];
};

struct bmp280_op* p_bmp280_op = NULL;

static void bmp280_handle(void)
{
    switch (p_bmp280_op->status) {
    case READ_ID_STEP:
        if (p_bmp280_op->cnt > 1000) {
            p_bmp280_op->cnt = 0;
            p_bmp280_op->data_buf[0] = BMP280_REG_ID;
            i2c_write_read_dt(&bmp280_spec, &p_bmp280_op->data_buf[0], 1, &p_bmp280_op->data_buf[1], 1);
            if ((p_bmp280_op->data_buf[1] & 0xff) == 0x58) {
                p_bmp280_op->status = READ_DATA_STEP;
            }
        }
        break;

    case READ_DATA_STEP:
        if (p_bmp280_op->cnt > 100) {
            create_event_payload(p_bmp280_op->data_buf, 7, BMP280_PS_EVENT);
        }
        break;

    default:
        break;
    }
}

void bmp280_ps_thread(void)
{
    if (!i2c_is_ready_dt(&bmp280_spec)) {
        printk("bmp280 not ready\n");
        return;
    }

    p_bmp280_op->cnt = 1000;

    for (;;) {
        k_mutex_lock(&i2c_mutex, K_FOREVER);
        p_bmp280_op->cnt += 10;
        bmp280_handle();
        k_mutex_unlock(&i2c_mutex);
        k_msleep(10);
    }
}

K_THREAD_DEFINE(bmp280_ps_thread_id, 1024, bmp280_ps_thread, NULL, NULL, NULL, BMP280_PS_PRIORITY, 0, 0);
#endif
