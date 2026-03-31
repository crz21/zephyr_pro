#include <zephyr/net/net_ip.h>

// static struct k_timer send_param_timer;

// #define UNPAIRED (0)
// #define ALLOWLIST (0x01)
// #define DENYLIST (0x02)

// #define ED_TYPE_LIGHT (0x01)
/**
 上电主动上送mac和ip
 没入网：广播能亮，单播不能亮
 黑名单：广播和单播都不能亮
 白名单：广播和单播都能亮
 */

/** 白名单：1 - 01b 没入网：0 - 00b 黑名单：2 - 10b */
// static uint8_t whitelist_flag;
// static local_mac_addr[8];
// static local_ip6_addr[16];

struct list_context {
    bool network_formation_enabled;
    network_formation_request_callback_t on_network_formation_request;
};

static struct list_context list_cxt = {false, NULL};

static otError network_formation_response_send(otMessage* request_message, const otMessageInfo* message_info)
{
    otError error = OT_ERROR_NO_BUFS;
    otMessage* response;
    const void* payload;
    uint16_t payload_size;

    struct otInstance* context_ot = openthread_get_default_instance();

    response = otCoapNewMessage(scontext_ot, NULL);
    if (response == NULL) {
        goto end;
    }

    otCoapMessageInit(response, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);

    error = otCoapMessageSetToken(response, otCoapMessageGetToken(request_message),
                                  otCoapMessageGetTokenLength(request_message));
    if (error != OT_ERROR_NONE) {
        goto end;
    }

    error = otCoapMessageSetPayloadMarker(response);
    if (error != OT_ERROR_NONE) {
        goto end;
    }

    memcpy(payload, otThreadGetMeshLocalEid(context_ot), sizeof(otIp6Address));
    payload_size = sizeof(otIp6Address);
    memcpy(payload + payload_size, otLinkGetExtendedAddress(context_ot), sizeof(otExtAddress));
    payload_size += sizeof(otExtAddress);

    error = otMessageAppend(response, payload, payload_size);
    if (error != OT_ERROR_NONE) {
        goto end;
    }

    error = otCoapSendResponse(context_ot, response, message_info);

    LOG_HEXDUMP_INF(payload, payload_size, "Sent network_formation response:");

end:
    if (error != OT_ERROR_NONE && response != NULL) {
        otMessageFree(response);
    }

    return error;
}

void network_formation_request_handler(void* context, otMessage* message, const otMessageInfo* message_info)
{
    otError error;
    otMessageInfo msg_info;

    ARG_UNUSED(context);

    if (!list_cxt.network_formation_enabled) {
        LOG_WRN(
            "Received network_formation request but network_formation "
            "is disabled");
        return;
    }

    LOG_INF("Received network_formation request");

    if ((otCoapMessageGetType(message) == OT_COAP_TYPE_CONFIRMABLE) &&
        (otCoapMessageGetCode(message) == OT_COAP_CODE_GET)) {
        msg_info = *message_info;
        memset(&msg_info.mSockAddr, 0, sizeof(msg_info.mSockAddr));

        error = network_formation_response_send(message, &msg_info);
        if (error == OT_ERROR_NONE) {
            list_cxt.on_network_formation_request();
            list_cxt.network_formation_enabled = false;
        }
    }
}

void coap_list_init(network_formation_request_callback_t on_network_formation_request)
{
    list_cxt.network_formation_enabled = false;
    list_cxt.on_network_formation_request = on_network_formation_request;
}