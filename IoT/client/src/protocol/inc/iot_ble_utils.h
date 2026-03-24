#ifndef __IOT_BLE_UTILS_H__
#define __IOT_BLE_UTILS_H__

#include <stdint.h>
#include <bluetooth/services/nus.h>

void crs_notify(uint8_t* data, uint16_t len);
int ble_utils_init(struct bt_nus_cb* nus_clbs, ble_connection_cb_t on_connect, ble_disconnection_cb_t on_disconnect);
#endif
