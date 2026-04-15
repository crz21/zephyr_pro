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

#define SYS_THREAD_PRIORITY (10)

struct SYS_TIM_PARAM sys_tim;
static struct k_timer sys_tick_timer_id;
struct k_mutex i2c_mutex;

void sys_tick_timer_handle(struct k_timer* timer)
{
    sys_tim._10ms_counter++;
    if (sys_tim._10ms_counter >= 10) {
        sys_tim._10ms_counter = 0;
        sys_tim._10ms_flag = 1;
        // printk("hsys_tick_timer_handle = %d\n",sys_tim._10ms_flag);
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
// K_TIMER_DEFINE(sys_tick_timer_id, sys_tick_timer_handle, NULL);
int main(void)
{
    // int ret;

    // if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
    //     power_down_unused_ram();
    // }


    k_mutex_init(&i2c_mutex);
    k_timer_init(&sys_tick_timer_id, sys_tick_timer_handle, NULL);
    k_timer_start(&sys_tick_timer_id, K_MSEC(1), K_MSEC(1));
    coap_client_utils_init();
    // k_work_queue_init(&coap_client_workq);
    // k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
    //                    K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);
    // k_work_init(&on_connect_work, on_ot_connect);
    // k_work_init(&on_disconnect_work, on_ot_disconnect);
    // k_work_init(&multicast_light_work, toggle_mesh_light_0);
    // if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
    //     k_work_init(&toggle_mtd_sed_work, toggle_minimal_sleepy_end_device);
    //     update_device_state();
    // }

    // ret = dk_buttons_init(on_button_changed);
    // if (ret) {
    //     LOG_ERR("Cannot init buttons (error: %d)", ret);
    //     return 0;
    // }

    // ret = dk_leds_init();
    // if (ret) {
    //     LOG_ERR("Cannot init leds, (error: %d)", ret);
    //     return 0;
    // }

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

    init_list_storage();
    // coap_client_utils_init(on_mtd_mode_toggle);
    while (1) {
        k_sleep(K_MSEC(1));
    }

    return 0;
}

static void param_init(void)
{
    // oled_par.poweron_flag = 1;
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
