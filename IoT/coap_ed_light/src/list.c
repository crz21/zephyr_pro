#include <zephyr/net/net_ip.h>

static struct k_timer send_param_timer;

#define UNPAIRED (0)
#define ALLOWLIST (0x01)
#define DENYLIST (0x02)

#define ED_TYPE_LIGHT (0x01)
/**
 上电主动上送mac和ip
 没入网：广播能亮，单播不能亮
 黑名单：广播和单播都不能亮
 白名单：广播和单播都能亮
 */

/** 白名单：1 - 01b 没入网：0 - 00b 黑名单：2 - 10b */
static uint8_t whitelist_flag;
static local_mac_addr[8];
static local_ip6_addr[16];

void save_ip_addr(void)
{
    struct otInstance* instance = openthread_get_default_instance();

    const otIp6Address* ip_addr = otThreadGetMeshLocalEid(instance);

    write_flash_section(ip_addr, 16);
}

void first_poweron(void) {}

static otError provisioning_response_send(otMessage* request_message, const otMessageInfo* message_info)
{
    otError error = OT_ERROR_NO_BUFS;
    otMessage* response;
    const void* payload;
    uint16_t payload_size;

    response = otCoapNewMessage(srv_context.ot, NULL);
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

    payload = otThreadGetMeshLocalEid(srv_context.ot);
    payload_size = sizeof(otIp6Address);

    error = otMessageAppend(response, payload, payload_size);
    if (error != OT_ERROR_NONE) {
        goto end;
    }

    error = otCoapSendResponse(srv_context.ot, response, message_info);

    LOG_HEXDUMP_INF(payload, payload_size, "Sent provisioning response:");

end:
    if (error != OT_ERROR_NONE && response != NULL) {
        otMessageFree(response);
    }

    return error;
}

static void end_param_timer_expiry(struct k_timer* timer_id)
{
    static uint8_t val = 1;

    ARG_UNUSED(timer_id);

    dk_set_led(PROVISIONING_LED, val);
    val = !val;
}

// static void end_param_timer_stop(struct k_timer *timer_id)
// {
// 	ARG_UNUSED(timer_id);

// 	dk_set_led_off(PROVISIONING_LED);
// }

k_timer_start(&send_param_timer, K_MSEC(1000), K_MSEC(1000));
k_timer_stop(&send_param_timer);

uint8_t list_init(void)
{
    struct otInstance* instance = openthread_get_default_instance();
    const otExtAddress* ext_addr = otLinkGetExtendedAddress(instance);
    const otIp6Address* ip_addr = otThreadGetMeshLocalEid(instance);

    memcpy(local_mac_addr, ext_addr->m8, 8);
    memcpy(local_ip6_addr, ip_addr->m16, 16);

    k_timer_init(&send_param_timer, end_param_timer_expiry, NULL);

    return 0;
}