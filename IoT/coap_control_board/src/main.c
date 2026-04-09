#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "ble_utils.h"
#include "coap_client_button.h"
#include "coap_client_oled.h"
#include "coap_client_utils.h"
#include "coap_list.h"
#include "includes.h"
#include "user_nvs.h"

struct SYS_TIM_PARAM* sys_tim;

void sys_tick_timer__handle(void)
{
    sys_tim->_1ms_counter++;
    if (sys_tim->_1ms_counter >= 10) {
        sys_tim->_1ms_counter = 0;
        sys_tim->_10ms_flag = 1;
        sys_tim->_10ms_counter++;
        if (sys_tim->_10ms_counter >= 100) {
            sys_tim->_10ms_counter = 0;
            if (sys_tim->_1s_counter >= 60) {
                sys_tim->_1s_counter = 0;
                sys_tim->_1min_flag = 1;
            }
        }
    }
}

int main(void)
{
    int ret;

    if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
        power_down_unused_ram();
    }

    k_timer_start(&sys_tick_timer_id, K_MSEC(1), K_NO_WAIT);

    k_work_queue_init(&coap_client_workq);
    k_work_queue_start(&coap_client_workq, coap_client_workq_stack_area,
                       K_THREAD_STACK_SIZEOF(coap_client_workq_stack_area), COAP_CLIENT_WORKQ_PRIORITY, NULL);
    k_work_init(&on_connect_work, on_ot_connect);
    k_work_init(&on_disconnect_work, on_ot_disconnect);
    k_work_init(&multicast_light_work, toggle_mesh_light_0);
    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        k_work_init(&toggle_mtd_sed_work, toggle_minimal_sleepy_end_device);
        update_device_state();
    }

    ret = dk_buttons_init(on_button_changed);
    if (ret) {
        LOG_ERR("Cannot init buttons (error: %d)", ret);
        return 0;
    }

    ret = dk_leds_init();
    if (ret) {
        LOG_ERR("Cannot init leds, (error: %d)", ret);
        return 0;
    }

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
    coap_client_utils_init(on_mtd_mode_toggle);
    for (;;) {
    }

    return 0;
}

K_TIMER_DEFINE(sys_tick_timer_id, sys_tick_timer__handle, NULL);