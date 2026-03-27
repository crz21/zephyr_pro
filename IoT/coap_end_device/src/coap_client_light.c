/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <dk_buttons_and_leds.h>
#include <openthread/thread.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>
#include <zephyr/pm/device.h>

#include "ble_utils.h"
#include "coap_client_utils.h"

LOG_MODULE_REGISTER(coap_server, CONFIG_COAP_SERVER_LOG_LEVEL);

#define OT_CONNECTION_LED DK_LED1
#define BLE_CONNECTION_LED DK_LED2
#define MTD_SED_LED DK_LED3
#define LIGHT_LED DK_LED4

enum light_command {
    THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE = '0',
};

#if CONFIG_BT_NUS

static void on_ble_connect(struct k_work* item)
{
    ARG_UNUSED(item);

    dk_set_led_on(BLE_CONNECTION_LED);
}

static void on_ble_disconnect(struct k_work* item)
{
    ARG_UNUSED(item);

    dk_set_led_off(BLE_CONNECTION_LED);
}

#endif /* CONFIG_BT_NUS */

static void on_ot_connect(struct k_work* item)
{
    ARG_UNUSED(item);

    dk_set_led_on(OT_CONNECTION_LED);
}

static void on_ot_disconnect(struct k_work* item)
{
    ARG_UNUSED(item);

    dk_set_led_off(OT_CONNECTION_LED);
}

static void on_mtd_mode_toggle(uint32_t med)
{
#if IS_ENABLED(CONFIG_PM_DEVICE)
    const struct device* cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (!device_is_ready(cons)) {
        return;
    }

    if (med) {
        pm_device_action_run(cons, PM_DEVICE_ACTION_RESUME);
    } else {
        pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
    }
#endif
    dk_set_led(MTD_SED_LED, med);
}

static void on_button_changed(uint32_t button_state, uint32_t has_changed)
{
    uint32_t buttons = button_state & has_changed;

    if (buttons & DK_BTN3_MSK) {
        coap_client_toggle_minimal_sleepy_end_device();
    }
}

static void on_light_request(uint8_t command)
{
    static uint8_t led_0_staus;

    switch (command) {
    case THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE:
        if (led_0_staus == 0) {
            led_0_staus = 1;
        } else {
            led_0_staus = 0;
        }
        dk_set_led(LIGHT_LED, led_0_staus);
        break;

    default:
        break;
    }
}

int main(void)
{
    int ret;

    if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
        power_down_unused_ram();
    }

    ret = dk_buttons_init(on_button_changed);
    if (ret) {
        LOG_ERR("Cannot init buttons (error: %d)", ret);
        return 0;
    }

    ret = dk_leds_init();
    if (ret) {
        LOG_ERR("Could not initialize leds, err code: %d", ret);
        return 0;
    }

#if CONFIG_BT_NUS
    struct bt_nus_cb nus_clbs = {
        .received = NULL,
        .sent = NULL,
    };

    ret = ble_utils_init(&nus_clbs, on_ble_connect, on_ble_disconnect);
    if (ret) {
        LOG_ERR("Cannot init BLE utilities");
        return 0;
    }

#endif /* CONFIG_BT_NUS */
    coap_client_utils_init(on_light_request, on_ot_connect, on_ot_disconnect, on_mtd_mode_toggle);

    return 0;
}
