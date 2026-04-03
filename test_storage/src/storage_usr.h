#ifndef __STORAGE_USR_H__
#define __STORAGE_USR_H__

#include <stdint.h>

void save_parameter(uint8_t* buf, uint16_t len);
void load_parameter(uint8_t* buf, uint16_t len);
void init_storage(void);

#endif
