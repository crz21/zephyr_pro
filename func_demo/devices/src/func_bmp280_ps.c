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
#define BMP280_REG_ID (0xD0)

// #define BMP280_SENSOR_NODE DT_NODELABEL(bmp280_sensor)
// const struct i2c_dt_spec bmp280_spec = I2C_DT_SPEC_GET(BMP280_SENSOR_NODE);
#define BMP280_I2C_ADDR (0x77)
#define I2C_DEV_NODE DT_ALIAS(bmp280_i2c)
const struct device* bmp280_i2c_dev = DEVICE_DT_GET(I2C_DEV_NODE);

void func_bmp280_ps_thread(void)
{
    uint8_t bmp280_data_buf[2] = {0};

    if (!device_is_ready(bmp280_i2c_dev)) {
        return;
    }

    for (;;) {
        bmp280_data_buf[0] = BMP280_REG_ID;
        i2c_write_read(bmp280_i2c_dev, BMP280_I2C_ADDR, &bmp280_data_buf[0], 1, &bmp280_data_buf[1], 1);
        // tx_buf_xxx(&bmp280_data_buf[1]);
        if ((bmp280_data_buf[1] & 0xff) != 0x58) {
            k_msleep(1000);
        } else {
            break;
        }
    }

    for (;;) {
        // k_mutex_lock(&i2c_mutex, K_FOREVER);
        // bmp280_data_buf[0] = 0xD0;
        // i2c_write_read_dt(&bmp280_spec, &bmp280_data_buf[0], 1, &bmp280_data_buf[1], 1);
        // create_event_payload(bmp280_data_buf, sizeof(bmp280_data_buf), BMP280_PS_EVENT);
        // k_mutex_unlock(&i2c_mutex);
    }
}

K_THREAD_DEFINE(func_bmp280_ps_thread_id, 1024, func_bmp280_ps_thread, NULL, NULL, NULL, 8, 0, 0);

#endif
