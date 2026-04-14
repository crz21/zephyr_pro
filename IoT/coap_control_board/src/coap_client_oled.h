#ifndef __COAP_CLIENT_OLED_H__
#define __COAP_CLIENT_OLED_H__

enum TABLE_INDEX {
    WELCOME_PAGE = 0,
    MAIN_PAGE,
    SET_PAGE_1,
    SET_PAGE_2,
    SET_PAGE_3,
    SET_PAGE_4,
    SENSOR_PARAM_PAGE_1,
    SENSOR_PARAM_PAGE_2,
    SENSOR_PARAM_PAGE_3,
    OLED_PARAM_PAGE_1,
    OLED_PARAM_PAGE_2,
    VISION_PAGE,
    SENSOR_PARAM_CONFIRM_PAGE,
    OLED_PARAM_CONFIRM_PAGE,
    FATORY_CONFIRM_PAGE,
    MAX_INDEX,
};
struct _oled_param {
    uint16_t oled_close_time;
    uint8_t distance;
    uint8_t sensitivity;
    uint8_t transmitted_power;
    uint8_t oled_light;
    uint8_t current_index;
    uint8_t pre_index;
    uint8_t key_on;
    uint8_t add_flag;
};
extern struct _oled_param oled_par;
struct _table_op {
    uint8_t current;
    uint8_t up;
    uint8_t down;
    uint8_t enter;
    uint8_t min_page;
    uint8_t max_page;
    void (*table_operation)();
};

extern struct _table_op table_op[MAX_INDEX];

void clear_oled(void);
void oled_send_cmd(uint8_t o_command);
#endif
