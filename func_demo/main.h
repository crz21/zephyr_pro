#ifndef __MAIN_H__
#define __MAIN_H__

#if defined(CONFIG_CUSTOM_BLINKY)
#include "devices/inc/func_blinky.h"
#endif

#if defined(CONFIG_CUSTOM_UART)
#include "devices/inc/func_uart.h"
#endif

#if defined(CONFIG_BMP280_PS)
#include "devices/inc/func_bmp280_ps.h"
#endif

#if defined(CONFIG_CUSTOM_SPI)
#include "devices/inc/func_custom_spi.h"
#endif

#endif
