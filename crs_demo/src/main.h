#ifndef __MAIN_H__
#define __MAIN_H__

#include <zephyr/kernel.h>

#include "protocol/inc/crs_producer.h"

enum {
#ifdef CONFIG_AHT20_TS
    AHT20_TS_EVENT_NUM,
#endif

#ifdef CONFIG_BMP280_PS
    BMP280_PS_EVENT_NUM,
#endif

#ifdef CONFIG_HRBO
    HRBO_EVENT_NUM,
#endif

#ifdef CONFIG_LD2410C_DDRS
    LD2410C_DDRS_EVENT_NUM,
#endif

#ifdef CONFIG_AS201_IMU
    AS201_IMU_EVENT_NUM,
#endif
    MAX_EVENT_NUM
};

#ifdef CONFIG_BT_CRS
#include "communicate/inc/crs_ble.h"
#define BLE_PRIORITY (5)
#endif

#ifdef CONFIG_DEBUG_UART
#include "communicate/inc/crs_debug_uart.h"
#define DEBUG_UART_PRIORITY (8)
#endif

#ifdef CONFIG_BLINK
#include "devices/inc/crs_blink.h"
#define BLINK0_PRIORITY (5)
#endif

#ifdef CONFIG_AHT20_TS
#include "devices/inc/crs_aht20_ts.h"
#define AHT20_TS_EVENT (1 << AHT20_TS_EVENT_NUM)
#define AHT20_TS_PRIORITY (5)
#endif

#ifdef CONFIG_BMP280_PS
#include "devices/inc/crs_bmp280_ps.h"
#define BMP280_PS_EVENT (1 << BMP280_PS_EVENT_NUM)
#define BMP280_PS_PRIORITY (5)
#endif

#ifdef CONFIG_HRBO
#include "devices/inc/crs_hrbo.h"
#define HRBO_EVENT (1 << HRBO_EVENT_NUM)
#define HRBO_PRIORITY (3)
#endif

#ifdef CONFIG_LD2410C_DDRS
#include "devices/inc/crs_ld2410c_ddrs.h"
#define LD2410C_DDRS_EVENT (1 << LD2410C_DDRS_EVENT_NUM)
#define LD2410C_DDRS_PRIORITY (6)
#endif

#ifdef CONFIG_AS201_IMU
#include "devices/inc/crs_as201_imu.h"
#define AS201_IMU_EVENT (1 << AS201_IMU_EVENT_NUM)
#define AS201_IMU_PRIORITY (6)
#endif

#define PRODUCER_PRIORITY (4)

extern struct k_mutex i2c_mutex;

#endif
