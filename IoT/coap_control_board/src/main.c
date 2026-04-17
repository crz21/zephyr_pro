#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "ble_utils.h"
#include "coap_client_button.h"
#include "coap_client_oled.h"
#include "coap_client_utils.h"
#include "coap_list.h"
#include "includes.h"
#include "user_nvs.h"

struct SYS_TIM_PARAM sys_tim;
static struct k_timer sys_tick_timer_id;
struct k_mutex i2c_mutex;

void sys_tick_timer_handle(struct k_timer* timer)
{
    sys_tim._10ms_counter++;
    if (sys_tim._10ms_counter >= 10) {
        sys_tim._10ms_counter = 0;
        sys_tim._10ms_flag = 1;
        sys_tim._20ms_counter++;
        if (sys_tim._20ms_counter >= 2) {
            sys_tim._20ms_counter = 0;
        }

        sys_tim._100ms_counter++;
        if (sys_tim._100ms_counter >= 100) {
            sys_tim._100ms_counter = 0;

            sys_tim._1min_counter++;
            if (sys_tim._1min_counter >= 60) {
                sys_tim._1min_counter = 0;
                sys_tim._1min_flag = 1;
            }
        }
    }
}

int main(void)
{
    k_mutex_init(&i2c_mutex);
    k_timer_init(&sys_tick_timer_id, sys_tick_timer_handle, NULL);
    k_timer_start(&sys_tick_timer_id, K_MSEC(1), K_MSEC(1));

#if CONFIG_OPENTHREAD
    coap_client_utils_init();
#endif

    init_list_storage();
 
#if CONFIG_BT_NUS
    struct bt_nus_cb nus_clbs = {
        .received = NULL,
        .sent = NULL,
    };

    ret = ble_utils_init(&nus_clbs, on_ble_connect, on_ble_disconnect);
    if (ret) {
        LOG_ERR("Cannot init BLE utilities");
        return 0;
    }

#endif /* CONFIG_BT_NUS */

    poweron_read_param();
    read_oled_light();

    while (1) {
        k_sleep(K_MSEC(1));
    }

    return 0;
}

static void param_init(void)
{
    oled_par.current_index = WELCOME_PAGE;
    oled_par.key_on = 1;
}

void sys_thread(void)
{
    param_init();

    for (;;) {
        k_sleep(K_MSEC(1));
        if (oled_par.current_index == WELCOME_PAGE || oled_par.current_index == MAIN_PAGE) {
            sys_tim._1min_counter = 0;
        }

        if (sys_tim._1min_flag) {
            sys_tim._1min_flag = 0;
            oled_par.key_on = 1;
            oled_par.current_index = MAIN_PAGE;
            clear_oled();
        }
    }
}
K_THREAD_DEFINE(sys_thread_id, 1024, sys_thread, NULL, NULL, NULL, 2, 0, 0);
