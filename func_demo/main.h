#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef(CONFIG_CUSTOM_BLINKY)
#include "devices/inc/func_blinky.h"
#endif

#ifdef(CONFIG_CUSTOM_UART)
#include "devices/inc/func_uart.h"
#endif

#ifdef(CONFIG_BMP280_PS)
#include "devices/inc/func_bmp280_ps.h"
#endif

#ifdef(CONFIG_CUSTOM_SPI)
#include "devices/inc/func_custom_spi.h"
#endif

#endif
