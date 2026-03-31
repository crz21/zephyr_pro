/**
 * @file
 * @defgroup ble_utils Bluetooth LE utilities API
 * @{
 */

/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __BLE_UTILS_H__
#define __BLE_UTILS_H__

#include <bluetooth/services/nus.h>

typedef void (*ble_connection_cb_t)(struct k_work *item);

typedef void (*ble_disconnection_cb_t)(struct k_work *item);

int ble_utils_init(struct bt_nus_cb *nus_clbs,
		   ble_connection_cb_t on_connect,
		   ble_disconnection_cb_t on_disconnect);

#endif

/**
 * @}
 */
