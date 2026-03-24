#ifndef __IOT_PRODUCER_H__
#define __IOT_PRODUCER_H__

#include <stdint.h>
void create_event_payload(uint8_t* data, uint16_t len, uint32_t event);

#endif
