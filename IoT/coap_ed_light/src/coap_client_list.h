#ifndef __COAP_CLIENT_LIST_H__
#define __COAP_CLIENT_LIST_H__

#define NETWORK_FORMATION_URI_PATH "network_formation"

typedef (*network_formation_request_callback_t)(int);
void coap_list_init(network_formation_request_callback_t on_network_formation_request);
void network_formation_request_handler(void* context, otMessage* message, const otMessageInfo* message_info);

#endif
