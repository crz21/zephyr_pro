/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
// #include <dk_buttons_and_leds.h>
// #include <ram_pwrdn.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
// #include <zephyr/logging/log.h>
// #include <zephyr/pm/device.h>

// #include "ble_utils.h"
// #include "coap_client_utils.h"
// #include "list.h"

// LOG_MODULE_REGISTER(coap_client, CONFIG_COAP_CLIENT_LOG_LEVEL);

#define ENTER_KEY (0xE)
#define UP_KEY (0xD)
#define DOWN_KEY (0xB)
#define MENU_KEY (0x7)

#define CT_MAX_VALUE (0xFF)
#define LIGHT_MAX_VALUE (0xFF)
#define CT_MIN_VALUE (1)
#define LIGHT_MIN_VALUE (1)

enum KEY_STATE {
    KEY_CHECK = 0,  // 按键检测
    KEY_COMFIRM,    // 按键再次确认
    KEY_RELEASE,    // 按键释放
};  // 按键状态

enum KEY_ACTION {
    NULL_KEY = 0,
    SHORT_KEY,  // 短按按键
    LONG_KEY,   // 长按按键
};  // 按键动作

enum KEY_NAME {
    KEY_ENTER = 0,  //
    KEY_UP,         //
    KEY_DOWN,       //
    KEY_MENU,       //
    KEY_MAX
};

static uint8_t g_key_state;  // 按键状态
static uint8_t g_key_action_flag;
static uint16_t g_oled_close_time;

struct KEY_PARAM* g_key_par;

static const struct gpio_dt_spec buttons[KEY_MAX] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios),
};

// static struct gpio_callback button_cb_data;

// #define OT_CONNECTION_LED DK_LED1
// #define BLE_CONNECTION_LED DK_LED2
// #define MTD_SED_LED DK_LED3

// static struct k_work_q coap_client_workq;
// static struct k_work on_connect_work;
// static struct k_work on_disconnect_work;
// static struct k_work multicast_light_work;
// static struct k_work toggle_mtd_sed_work;

// #define COAP_CLIENT_WORKQ_STACK_SIZE 2048
// #define COAP_CLIENT_WORKQ_PRIORITY 5
// K_THREAD_STACK_DEFINE(coap_client_workq_stack_area, COAP_CLIENT_WORKQ_STACK_SIZE);

// #if CONFIG_BT_NUS
// static void on_ble_connect(struct k_work* item)
// {
//     ARG_UNUSED(item);

//     dk_set_led_on(BLE_CONNECTION_LED);
// }

// static void on_ble_disconnect(struct k_work* item)
// {
//     ARG_UNUSED(item);

//     dk_set_led_off(BLE_CONNECTION_LED);
// }

// #endif /* CONFIG_BT_NUS */

// static void on_ot_connect(struct k_work* item)
// {
//     ARG_UNUSED(item);

//     dk_set_led_on(OT_CONNECTION_LED);
// }

// static void on_ot_disconnect(struct k_work* item)
// {
//     ARG_UNUSED(item);

//     dk_set_led_off(OT_CONNECTION_LED);
// }

// static void on_mtd_mode_toggle(uint32_t med)
// {
// #if IS_ENABLED(CONFIG_PM_DEVICE)
//     const struct device* cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

//     if (!device_is_ready(cons)) {
//         return;
//     }

//     if (med) {
//         pm_device_action_run(cons, PM_DEVICE_ACTION_RESUME);
//     } else {
//         pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
//     }
// #endif
//     dk_set_led(MTD_SED_LED, med);
// }

// static void on_button_changed(uint32_t button_state, uint32_t has_changed)
// {
//     uint32_t buttons = button_state & has_changed;

//     if (buttons & DK_BTN2_MSK) {
//         k_work_submit_to_queue(&coap_client_workq, &multicast_light_work);
//     }

//     if (buttons & DK_BTN3_MSK) {
//         if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
//             k_work_submit_to_queue(&coap_client_workq, &toggle_mtd_sed_work);
//         }
//     }
// }

// void ot_connect_statue(bool statue)
// {
//     if (true) {
//         k_work_submit_to_queue(&coap_client_workq, &on_connect_work);
//     } else {
//         k_work_submit_to_queue(&coap_client_workq, &on_disconnect_work);
//     }
// }

// int main(void)
// {
//     int ret;

//     if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
//         power_down_unused_ram();
//     }

//     k_work_queue_init(&coap_client_workq);
//     k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
//                        K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);
//     k_work_init(&on_connect_work, on_ot_connect);
//     k_work_init(&on_disconnect_work, on_ot_disconnect);
//     k_work_init(&multicast_light_work, toggle_mesh_light_0);
//     if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
//         k_work_init(&toggle_mtd_sed_work, toggle_minimal_sleepy_end_device);
//         update_device_state();
//     }

//     ret = dk_buttons_init(on_button_changed);
//     if (ret) {
//         LOG_ERR("Cannot init buttons (error: %d)", ret);
//         return 0;
//     }

//     ret = dk_leds_init();
//     if (ret) {
//         LOG_ERR("Cannot init leds, (error: %d)", ret);
//         return 0;
//     }

// #if CONFIG_BT_NUS
//     struct bt_nus_cb nus_clbs = {
//         .received = NULL,
//         .sent = NULL,
//     };

//     ret = ble_utils_init(&nus_clbs, on_ble_connect, on_ble_disconnect);
//     if (ret) {
//         LOG_ERR("Cannot init BLE utilities");
//         return 0;
//     }

// #endif /* CONFIG_BT_NUS */

//     init_list_storage();
//     coap_client_utils_init(on_mtd_mode_toggle);

//     return 0;
// }

// 按键处理函数
// 返回按键值
// mode:0,不支持连续按;1,支持连续按;
// 返回值：
// 0，没有任何按键按下
// KEY0_PRES，KEY0按下
// KEY1_PRES，KEY1按下
// WKUP_PRES，WK_UP按下
// 注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
// g_key_state |00|00|00|00|:00->检测  01->确认  10->释放
uint8_t key_scan(void)
{
    static uint8_t time_cnt = 0;
    static uint8_t lock = 0;
    static uint8_t last_key_bit = 0;
    uint8_t key_bit;
    uint8_t i = 0;

    for (i = 0; i < KEY_MAX; i++) {
        key_bit |= gpio_pin_get_dt(&buttons[i]) << i;
    }

    switch (g_key_state) {
        // 按键未按下状态，此时判断Key的值
    case KEY_CHECK:  // 如果按键Key值为真，说明有按键开始按下，进入下一个状态
    {
        if (key_bit) {
            g_key_state = KEY_COMFIRM;
            last_key_bit = key_bit;
        }
        time_cnt = 0;  // 计数复位
        lock = 0;
    } break;

    case KEY_COMFIRM:  // 查看当前Key是否还是同样的按键，再次确认是否按下
    {
        if (key_bit) {
            /** 确认跟之前按键是否一样 */
            if (last_key_bit == key_bit) {
                if (!lock) {
                    lock = 1;
                }

                time_cnt++;

                /*按键时长判断*/
                if (time_cnt > 100) {  // 长按 1 s
                    g_key_action_flag = LONG_KEY;
                    time_cnt = 0;
                    lock = 0;                   // 重新检查
                    g_key_state = KEY_RELEASE;  // 需要进入按键释放状态
                }
            } else {
            }
        } else {
            if (lock) {                         // 不是第一次进入，  释放按键才执行
                g_key_action_flag = SHORT_KEY;  // 短按
                g_key_state = KEY_RELEASE;      // 需要进入按键释放状态
            } else {                            // 当前Key值为1，确认为抖动，则返回上一个状态
                g_key_state = KEY_CHECK;        // 返回上一个状态
            }
        }
    } break;

    case KEY_RELEASE:  // 当前Key位值全部为0，说明按键已经释放，返回开始状态
    {
        if ((0 == key_bit) || (LONG_KEY == g_key_action_flag)) {
            g_key_state = KEY_CHECK;
        }
    } break;

    default:
        break;
    }
    return key_bit;
}

void key_status(void)
{
    uint8_t read_key_function = 0;  // 读取当前按键功能
    uint8_t flash_data[10] = {0};

    if (sys_tim->_tim_10ms_flag) {
        sys_tim->_tim_10ms_flag = 0;
        read_key_function = key_scan();

        if (KEY_CHECK == g_key_state) {
            if (SHORT_KEY == g_key_action_flag)  // 短按按键
            {
                sys_tim->_1min_counter = 0;
                oled_send_cmd(0xAF);
                oled_par->key_on = 1;
                switch (read_key_function) {
                case ENTER_KEY: {
                    if (FATORY_CONFIRM_PAGE == oled_par->current_index) {
                        // Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                        oled_par->distance = 100;
                        flash_data[2] = 100;
                        oled_par->sensitivity = 50;
                        flash_data[3] = 50;
                        oled_par->transmitted_power = 100;
                        flash_data[4] = 100;
                        oled_par->oled_light = 0xEF;
                        flash_data[5] = 0xEF;
                        oled_par->oled_close_time = 30;
                        flash_data[6] = 30;

                        // Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                    } else if (SENSOR_PARAM_CONFIRM_PAGE == oled_par->current_index) {
                        // Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                        flash_data[2] = oled_par->distance;
                        flash_data[3] = oled_par->sensitivity;
                        flash_data[4] = oled_par->transmitted_power;

                        // Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                    } else if (OLED_PARAM_CONFIRM_PAGE == oled_par->current_index) {
                        // Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                        flash_data[5] = oled_par->oled_light;
                        flash_data[6] = oled_par->oled_close_time;

                        // Flash_Write(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));

                        oled_send_cmd(0x81);
                        oled_send_cmd(oled_par->oled_light);
                        g_oled_close_time = oled_par->oled_close_time * 100;
                    }
                    oled_par->current_index = table_op[oled_par->current_index].enter;
                } break;

                case UP_KEY: {
                    oled_par->current_index = table_op[oled_par->current_index].up;
                } break;

                case DOWN_KEY: {
                    if ((SENSOR_PARAM_PAGE_1 <= oled_par->current_index &&
                         oled_par->current_index <= SENSOR_PARAM_PAGE_3) ||
                        (OLED_PARAM_PAGE_1 <= oled_par->current_index &&
                         oled_par->current_index <= OLED_PARAM_PAGE_2)) {
                        oled_par->add_flag = 1;
                    } else {
                        oled_par->current_index = table_op[oled_par->current_index].down;
                    }
                } break;

                case MENU_KEY: {
                    if (SENSOR_PARAM_CONFIRM_PAGE == oled_par->current_index ||
                        SENSOR_PARAM_PAGE_1 == oled_par->current_index ||
                        SENSOR_PARAM_PAGE_2 == oled_par->current_index ||
                        SENSOR_PARAM_PAGE_3 == oled_par->current_index ||
                        OLED_PARAM_CONFIRM_PAGE == oled_par->current_index ||
                        OLED_PARAM_PAGE_1 == oled_par->current_index || OLED_PARAM_PAGE_2 == oled_par->current_index) {
                        Flash_Read(FLASH_SAVE_DATA_ADDR, flash_data, sizeof(flash_data));
                        oled_par->distance = flash_data[2];
                        oled_par->sensitivity = flash_data[3];
                        oled_par->transmitted_power = flash_data[4];
                        oled_par->oled_light = flash_data[5];
                        oled_par->oled_close_time = flash_data[6];
                    }
                    oled_par->current_index = MAIN_PAGE;
                } break;

                default:
                    break;
                }
            }
            g_key_action_flag = NULL_KEY;
        } else if (LONG_KEY == g_key_action_flag)  // 长按按键
        {
            g_key_action_flag = NULL_KEY;
        }
    }
}

void key_thread_entry(void* p1, void* p2, void* p3)
{
    uint8_t i = 0;
    uint8_t last_state = 0;

    for (i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
        gpio_is_ready_dt(&buttons[i]);
        gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
    }

    while (1) {
        key_status();
        k_msleep(10);
    }
}

K_THREAD_DEFINE(key_tid, 2048, key_thread_entry, NULL, NULL, NULL, 8, 0, 0);
