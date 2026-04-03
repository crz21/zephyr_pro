#ifndef __USER_NVS_H__
#define __USER_NVS_H__

#include <stdint.h>

int init_list_storage(void);
uint8_t read_ip_addr(uint8_t* ip_buf, uint8_t ip_len);
uint8_t read_first_time_usage_flag(void);
void save_first_time_usage_flag(uint8_t flag);
void read_param(uint8_t* param_arr);
#endif
