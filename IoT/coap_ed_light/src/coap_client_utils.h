#ifndef __COAP_CLIENT_UTILS_H__
#define __COAP_CLIENT_UTILS_H__

enum
{
    ZONE_UNASSIGNED=0,
    ZONE_LIVING_ROOM=1,
    ZONE_BEDROOM=2
};

int coap_client_utils_init(void);

#endif
