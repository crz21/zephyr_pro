/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>
#include "ble.h"
#include <openthread/thread.h>
#include <zephyr/net/openthread.h>

LOG_MODULE_REGISTER(coap_server, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

#define LEADER_LED DK_LED1
#define ROUTER_LED DK_LED3
#define CHILD_LED DK_LED3

static void on_thread_state_changed(otChangedFlags flags, void* user_data)
{
    if (flags & OT_CHANGED_THREAD_ROLE) {
        switch (otThreadGetDeviceRole(openthread_get_default_instance())) {
        case OT_DEVICE_ROLE_CHILD:
            dk_set_led_on(CHILD_LED);
            break;

        case OT_DEVICE_ROLE_ROUTER:
            dk_set_led_on(ROUTER_LED);
            break;

        case OT_DEVICE_ROLE_LEADER:
            dk_set_led_on(LEADER_LED);
            break;

        case OT_DEVICE_ROLE_DISABLED:
        case OT_DEVICE_ROLE_DETACHED:
        default:
            dk_set_led_off(LEADER_LED);
            dk_set_led_off(ROUTER_LED);
            dk_set_led_off(CHILD_LED);
            break;
        }
    }
}

static struct openthread_state_changed_callback ot_state_chaged_cb = {.otCallback = on_thread_state_changed};

int main(void)
{
    int ret;

#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)
    const struct device* dev;
    uint32_t dtr = 0U;

    dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
    if (dev == NULL) {
        LOG_ERR("Failed to find specific UART device");
        return 0;
    }

    LOG_INF("Waiting for host to be ready to communicate");

    /* Data Terminal Ready - check if host is ready to communicate */
    while (!dtr) {
        ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        if (ret) {
            LOG_ERR("Failed to get Data Terminal Ready line state: %d", ret);
            continue;
        }
        k_msleep(100);
    }

    /* Data Carrier Detect Modem - mark connection as established */
    (void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
    /* Data Set Ready - the NCP SoC is ready to communicate */
    (void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
#endif

    ret = dk_leds_init();
    if (ret) {
        LOG_ERR("Could not initialize leds, err code: %d", ret);
        return 0;
    }

    ble_enable();

    openthread_state_changed_callback_register(&ot_state_chaged_cb);
    openthread_run();

    return 0;
}
