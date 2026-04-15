/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <net/coap_utils.h>
#include <openthread.h>
#include <openthread/coap.h>
#include <openthread/thread.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>

#include "coap_client_button.h"

LOG_MODULE_REGISTER(coap_client_utils, CONFIG_COAP_CLIENT_UTILS_LOG_LEVEL);

#define COAP_PORT 5683
#define PROVISIONING_URI_PATH "provisioning"
#define COAP_CLIENT_WORKQ_STACK_SIZE 2048
#define COAP_CLIENT_WORKQ_PRIORITY   5

K_THREAD_STACK_DEFINE(coap_client_workq_stack_area, COAP_CLIENT_WORKQ_STACK_SIZE);
static struct k_work_q coap_client_workq;
static struct k_work provisioning_work;

static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

struct server_context {
    struct otInstance* ot;
};

static struct server_context srv_context = {
    .ot = NULL,
};

enum light_command {
    THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE = '0',
};

// mtd_mode_toggle_cb_t on_mtd_mode_toggle;

void send_provisioning_request(struct k_work* item)
{
    otError error = OT_ERROR_NONE;
    otMessage* message = NULL;
    otMessageInfo messageInfo;

    printf("send_provisioning_request");
    memset(&messageInfo, 0, sizeof(messageInfo));
    otIp6AddressFromString("ff03::1", &messageInfo.mPeerAddr);
    messageInfo.mPeerPort = COAP_PORT;

    message = otCoapNewMessage(srv_context.ot, NULL);
    if (message == NULL) {
        goto exit;
    }

    otCoapMessageInit(message, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);

    error = otCoapMessageAppendUriPathOptions(message, PROVISIONING_URI_PATH);
    if (error != OT_ERROR_NONE) {
        goto exit;
    }

    otCoapMessageSetPayloadMarker(message);
    uint8_t command = THREAD_COAP_UTILS_LIGHT_0_CMD_TOGGLE;
    error = otMessageAppend(message, &command, sizeof(command));
    if (error != OT_ERROR_NONE) {
        goto exit;
    }

    error = otCoapSendRequest(srv_context.ot, message, &messageInfo, NULL, NULL);

exit:
    if (error != OT_ERROR_NONE && message != NULL) {
        otMessageFree(message);
        LOG_ERR("Failed to send CoAP request: %d", error);
    } else {
        LOG_INF("CoAP multicast request sent successfully");
    }
}

static void on_thread_state_changed(otChangedFlags flags, void* user_data)
{
    if (flags & OT_CHANGED_THREAD_ROLE) {
        switch (otThreadGetDeviceRole(srv_context.ot)) {
        case OT_DEVICE_ROLE_CHILD:
        case OT_DEVICE_ROLE_ROUTER:
        case OT_DEVICE_ROLE_LEADER:
            break;

        case OT_DEVICE_ROLE_DISABLED:
        case OT_DEVICE_ROLE_DETACHED:
        default:
            break;
        }
    }
}
static struct openthread_state_changed_callback ot_state_chaged_cb = {.otCallback = on_thread_state_changed};

int coap_client_utils_init(void)
{
    otError error;
    int ret = 0;

    // on_mtd_mode_toggle = on_toggle;
    srv_context.ot = openthread_get_default_instance();

    for (int i = 0; i < 4; i++) {
        if (!gpio_is_ready_dt(&leds[i])) {
            return 0;
        }

        ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_ACTIVE);
        if (ret < 0) {
            return 0;
        }
    }

    if (!srv_context.ot) {
        LOG_ERR("There is no valid OpenThread instance");
        error = OT_ERROR_FAILED;
        goto end;
    }

    error = otCoapStart(srv_context.ot, COAP_PORT);
    if (error != OT_ERROR_NONE) {
        LOG_ERR("Failed to start OT CoAP. Error: %d", error);
        goto end;
    }

	k_work_queue_init(&coap_client_workq);

	k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
			   K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area),
			   COAP_CLIENT_WORKQ_PRIORITY, NULL);

	k_work_init(&provisioning_work, send_provisioning_request);

    openthread_state_changed_callback_register(&ot_state_chaged_cb);
    openthread_run();

end:
    return error == OT_ERROR_NONE ? 0 : 1;
}

static void submit_work_if_connected(struct k_work *work)
{
	if (1) {
		k_work_submit_to_queue(&coap_client_workq, work);
	} else {
		LOG_INF("Connection is broken");
	}
}

void coap_client_send_provisioning_request(void)
{
	submit_work_if_connected(&provisioning_work);
}
