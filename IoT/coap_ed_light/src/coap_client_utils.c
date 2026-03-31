#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/openthread.h>

#include "coap_client_light.h"
#include "coap_client_list.h"

#define COAP_PORT 5683
struct otInstance* client_context_ot = NULL;
typedef void (*request_arry_t)(void*, otMessage*, const otMessageInfo*);

static otCoapResource resource_arr[] = {
    {LIGHT_URI_PATH, NULL, NULL, NULL},             //
    {NETWORK_FORMATION_URI_PATH, NULL, NULL, NULL}  //
};

static const request_arry_t request_table[] = {
    light_request_handler,
    network_formation_request_handler,
};

static void coap_default_handler(void* context, otMessage* message, const otMessageInfo* message_info)
{
    ARG_UNUSED(context);
    ARG_UNUSED(message);
    ARG_UNUSED(message_info);

    LOG_INF(
        "Received CoAP message that does not match any request "
        "or resource");
}

int coap_client_utils_init(void)
{
    otError error;

    client_context_ot = openthread_get_default_instance();
    if (!client_context_ot) {
        LOG_ERR("There is no valid OpenThread instance");
        error = OT_ERROR_FAILED;
        goto end;
    }

    otCoapSetDefaultHandler(client_context_ot, coap_default_handler, NULL);

    for (i = 0; i < sizeof(resource_arr) / sizeof(otCoapResource); i++) {
        resource_arr[i].mContext = client_context_ot;
        resource_arr[i].mHandler = request_table[i];
        otCoapAddResource(client_context_ot, &resource_arr[i]);
    }

    error = otCoapStart(client_context_ot, COAP_PORT);
    if (error != OT_ERROR_NONE) {
        LOG_ERR("Failed to start OT CoAP. Error: %d", error);
        goto end;
    }

    openthread_state_changed_callback_register(&ot_state_chaged_cb);
    openthread_run();

end:
    return error == OT_ERROR_NONE ? 0 : 1;
}