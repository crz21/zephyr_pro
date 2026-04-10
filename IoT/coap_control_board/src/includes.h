#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdint.h>

struct SYS_TIM_PARAM {
    uint8_t _10ms_counter;
    uint8_t _20ms_counter;
    uint8_t _100ms_counter;
    uint8_t _1min_counter;
    uint8_t _10ms_flag;
    uint8_t _1min_flag;
};


extern struct SYS_TIM_PARAM sys_tim;

#endif
