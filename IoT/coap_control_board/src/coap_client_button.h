#ifndef __COAP_CLIENT_BUTTON_H__
#define __COAP_CLIENT_BUTTON_H__

// void ot_connect_statue(bool statue);
struct _key_op {
    uint8_t state;
    uint8_t action;
};

extern struct _key_op key_op;
#endif
