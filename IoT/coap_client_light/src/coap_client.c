/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <dk_buttons_and_leds.h>
#include <openthread/thread.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/openthread.h>

#include "ot_coap_utils.h"

LOG_MODULE_REGISTER(coap_server, CONFIG_COAP_SERVER_LOG_LEVEL);

#define LIGHT_LED_0 DK_LED1
#define LIGHT_LED_1 DK_LED2
#define LIGHT_LED_2 DK_LED3
#define LIGHT_LED_3 DK_LED4

#define COAP_SERVER_WORKQ_STACK_SIZE 512
#define COAP_SERVER_WORKQ_PRIORITY 5

K_THREAD_STACK_DEFINE(coap_server_workq_stack_area, COAP_SERVER_WORKQ_STACK_SIZE);
static struct k_work_q coap_server_workq;
static struct k_timer led_timer;

#if CONFIG_BT_NUS

#define COMMAND_REQUEST_UNICAST 'u'
#define COMMAND_REQUEST_MULTICAST 'm'
#define COMMAND_REQUEST_PROVISIONING 'p'

static void on_nus_received(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	LOG_INF("Received data: %c", data[0]);

	switch (*data) {
	case COMMAND_REQUEST_UNICAST:
		coap_client_toggle_one_light();
		break;

	case COMMAND_REQUEST_MULTICAST:
		coap_client_toggle_mesh_lights();
		break;

	case COMMAND_REQUEST_PROVISIONING:
		coap_client_send_provisioning_request();
		break;

	default:
		LOG_WRN("Received invalid data from NUS");
	}
}

static void on_ble_connect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_on(BLE_CONNECTION_LED);
}

static void on_ble_disconnect(struct k_work *item)
{
	ARG_UNUSED(item);

	dk_set_led_off(BLE_CONNECTION_LED);
}

#endif /* CONFIG_BT_NUS */

static void on_light_request(uint8_t command)
{
    static uint8_t led_0_staus;
    static uint8_t led_1_staus;
    static uint8_t led_2_staus;
    static uint8_t led_3_staus;

    switch (command) {
    case THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE:
        led_0_staus = !led_0_staus;
        dk_set_led(LIGHT_LED_0, led_0_staus);
        break;

    case THREAD_COAP_UTILS_LIGHT_1_CMD_TOGGLE:
        led_1_staus = !led_1_staus;
        dk_set_led(LIGHT_LED_1, led_1_staus);
        break;

    case THREAD_COAP_UTILS_LIGHT_2_CMD_TOGGLE:
        led_2_staus = !led_2_staus;
        dk_set_led(LIGHT_LED_2, led_2_staus);
        break;

    case THREAD_COAP_UTILS_LIGHT_3_CMD_TOGGLE:
        led_3_staus = !led_3_staus;
        dk_set_led(LIGHT_LED_3, led_3_staus);
        break;

    default:
        break;
    }
}

static void activate_provisioning(struct k_work* item)
{
    ARG_UNUSED(item);

    ot_coap_activate_provisioning();

    k_timer_start(&led_timer, K_MSEC(100), K_MSEC(100));
    k_timer_start(&provisioning_timer, K_SECONDS(5), K_NO_WAIT);

    LOG_INF("Provisioning activated");
}

static void deactivate_provisionig(void)
{
    k_timer_stop(&led_timer);
    k_timer_stop(&provisioning_timer);

    if (ot_coap_is_provisioning_active()) {
        ot_coap_deactivate_provisioning();
        LOG_INF("Provisioning deactivated");
    }
}

static void on_provisioning_timer_expiry(struct k_timer* timer_id)
{
    ARG_UNUSED(timer_id);

    deactivate_provisionig();
}

static void on_led_timer_expiry(struct k_timer* timer_id)
{
    static uint8_t val = 1;

    ARG_UNUSED(timer_id);

    dk_set_led(PROVISIONING_LED, val);
    val = !val;
}

static void on_led_timer_stop(struct k_timer* timer_id)
{
    ARG_UNUSED(timer_id);

    dk_set_led_off(PROVISIONING_LED);
}

static void on_thread_state_changed(otChangedFlags flags, void* user_data)
{
    if (flags & OT_CHANGED_THREAD_ROLE) {
        switch (otThreadGetDeviceRole(openthread_get_default_instance())) {
        case OT_DEVICE_ROLE_CHILD:
        case OT_DEVICE_ROLE_ROUTER:
        case OT_DEVICE_ROLE_LEADER:
            break;

        case OT_DEVICE_ROLE_DISABLED:
        case OT_DEVICE_ROLE_DETACHED:
        default:
            deactivate_provisionig();
            break;
        }
    }
}

static struct openthread_state_changed_callback ot_state_chaged_cb = {.otCallback = on_thread_state_changed};

int main(void)
{
    int ret;

    LOG_INF("Start CoAP-server sample");

    k_timer_init(&led_timer, on_led_timer_expiry, on_led_timer_stop);
    k_work_queue_init(&coap_server_workq);
    k_work_queue_start(&coap_server_workq, coap_server_workq_stack_area,
                       K_THREAD_STACK_SIZEOF(coap_server_workq_stack_area), COAP_SERVER_WORKQ_PRIORITY, NULL);
    ret = ot_coap_init(&on_light_request);
    if (ret) {
        LOG_ERR("Could not initialize OpenThread CoAP");
        goto end;
    }

    ret = dk_leds_init();
    if (ret) {
        LOG_ERR("Could not initialize leds, err code: %d", ret);
        goto end;
    }

    openthread_state_changed_callback_register(&ot_state_chaged_cb);
    openthread_run();

#if CONFIG_BT_NUS
    struct bt_nus_cb nus_clbs = {
        .received = on_nus_received,
        .sent = NULL,
    };

    ret = ble_utils_init(&nus_clbs, on_ble_connect, on_ble_disconnect);
    if (ret) {
        LOG_ERR("Cannot init BLE utilities");
        return 0;
    }

#endif /* CONFIG_BT_NUS */

end:
    return 0;
}
