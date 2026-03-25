/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/kernel.h>
#include <net/coap_utils.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <openthread.h>
#include <openthread/thread.h>

#include "coap_client_utils.h"
LOG_MODULE_REGISTER(coap_client_utils, CONFIG_COAP_CLIENT_UTILS_LOG_LEVEL);

#define RESPONSE_POLL_PERIOD 100

static bool is_connected;

#define COAP_PORT 5683

enum light_command {
    THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE = '0',
};

#define LIGHT_URI_PATH "light"

#define COAP_CLIENT_WORKQ_STACK_SIZE 2048
#define COAP_CLIENT_WORKQ_PRIORITY 5
K_THREAD_STACK_DEFINE(coap_client_workq_stack_area, COAP_CLIENT_WORKQ_STACK_SIZE);
static struct k_work_q coap_client_workq;
static struct k_work multicast_light_work;
static struct k_work toggle_MTD_SED_work;
static struct k_work on_connect_work;
static struct k_work on_disconnect_work;
mtd_mode_toggle_cb_t on_mtd_mode_toggle;

/* Options supported by the server */
static const char *const light_option[] = { LIGHT_URI_PATH, NULL };

/* Thread multicast mesh local address */
static struct sockaddr_in6 multicast_local_addr = {
    .sin6_family = AF_INET6,
    .sin6_port = htons(COAP_PORT),
    .sin6_addr.s6_addr = {0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x01},
    .sin6_scope_id = 0U};

/* Variable for storing server address acquiring in provisioning handshake */
static char unique_local_addr_str[INET6_ADDRSTRLEN];
static struct sockaddr_in6 unique_local_addr = {.sin6_family = AF_INET6,
                                                .sin6_port = htons(COAP_PORT),
                                                .sin6_addr.s6_addr =
                                                    {
                                                        0,
                                                    },
                                                .sin6_scope_id = 0U};

static void toggle_mesh_light_0(struct k_work* item)
{
    static uint8_t command = (uint8_t)THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE;

    ARG_UNUSED(item);

    LOG_INF("Send multicast mesh 'light' request");
    coap_send_request(COAP_METHOD_PUT, (const struct sockaddr*)&multicast_local_addr, light_option, &command,
                      sizeof(command), NULL);
}

static void toggle_minimal_sleepy_end_device(struct k_work* item)
{
    otError error;
    otLinkModeConfig mode;
    struct otInstance* instance = openthread_get_default_instance();

    openthread_mutex_lock();
    mode = otThreadGetLinkMode(instance);
    mode.mRxOnWhenIdle = !mode.mRxOnWhenIdle;
    error = otThreadSetLinkMode(instance, mode);
    openthread_mutex_unlock();

    if (error != OT_ERROR_NONE) {
        LOG_ERR("Failed to set MLE link mode configuration");
    } else {
        on_mtd_mode_toggle(mode.mRxOnWhenIdle);
    }
}

static void update_device_state(void)
{
    struct otInstance* instance = openthread_get_default_instance();
    otLinkModeConfig mode = otThreadGetLinkMode(instance);
    on_mtd_mode_toggle(mode.mRxOnWhenIdle);
}

static void on_thread_state_changed(otChangedFlags flags, void* user_data)
{
    struct otInstance* instance = openthread_get_default_instance();

    if (flags & OT_CHANGED_THREAD_ROLE) {
        switch (otThreadGetDeviceRole(instance)) {
        case OT_DEVICE_ROLE_CHILD:
        case OT_DEVICE_ROLE_ROUTER:
        case OT_DEVICE_ROLE_LEADER:
            k_work_submit_to_queue(&coap_client_workq, &on_connect_work);
            is_connected = true;
            break;

        case OT_DEVICE_ROLE_DISABLED:
        case OT_DEVICE_ROLE_DETACHED:
        default:
            k_work_submit_to_queue(&coap_client_workq, &on_disconnect_work);
            is_connected = false;
            break;
        }
    }
}
static struct openthread_state_changed_callback ot_state_chaged_cb = {.otCallback = on_thread_state_changed};

void coap_client_utils_init(ot_connection_cb_t on_connect, ot_disconnection_cb_t on_disconnect,
                            mtd_mode_toggle_cb_t on_toggle)
{
    on_mtd_mode_toggle = on_toggle;

    coap_init(AF_INET6, NULL);

    k_work_queue_init(&coap_client_workq);
    k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
                       K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);

    k_work_init(&on_connect_work, on_connect);
    k_work_init(&on_disconnect_work, on_disconnect);
    k_work_init(&multicast_light_work, toggle_mesh_light_0);

    openthread_state_changed_callback_register(&ot_state_chaged_cb);
    openthread_run();

    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_init(&toggle_MTD_SED_work, toggle_minimal_sleepy_end_device);
        update_device_state();
    }
}

static void submit_work_if_connected(struct k_work* work)
{
    if (is_connected) {
        k_work_submit_to_queue(&coap_client_workq, work);
    } else {
        LOG_INF("Connection is broken");
    }
}

void coap_client_toggle_mesh_lights(void) { submit_work_if_connected(&multicast_light_work); }

void coap_client_toggle_minimal_sleepy_end_device(void)
{
    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_submit_to_queue(&coap_client_workq, &toggle_MTD_SED_work);
    }
}
