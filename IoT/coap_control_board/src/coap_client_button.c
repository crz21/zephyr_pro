/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/input/input.h>

#include "coap_client_oled.h"
#include "coap_client_button.h"
#include "includes.h"
#include <stdio.h>

#define ENTER_KEY (0x1)
#define UP_KEY	  (0x2)
#define DOWN_KEY  (0x4)
#define MENU_KEY  (0x8)

#define CT_MAX_VALUE	(0xFF)
#define LIGHT_MAX_VALUE (0xFF)
#define CT_MIN_VALUE	(1)
#define LIGHT_MIN_VALUE (1)

enum KEY_STATE {
	KEY_CHECK = 0, // 按键检测
	KEY_COMFIRM,   // 按键再次确认
	KEY_RELEASE,   // 按键释放
}; // 按键状态

enum KEY_ACTION {
	KEY_NULL = 0,
	KEY_SHORT, // 短按按键
	KEY_LONG,  // 长按按键
}; // 按键动作

enum KEY_NAME {
	KEY_ENTER = 0, //
	KEY_UP,	       //
	KEY_DOWN,      //
	KEY_MENU,      //
	KEY_MAX
};

struct _key_op key_op;

static const struct gpio_dt_spec buttons[KEY_MAX] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios),
};

// 按键处理函数
// 返回按键值
// mode:0,不支持连续按;1,支持连续按;
// 返回值：
// 0，没有任何按键按下
// KEY0_PRES，KEY0按下
// KEY1_PRES，KEY1按下
// WKUP_PRES，WK_UP按下
// 注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
// key_op.state |00|00|00|00|:00->检测  01->确认  10->释放
/** 60ms-200ms之间采集按下的次数6-20次为短按*/
uint8_t key_scan(void)
{
	static uint8_t time_cnt = 0;
	static uint8_t lock = 0;
	uint8_t last_key_bit = 0;
	static uint8_t current_key_bit;
	uint8_t key_bit;
	uint8_t i = 0;

	for (i = 0; i < KEY_MAX; i++) {
		key_bit |= (gpio_pin_get_dt(&buttons[i]) << i);
	}

	switch (key_op.state) {
		// 按键未按下状态，此时判断Key的值
	case KEY_CHECK: // 如果按键Key值为真，说明有按键开始按下，进入下一个状态
		if (key_bit) {
			key_op.state = KEY_COMFIRM;
			current_key_bit = key_bit;
		}
		time_cnt = 0; // 计数复位
		lock = 0;
		break;

	case KEY_COMFIRM: // 查看当前Key是否还是同样的按键，再次确认是否按下
		if (key_bit) {
			last_key_bit = key_bit;
			if (last_key_bit == current_key_bit) {
				if (!lock) {
					lock = 1;
				}

				time_cnt++;

				/*按键时长判断*/
				if (time_cnt > 100) { // 长按 1 s
					key_op.action = KEY_LONG;
					time_cnt = 0;
					lock = 0;		    // 重新检查
					key_op.state = KEY_RELEASE; // 需要进入按键释放状态
				}
			} else {
				key_op.state = KEY_CHECK;
			}
		} else {
			if (lock) {			   // 不是第一次进入，  释放按键才执行
				key_op.action = KEY_SHORT; // 短按
				key_op.state = KEY_RELEASE; // 需要进入按键释放状态
			} else { // 当前Key值为1，确认为抖动，则返回上一个状态
				key_op.state = KEY_CHECK; // 返回上一个状态
			}
		}
		break;

	case KEY_RELEASE: // 当前Key位值全部为0，说明按键已经释放，返回开始状态
		if ((0 == key_bit) || (KEY_LONG == key_op.action)) {
			key_op.state = KEY_CHECK;
		}
		break;

	default:
		break;
	}
	return current_key_bit;
}

void key_status(void)
{
	uint8_t read_key_function = 0; // 读取当前按键功能
	uint8_t flash_data[10] = {0};

	if (sys_tim._10ms_flag) {
		sys_tim._10ms_flag = 0;
		read_key_function = key_scan();

		if (KEY_CHECK == key_op.state) {
			if (KEY_SHORT == key_op.action) {
				sys_tim._1min_counter = 0;
				oled_send_cmd(0xAF);
				oled_par.key_on = 1;
				switch (read_key_function) {
				case ENTER_KEY:
					printf("ENTER_KEY\n");
					if (FATORY_CONFIRM_PAGE == oled_par.current_index) {
						// Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
						oled_par.distance = 100;
						flash_data[2] = 100;
						oled_par.sensitivity = 50;
						flash_data[3] = 50;
						oled_par.transmitted_power = 100;
						flash_data[4] = 100;
						oled_par.oled_light = 0xEF;
						flash_data[5] = 0xEF;
						oled_par.oled_close_time = 30;
						flash_data[6] = 30;

						// Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
					} else if (SENSOR_PARAM_CONFIRM_PAGE ==
						   oled_par.current_index) {
						// Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
						flash_data[2] = oled_par.distance;
						flash_data[3] = oled_par.sensitivity;
						flash_data[4] = oled_par.transmitted_power;

						// Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
					} else if (OLED_PARAM_CONFIRM_PAGE ==
						   oled_par.current_index) {
						// Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
						flash_data[5] = oled_par.oled_light;
						flash_data[6] = oled_par.oled_close_time;

						// Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));

						oled_send_cmd(0x81);
						oled_send_cmd(oled_par.oled_light);
						// g_oled_close_time = oled_par.oled_close_time *
						// 100;
					}
					oled_par.current_index =
						table_op[oled_par.current_index].enter;
					break;

				case UP_KEY:
					printf("UP_KEY\n");
					oled_par.current_index =
						table_op[oled_par.current_index].up;
					break;

				case DOWN_KEY:
					printf("DOWN_KEY\n");
					if ((SENSOR_PARAM_PAGE_1 <= oled_par.current_index &&
					     oled_par.current_index <= SENSOR_PARAM_PAGE_3) ||
					    (OLED_PARAM_PAGE_1 <= oled_par.current_index &&
					     oled_par.current_index <= OLED_PARAM_PAGE_2)) {
						oled_par.add_flag = 1;
					} else {
						oled_par.current_index =
							table_op[oled_par.current_index].down;
					}
					break;

				case MENU_KEY:
					printf("MENU_KEY\n");
					if (SENSOR_PARAM_CONFIRM_PAGE == oled_par.current_index ||
					    SENSOR_PARAM_PAGE_1 == oled_par.current_index ||
					    SENSOR_PARAM_PAGE_2 == oled_par.current_index ||
					    SENSOR_PARAM_PAGE_3 == oled_par.current_index ||
					    OLED_PARAM_CONFIRM_PAGE == oled_par.current_index ||
					    OLED_PARAM_PAGE_1 == oled_par.current_index ||
					    OLED_PARAM_PAGE_2 == oled_par.current_index) {
						// Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data,
						// sizeof(flash_data));
						oled_par.distance = flash_data[2];
						oled_par.sensitivity = flash_data[3];
						oled_par.transmitted_power = flash_data[4];
						oled_par.oled_light = flash_data[5];
						oled_par.oled_close_time = flash_data[6];
					}
					oled_par.current_index = MAIN_PAGE;
					break;

				default:
					break;
				}
				key_op.action = KEY_NULL;
			} else if (KEY_LONG == key_op.action) { // 长按按键
				key_op.action = KEY_NULL;
			}
		}
	}
}

void key_thread_entry(void *p1, void *p2, void *p3)
{
	uint8_t i = 0;

	for (i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
		gpio_is_ready_dt(&buttons[i]);
		gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
	}

	for (;;) {
		key_status();
		k_msleep(1);
	}
}

K_THREAD_DEFINE(key_tid, 2048, key_thread_entry, NULL, NULL, NULL, 3, 0, 0);
