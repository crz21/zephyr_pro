#ifndef __MAIN_H__
#define __MAIN_H__

#include <zephyr/kernel.h>

#include "devices/inc/crs_ble.h"

#if defined(CONFIG_AHT20_TS)
#include "devices/inc/crs_aht20_ts.h"
#endif

#if defined(CONFIG_BLINK)
#include "devices/inc/crs_blink.h"
#endif

#if defined(CONFIG_BMP280_PS)
#include "devices/inc/crs_bmp280_ps.h"
#endif

#if defined(CONFIG_HRBO)
#include "devices/inc/crs_hrbo.h"
#endif

#if defined(CONFIG_DEBUG_UART)
#include "devices/inc/crs_debug_uart.h"
#endif

#if defined(CONFIG_LD2410C_DDRS)
#include "devices/inc/crs_ld2410c_ddrs.h"
#endif

#if defined(CONFIG_AS201_IMU)
#include "devices/inc/crs_as201_imu.h"
#endif

enum {
#if defined(CONFIG_AHT20_TS)
    AHT20_TS_EVENT_NUM,
#endif

#if defined(CONFIG_BMP280_PS)
    BMP280_PS_EVENT_NUM,
#endif

#if defined(CONFIG_HRBO)
    HRBO_EVENT_NUM,
#endif

#if defined(CONFIG_LD2410C_DDRS)
    LD2410C_DDRS_EVENT_NUM,
#endif

#if defined(CONFIG_AS201_IMU)
    AS201_IMU_EVENT_NUM,
#endif
    MAX_EVENT_NUM
};

#if defined(CONFIG_AHT20_TS)
#define AHT20_TS_EVENT (1 << AHT20_TS_EVENT_NUM)
#endif

#if defined(CONFIG_BMP280_PS)
#define BMP280_PS_EVENT (1 << BMP280_PS_EVENT_NUM)
#endif

#if defined(CONFIG_HRBO)
#define HRBO_EVENT (1 << HRBO_EVENT_NUM)
#endif

#if defined(CONFIG_LD2410C_DDRS)
#define LD2410C_DDRS_EVENT (1 << LD2410C_DDRS_EVENT_NUM)
#endif

#if defined(CONFIG_AS201_IMU)
#define AS201_IMU_EVENT (1 << AS201_IMU_EVENT_NUM)
#endif

#define HRBO_PRIORITY (3)
#define AHT20_TS_PRIORITY (5)
#define BMP280_PS_PRIORITY (5)
#define BLINK0_PRIORITY (5)
#define LD2410C_DDRS_PRIORITY (6)
#define DEBUG_UART_PRIORITY (8)
#define PRODUCER_PRIORITY (4)
#define BLE_PRIORITY (5)
#define AS201_IMU_PRIORITY (6)

void create_event_payload(uint8_t* data, uint16_t len, uint32_t event);
extern struct k_mutex i2c_mutex;
#endif
