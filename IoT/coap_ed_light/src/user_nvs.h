#ifndef __USER_NVS_H__
#define __USER_NVS_H__

int init_list_storage(void);
int write_flash_section(uint8_t* data_buf, uint16_t len);
int read_flash_section(uint8_t* data_buf, uint16_t len);

#endif
