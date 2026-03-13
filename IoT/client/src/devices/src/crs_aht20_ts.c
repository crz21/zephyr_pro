#ifdef CONFIG_AHT20_TS
#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "main.h"

#if 0

#define AHT20_SENSOR_NODE DT_NODELABEL(aht20_sensor)
const struct i2c_dt_spec aht20_spec = I2C_DT_SPEC_GET(AHT20_SENSOR_NODE);

#else

#define NODE_ID DT_COMPAT_GET_ANY_STATUS_OKAY(vnd_sensor)
#define SAMPLE_PERIOD DT_PROP(NODE_ID, sample_period)
#define SAMPLE_SIZE DT_PROP(NODE_ID, sample_size)
#define PROCESS_TIME ((M - 1) * SAMPLE_PERIOD)
RTIO_DEFINE_WITH_MEMPOOL(aht20_io, SQ_SZ, CQ_SZ, N, SAMPLE_SIZE, 4);

#endif

void aht20_ts_thread(void)
{
#if 0
    uint8_t aht20_data_buf[2] = {0};

    if (!i2c_is_ready_dt(&aht20_spec)) {
        printk("aht20 not ready\n");
        return;
    }

    for (;;) {
        k_mutex_lock(&i2c_mutex, K_FOREVER);
        aht20_data_buf[0] = 0x71;
        i2c_write_read_dt(&aht20_spec, &aht20_data_buf[0], 1, &aht20_data_buf[1], 1);
        create_event_payload(aht20_data_buf, sizeof(aht20_data_buf), AHT20_TS_EVENT);
        k_mutex_unlock(&i2c_mutex);
    }
#else
#endif
}

K_THREAD_DEFINE(aht20_ts_thread_id, 1024, aht20_ts_thread, NULL, NULL, NULL, AHT20_TS_PRIORITY, 0, 0);
#endif
