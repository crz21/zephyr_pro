#include <zephyr/device.h>

#include "ble_utils.h"
#include "coap_client_button.h"
#include "coap_client_oled.h"
#include "coap_client_utils.h"
#include "coap_list.h"
#include "user_nvs.h"

int main(void)
{
    int ret;

    if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
        power_down_unused_ram();
    }

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

    return 0;
}