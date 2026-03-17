#ifndef __IOT_BLE_H__
#define __IOT_BLE_H__
#ifdef CONFIG_BT_CRS
#include <stdint.h>

void crs_notify(uint8_t* data, uint16_t len);
#endif
#endif
