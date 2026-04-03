#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/openthread.h>

#include "coap_client_utils.h"

LOG_MODULE_REGISTER(ot_coap_utils, CONFIG_OT_COAP_UTILS_LOG_LEVEL);

struct light_context {
    struct otInstance* ot;
    light_request_callback_t on_light_request;
};

static struct light_context light_cxt = {
    .ot = NULL,
    .on_light_request = NULL,
};

#define COAP_CLIENT_WORKQ_STACK_SIZE 2048
#define COAP_CLIENT_WORKQ_PRIORITY 5
K_THREAD_STACK_DEFINE(coap_client_workq_stack_area, COAP_CLIENT_WORKQ_STACK_SIZE);

static struct k_work_q coap_client_workq;
static struct k_work toggle_mtd_sed_work;
static struct k_work on_connect_work;
static struct k_work on_disconnect_work;

mtd_mode_toggle_cb_t on_mtd_mode_toggle;

static void light_request_handler(void* context, otMessage* message, const otMessageInfo* message_info)
{
    uint8_t command;

    ARG_UNUSED(context);

    if (otCoapMessageGetType(message) != OT_COAP_TYPE_CONFIRMABLE) {
        LOG_ERR("Light handler - Unexpected type of message");
        goto end;
    }

    if (otCoapMessageGetCode(message) != OT_COAP_CODE_PUT && otCoapMessageGetCode(message) != OT_COAP_CODE_POST) {
        LOG_ERR("Light handler - Unexpected CoAP code");
        goto end;
    }

    if (otMessageRead(message, otMessageGetOffset(message), &command, 1) != 1) {
        LOG_ERR("Light handler - Missing light command");
        goto end;
    }

    LOG_INF("Received light request: %c", command);

    srv_context.on_light_request(command);

end:
    return;
}

static void coap_default_handler(void* context, otMessage* message, const otMessageInfo* message_info)
{
    ARG_UNUSED(context);
    ARG_UNUSED(message);
    ARG_UNUSED(message_info);

    LOG_INF(
        "Received CoAP message that does not match any request "
        "or resource");
}

static void toggle_minimal_sleepy_end_device(struct k_work* item)
{
    otError error;
    otLinkModeConfig mode;

    openthread_mutex_lock();
    mode = otThreadGetLinkMode(light_cxt.ot);
    mode.mRxOnWhenIdle = !mode.mRxOnWhenIdle;
    error = otThreadSetLinkMode(light_cxt.ot, mode);
    openthread_mutex_unlock();

    if (error != OT_ERROR_NONE) {
        LOG_ERR("Failed to set MLE link mode configuration");
    } else {
        on_mtd_mode_toggle(mode.mRxOnWhenIdle);
    }
}

static void update_device_state(void)
{
    otLinkModeConfig mode = otThreadGetLinkMode(light_cxt.ot);
    on_mtd_mode_toggle(mode.mRxOnWhenIdle);
}

static void on_thread_state_changed(otChangedFlags flags, void* user_data)
{
    if (flags & OT_CHANGED_THREAD_ROLE) {
        switch (otThreadGetDeviceRole(light_cxt.ot)) {
        case OT_DEVICE_ROLE_CHILD:
        case OT_DEVICE_ROLE_ROUTER:
        case OT_DEVICE_ROLE_LEADER:
            k_work_submit_to_queue(&coap_client_workq, &on_connect_work);
            break;

        case OT_DEVICE_ROLE_DISABLED:
        case OT_DEVICE_ROLE_DETACHED:
        default:
            k_work_submit_to_queue(&coap_client_workq, &on_disconnect_work);
            break;
        }
    }
}
static struct openthread_state_changed_callback ot_state_chaged_cb = {.otCallback = on_thread_state_changed};

void coap_client_light_init(void)
{
    light_cxt.ot = openthread_get_default_instance();
    k_work_queue_init(&coap_client_workq);
    k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
                       K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);

    k_work_init(&on_connect_work, on_connect);
    k_work_init(&on_disconnect_work, on_disconnect);

    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_init(&toggle_mtd_sed_work, toggle_minimal_sleepy_end_device);
        update_device_state();
    }
}

void coap_client_toggle_minimal_sleepy_end_device(void)
{
    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_submit_to_queue(&coap_client_workq, &toggle_mtd_sed_work);
    }
}
