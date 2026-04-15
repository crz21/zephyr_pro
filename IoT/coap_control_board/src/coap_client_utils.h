/**
 * @file
 * @defgroup coap_client_utils API for coap_client_* samples
 * @{
 */

/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef __COAP_CLIENT_UTILS_H__
#define __COAP_CLIENT_UTILS_H__

typedef void (*mtd_mode_toggle_cb_t)(uint32_t val);

int coap_client_utils_init(void);

void toggle_mesh_light_0(struct k_work* item);
void toggle_minimal_sleepy_end_device(struct k_work* item);
void update_device_state(void);
void coap_client_send_provisioning_request(void);
#endif

/**
 * @}
 */
