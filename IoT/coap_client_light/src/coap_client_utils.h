/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __OT_COAP_UTILS_H__
#define __OT_COAP_UTILS_H__


/**@brief Type definition of the function used to handle light resource change.
 */
typedef void (*light_request_callback_t)(uint8_t cmd);

/** @brief Type indicates function called when OpenThread connection
 *         is established.
 *
 * @param[in] item pointer to work item.
 */
typedef void (*ot_connection_cb_t)(struct k_work* item);

/** @brief Type indicates function called when OpenThread connection is ended.
 *
 * @param[in] item pointer to work item.
 */
typedef void (*ot_disconnection_cb_t)(struct k_work* item);

/** @brief Type indicates function called when the MTD modes are toggled.
 *
 * @param[in] val 1 if the MTD is in MED mode
 *                0 if the MTD is in SED mode
 */
typedef void (*mtd_mode_toggle_cb_t)(uint32_t val);

int coap_client_utils_init(light_request_callback_t on_light_request, ot_connection_cb_t on_connect,
                           ot_disconnection_cb_t on_disconnect, mtd_mode_toggle_cb_t on_toggle);

void coap_client_toggle_minimal_sleepy_end_device(void);

#endif
