#ifndef __MAIN_H__
#define __MAIN_H__

enum {
#ifdef CONFIG_AHT20_TS
    AHT20_TS_EVENT_NUM,
#endif

#ifdef CONFIG_BMP280_PS
    BMP280_PS_EVENT_NUM,
#endif

#ifdef CONFIG_LD2410C_DDRS
    LD2410C_DDRS_EVENT_NUM,
#endif
    MAX_EVENT_NUM
};
#include <zephyr/kernel.h>

#include "iot_producer.h"
#include "protocol/inc/iot_ble.h"
#define BLE_PRIORITY (5)

#include "protocol/inc/iot_coap_utils.h"
#define OPENTHREAD_PRIORITY (5)

#include "devices/inc/iot_button.h"
#include "devices/inc/iot_led.h"

#ifdef CONFIG_AHT20_TS
#include "devices/inc/iot_aht20_ts.h"
#define AHT20_TS_EVENT (1 << AHT20_TS_EVENT_NUM)
#define AHT20_TS_PRIORITY (5)
#endif

#ifdef CONFIG_BMP280_PS
#include "devices/inc/iot_bmp280_ps.h"
#define BMP280_PS_EVENT (1 << BMP280_PS_EVENT_NUM)
#define BMP280_PS_PRIORITY (5)
#endif

#ifdef CONFIG_LD2410C_DDRS
#include "devices/inc/iot_ld2410c_ddrs.h"
#define LD2410C_DDRS_EVENT (1 << LD2410C_DDRS_EVENT_NUM)
#define LD2410C_DDRS_PRIORITY (6)
#endif

#define PRODUCER_PRIORITY (4)

extern struct k_mutex i2c_mutex;

#endif
