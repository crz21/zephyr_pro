/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <dk_buttons_and_leds.h>
#include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

#include "ble_utils.h"
#include "coap_client_utils.h"
#include "list.h"

LOG_MODULE_REGISTER(coap_client, CONFIG_COAP_CLIENT_LOG_LEVEL);

#define OT_CONNECTION_LED DK_LED1
#define BLE_CONNECTION_LED DK_LED2
#define MTD_SED_LED DK_LED3

static struct k_work_q coap_client_workq;
static struct k_work on_connect_work;
static struct k_work on_disconnect_work;
static struct k_work multicast_light_work;
static struct k_work toggle_mtd_sed_work;

#define COAP_CLIENT_WORKQ_STACK_SIZE 2048
#define COAP_CLIENT_WORKQ_PRIORITY 5
K_THREAD_STACK_DEFINE(coap_client_workq_stack_area, COAP_CLIENT_WORKQ_STACK_SIZE);

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

    if (buttons & DK_BTN2_MSK) {
        k_work_submit_to_queue(&coap_client_workq, &multicast_light_work);
    }

    if (buttons & DK_BTN3_MSK) {
        if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
            k_work_submit_to_queue(&coap_client_workq, &toggle_mtd_sed_work);
        }
    }
}

void ot_connect_statue(bool statue)
{
    if (true) {
        k_work_submit_to_queue(&coap_client_workq, &on_connect_work);
    } else {
        k_work_submit_to_queue(&coap_client_workq, &on_disconnect_work);
    }
}

int main(void)
{
    int ret;

    if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
        power_down_unused_ram();
    }

    k_work_queue_init(&coap_client_workq);
    k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
                       K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);
    k_work_init(&on_connect_work, on_ot_connect);
    k_work_init(&on_disconnect_work, on_ot_disconnect);
    k_work_init(&multicast_light_work, toggle_mesh_light_0);
    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_init(&toggle_mtd_sed_work, toggle_minimal_sleepy_end_device);
        update_device_state();
    }

    ret = dk_buttons_init(on_button_changed);
    if (ret) {
        LOG_ERR("Cannot init buttons (error: %d)", ret);
        return 0;
    }

    ret = dk_leds_init();
    if (ret) {
        LOG_ERR("Cannot init leds, (error: %d)", ret);
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

    init_list_storage();
    coap_client_utils_init(on_mtd_mode_toggle);

    return 0;
}
