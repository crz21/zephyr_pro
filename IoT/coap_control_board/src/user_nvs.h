#ifndef __USER_NVS_H__
#define __USER_NVS_H__

#include <stdint.h>

enum _op_id {
    // NULL_ID = 0,              //
    FIRST_USAGE_FLAG_ID=0x10,      //
    OLED_CONFIG_ID,           //
    NVS_ID_FULL_WHITELIST,//
    MAX_ID                    //
};

// void init_list_storage(void);
int init_list_storage(void);
void poweron_read_param(void);
void read_oled_light(void);
#endif
