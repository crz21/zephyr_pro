#ifndef __CRS_BLE_H__
#define __CRS_BLE_H__

#include <stdint.h>

void crs_notify(uint8_t* data, uint16_t len);
void ble_thread(void);

#endif
