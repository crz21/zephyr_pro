

#include <stdint.h>
#include <stdio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "coap_client_button.h"
#include "font_table.h"
#include "coap_client_oled.h"
#include "includes.h"

#define OLED_SSD1306_PRIORITY (5)
#define MAX_COLUMN (128)      // 最大列数
#define MAX_PAGE (64)         // 最大行数 1个page代表8小行所以为 64 / 8
#define MAX_COLUMN_WORDS (8)  // 一行最大显示字数
#define ADJUST_VALUE (30)

#define OLED_SSD1306_NODE DT_NODELABEL(gen_oled_ssd1306)
const struct i2c_dt_spec oled_ssd1306_spec = I2C_DT_SPEC_GET(OLED_SSD1306_NODE);

struct _draw_word_op {
    uint8_t page_offset;
    uint8_t column_offset;
    void* ptr_word;
    uint8_t word_len;
    void (*dram_func)(uint8_t, uint8_t, char*, uint8_t);
};

struct _oled_param oled_par;

void draw_welcom_page(void);
void draw_main_page(void);
void draw_set_page(void);
void draw_sensor_param_page(void);
void draw_oled_param_page(void);
void draw_vision_page(void);
void draw_param_confirm_page(void);
void draw_fatory_confirm_page(void);
void draw_part(uint8_t page_offset, uint8_t column_offset, char* str, uint8_t word_len);
void draw_provisioning_url_page(void);

struct _draw_word_op welcome_page_op[] = {
    // {2, 4, "\xBB\xB6\xD3\xAD\xCA\xB9\xD3\xC3", 4, &draw_part},          // 欢迎使用
    // {5, 3, "\xC7\xEB\xB0\xB4\xC8\xB7\xC8\xCF\xBC\xFC", 5, &draw_part},  // 请按确认键

    {2, 4, "欢迎使用", 4, &draw_part},          // 欢迎使用
    {5, 3, "请按确认键", 5, &draw_part},  // 请按确认键
};

struct _draw_word_op main_page_op[] = {
    // {0, 0, "\xD7\xB4\xCC\xAC", 2, &draw_part},                   // 状态
    // {3, 0, "\xD3\xD0\xC8\xCB\xBE\xE0\xC0\xEB", 4, &draw_part},   // 有人距离
    // {6, 0, "\xCE\xDE\xC8\xCB\xBE\xE0\xC0\xEB", 4, &draw_part},   // 无人距离

    {0, 0, "客厅", 2, &draw_part},                   // 状态
    {3, 0, "房间", 4, &draw_part},   // 有人距离
    {6, 0, "厨房", 4, &draw_part},   // 无人距离
};

struct _draw_word_op set_page1_op[] = {
    // {0, 1, "\xB2\xCE\xCA\xFD\xC9\xE8\xD6\xC3", 4, &draw_part},  // 参数设置
    // {3, 1, "\xC6\xC1\xC4\xBB\xC9\xE8\xD6\xC3", 4, &draw_part},  // 屏幕设置
    // {6, 1, "\xB0\xE6\xB1\xBE", 2, &draw_part},                  // 版本

    {0, 1, "参数设置", 4, &draw_part},  // 参数设置
    {3, 1, "屏幕设置", 4, &draw_part},  // 屏幕设置
    {6, 1, "版本", 2, &draw_part},                  // 版本
};

struct _draw_word_op set_page2_op[] = {
    // {0, 1, "\xBB\xD6\xB8\xB4\xC4\xAC\xC8\xCF", 4, &draw_part},  // 恢复默认

    {0, 1, "恢复默认", 4, &draw_part},  // 恢复默认
    {3, 1, "组网", 2, &draw_part},  // 屏幕设置
};

struct _draw_word_op sensor_param_page_op[] = {
    {0, 1, "距离", 2, &draw_part},                  // 距离
    {3, 1, "灵敏度", 3, &draw_part},          // 灵敏度
    {6, 1, "发射功率", 4, &draw_part},  // 发射功率
};

struct _draw_word_op oled_param_page_op[] = {
    {0, 1, "亮度", 2, &draw_part},                  // 亮度
    {3, 1, "息屏时间", 4, &draw_part},  // 息屏时间
};

struct _draw_word_op param_word_op[] = {
    {0, 7, "没有目标", 4, &draw_part},  // 没有目标
    {0, 7, "运动目标", 4, &draw_part},  // 运动目标
    {0, 7, "静止目标", 4, &draw_part},  // 静止目标
    {0, 7, "运动静止", 4, &draw_part},  // 运动静止
};

struct _table_op table_op[MAX_INDEX] = {
    /* 欢迎菜单 */
    {WELCOME_PAGE, WELCOME_PAGE, WELCOME_PAGE, MAIN_PAGE, WELCOME_PAGE, WELCOME_PAGE, &draw_welcom_page},

    /* 主界面 */
    {MAIN_PAGE, MAIN_PAGE, MAIN_PAGE, SET_PAGE_1, MAIN_PAGE, MAIN_PAGE, &draw_main_page},

    /* 1级菜单 */
    {SET_PAGE_1, SET_PAGE_2, SET_PAGE_5, SENSOR_PARAM_PAGE_1, SET_PAGE_1, SET_PAGE_3, &draw_set_page},
    {SET_PAGE_2, SET_PAGE_3, SET_PAGE_1, OLED_PARAM_PAGE_1, SET_PAGE_1, SET_PAGE_3, &draw_set_page},
    {SET_PAGE_3, SET_PAGE_4, SET_PAGE_2, VISION_PAGE, SET_PAGE_1, SET_PAGE_3, &draw_set_page},
    {SET_PAGE_4, SET_PAGE_5, SET_PAGE_3, FATORY_CONFIRM_PAGE, SET_PAGE_4, SET_PAGE_5, &draw_set_page},
    {SET_PAGE_5, SET_PAGE_1, SET_PAGE_4, PROVISIONING_URL_PAGE, SET_PAGE_4, SET_PAGE_5, &draw_set_page},

    /* 2级菜单 */
    {SENSOR_PARAM_PAGE_1, SENSOR_PARAM_PAGE_2, SENSOR_PARAM_PAGE_1, SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_PAGE_1,
     SENSOR_PARAM_PAGE_3, &draw_sensor_param_page},
    {SENSOR_PARAM_PAGE_2, SENSOR_PARAM_PAGE_3, SENSOR_PARAM_PAGE_2, SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_PAGE_1,
     SENSOR_PARAM_PAGE_3, &draw_sensor_param_page},
    {SENSOR_PARAM_PAGE_3, SENSOR_PARAM_PAGE_1, SENSOR_PARAM_PAGE_3, SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_PAGE_1,
     SENSOR_PARAM_PAGE_3, &draw_sensor_param_page},
    {OLED_PARAM_PAGE_1, OLED_PARAM_PAGE_2, OLED_PARAM_PAGE_1, OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_PAGE_1,
     OLED_PARAM_PAGE_2, &draw_oled_param_page},
    {OLED_PARAM_PAGE_2, OLED_PARAM_PAGE_1, OLED_PARAM_PAGE_2, OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_PAGE_1,
     OLED_PARAM_PAGE_2, &draw_oled_param_page},
    {VISION_PAGE, VISION_PAGE, VISION_PAGE, SET_PAGE_3, VISION_PAGE, VISION_PAGE, &draw_vision_page},
    {PROVISIONING_URL_PAGE, PROVISIONING_URL_PAGE, PROVISIONING_URL_PAGE, SET_PAGE_5, PROVISIONING_URL_PAGE, PROVISIONING_URL_PAGE, &draw_provisioning_url_page},

    /* 确认菜单 */
    {SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_PAGE_1,
     SENSOR_PARAM_CONFIRM_PAGE, SENSOR_PARAM_CONFIRM_PAGE, &draw_param_confirm_page},
    {OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_PAGE_1,
     OLED_PARAM_CONFIRM_PAGE, OLED_PARAM_CONFIRM_PAGE, &draw_param_confirm_page},
    {FATORY_CONFIRM_PAGE, FATORY_CONFIRM_PAGE, FATORY_CONFIRM_PAGE, SET_PAGE_4, FATORY_CONFIRM_PAGE,
     FATORY_CONFIRM_PAGE, &draw_fatory_confirm_page},
};

void oled_send_cmd(uint8_t o_command)
{
    uint8_t buf[2] = {0, o_command};

    int ret = i2c_write_dt(&oled_ssd1306_spec, buf, 2);
    if (ret != 0) {
        printk("I2C send cmd failed: %d\n", ret);
    }
}

void oled_send_data(uint8_t* o_data, uint16_t o_len)
{
    uint8_t buf[129] = {0};
    uint8_t i = 0;
    uint8_t total_len = o_len + 1;

    buf[0] = 0x40;
    for (i = 0; i < o_len; i++) {
        buf[i + 1] = o_data[i];
    }

    int ret = i2c_write_dt(&oled_ssd1306_spec, buf, total_len);
    if (ret != 0) {
        printk("I2C send data failed: %d\n", ret);
    }
}

void column_set(uint8_t column)
{
    oled_send_cmd(0x10 | (column >> 4));    // 设置列地址高位
    oled_send_cmd(0x00 | (column & 0x0f));  // 设置列地址低位
}

void page_set(uint8_t page) { oled_send_cmd(0xb0 + page); }

void draw_half_words(uint8_t page_offset, uint8_t column_offset, uint8_t* half_word, uint8_t half_word_num)
{
    uint8_t page, column_word;

    for (column_word = 0; column_word < half_word_num; column_word++) {
        for (page = 0; page < 2; page++) {
            page_set(page + page_offset);
            column_set(8 * column_word + column_offset * 8);
            uint8_t* ptr = half_word + (column_word * 16) + (page * 8);
            oled_send_data(ptr, 8);
        }
    }
    //         }
    //     }
    // }
    // draw_words(page_offset, column_offset, strw, word_len);

    // uint8_t strw[512] = {0};
    // uint16_t i, j;
    // uint8_t *p_str = (uint8_t *)str;
    // uint8_t k = 0;

    // 遍历传入的字符串
}

void draw_words(uint8_t page_offset, uint8_t column_offset, uint8_t* word, uint8_t word_num)
{
    uint8_t page, column_word;

    for (column_word = 0; column_word < word_num; column_word++) {
        for (page = 0; page < 2; page++) {
            page_set(page + page_offset);
            column_set(16 * column_word + column_offset * 8);
            uint8_t* ptr = word + (column_word * 32) + (page * 16);
            oled_send_data(ptr, 16);
        }
    }
}

void clear_oled(void)
{
    uint8_t page, column;
    uint8_t buf[MAX_COLUMN] = {0};

    for (page = 0; page < (MAX_PAGE / 8); page++) {
        page_set(page);
        column_set(0);
        for (column = 0; column < MAX_COLUMN; column++) {
            oled_send_data(buf, 1);
        }
    }
}

void clear_part(uint8_t page_offset, uint8_t column_offset, char* str, uint8_t word_len)
{
    uint8_t strw[512] = {0};
    uint16_t j, k;

    for (k = 0; k < word_len; k++) {
        for (j = 0; j < 32; j++) {
            strw[k * 32 + j] = 0;
        }
    }
    draw_words(page_offset, column_offset, strw, word_len);
}

void clear_half_part(uint8_t page_offset, uint8_t column_offset, char* str, uint8_t word_len)
{
    uint8_t strw[512] = {0};
    uint16_t j, k;

    for (k = 0; k < word_len; k++) {
        for (j = 0; j < 8; j++) {
            strw[k * 8 + j] = 0;
        }
    }
    draw_half_words(page_offset, column_offset, strw, word_len);
}

void draw_part(uint8_t page_offset, uint8_t column_offset, char* str, uint8_t word_len)
{
    // uint8_t strw[512] = {0};
    // uint16_t i, j, k;
    // uint16_t total_words = sizeof(gbk_code) / 2;

    // for (k = 0; k < word_len; k++) {
    //     for (i = 0; i < total_words; i++) {
    //         if (0 == memcmp(str + k * 2, gbk_code + i * 2, 2)) {
    //             for (j = 0; j < 32; j++) {
    //                 strw[k * 32 + j] = font_size_16x16[i * 32 + j];
    //             }
    //         }
    //     }
    // }
    // draw_words(page_offset, column_offset, strw, word_len);

    uint8_t strw[512] = {0};
    uint16_t j;
    uint8_t *p_str = (uint8_t *)str;
    uint8_t k = 0;

    // 遍历传入的字符串
    while (*p_str != '\0' && k < word_len) {
        uint8_t step = 0;
        
        // 判断当前字符占几个字节 (UTF-8 标准)
        if (*p_str < 0x80) step = 1;        // ASCII (不处理，或调用 half_draw)
        else if (*p_str < 0xE0) step = 2;   // 2字节字符
        else if (*p_str < 0xF0) step = 3;   // 3字节汉字 (最常用)
        else step = 4;

        // 在 utf8_code 库中寻找匹配
        uint8_t *p_table = (uint8_t *)utf8_code;
        uint16_t table_idx = 0;
        bool found = false;

        while (*p_table != '\0') {
            // 同样需要判断库里那个字占几字节
            uint8_t t_step = (*p_table < 0xE0) ? 2 : 3; 

            if (step == t_step && memcmp(p_str, p_table, step) == 0) {
                // 找到了，拷贝 16x16 点阵
                for (j = 0; j < 32; j++) {
                    strw[k * 32 + j] = font_size_16x16[table_idx * 32 + j];
                }
                found = true;
                break;
            }
            p_table += t_step;
            table_idx++;
        }

        p_str += step; // 移动到下一个字
        k++;           // 已处理的显示字数
    }
    draw_words(page_offset, column_offset, strw, k);
}

void draw_half_part(uint8_t page_offset, uint8_t column_offset, char* str, uint8_t word_len)
{
    uint8_t strw[512] = {0};
    uint16_t i, j, k;

    for (k = 0; k < word_len; k++) {
        for (i = 0; i < sizeof(utf8_size_8x16_code); i++) {
            if (0 == memcmp(str + k, utf8_size_8x16_code + i, 1)) {
                for (j = 0; j < 16; j++) {
                    strw[k * 16 + j] = font_size_8x16[i * 16 + j];
                }
            }
        }
    }
    draw_half_words(page_offset, column_offset, strw, word_len);
}

void val_to_str(uint16_t data, uint8_t* output_data)
{
    uint8_t value_buf1[4] = {0};
    uint16_t value_buf = 0;
    uint8_t j, i;

    value_buf = data;
    value_buf1[0] = value_buf / 10000;
    value_buf1[1] = value_buf % 1000 / 100;
    value_buf1[2] = value_buf % 100 / 10;
    value_buf1[3] = value_buf % 10;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 16; j++) {
            output_data[j + i * 16] = font_size_8x16[j + value_buf1[i] * 16];
        }
    }
}

void draw_welcom_page(void)
{
    uint8_t i;

    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;
    for (i = 0; i < sizeof(welcome_page_op) / sizeof(welcome_page_op[0]); i++) {
        welcome_page_op[i].dram_func(welcome_page_op[i].page_offset, welcome_page_op[i].column_offset,
                                     welcome_page_op[i].ptr_word, welcome_page_op[i].word_len);
    }
}
// uint8_t test_get_buf[30];
void draw_main_page(void)
{
    uint8_t i;
    // uint8_t sta, j;
    // uint8_t oled_num[64] = {0};
    // uint16_t rec_data;

    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }
    oled_par.pre_index = oled_par.current_index;

    for (i = 0; i < sizeof(main_page_op) / sizeof(main_page_op[0]); i++) {
        main_page_op[i].dram_func(main_page_op[i].page_offset, main_page_op[i].column_offset, main_page_op[i].ptr_word,
                                  main_page_op[i].word_len);
    }
    
    // sta = test_get_buf[2];
    // param_word_op[sta].dram_func(param_word_op[sta].page_offset, param_word_op[sta].column_offset,
    //                              param_word_op[sta].ptr_word, param_word_op[sta].word_len);

    // for (i = 3, j = 3; i < 6; i += 2, j += 3) {
    //     if ((uint16_t)(test_get_buf[i] | test_get_buf[i + 1] << 8) >= ADJUST_VALUE) {
    //         rec_data = (uint16_t)(test_get_buf[i] | test_get_buf[i + 1] << 8) - ADJUST_VALUE;
    //     } else {
    //         rec_data = 0;
    //     }
    //     val_to_str(rec_data, oled_num);
    //     draw_half_words(j, 9, oled_num, 4);
    // }
}

void draw_set_page(void)
{
    uint8_t i;
    uint8_t min_page, max_page;
    uint8_t len = 0;
    struct _draw_word_op* set_page_op = NULL;

    min_page = table_op[oled_par.current_index].min_page;
    max_page = table_op[oled_par.current_index].max_page;

    if (SET_PAGE_1 == min_page) {
        len = sizeof(set_page1_op) / sizeof(set_page1_op[0]);
        set_page_op = set_page1_op;
    } else if (SET_PAGE_4 == min_page) {
        len = sizeof(set_page2_op) / sizeof(set_page2_op[0]);
        set_page_op = set_page2_op;
    }

    if ((oled_par.pre_index < min_page) || (oled_par.pre_index > max_page)) {
        clear_oled();
    }
    oled_par.pre_index = oled_par.current_index;

    for (i = 0; i < len * 2 + 1; i += 3) {
        clear_half_part(i, 0, 0, 1);
    }

    draw_half_part((oled_par.current_index - min_page) * 3, 0, ">", 1);

    for (i = 0; i < len; i++) {
        set_page_op[i].dram_func(set_page_op[i].page_offset, set_page_op[i].column_offset, set_page_op[i].ptr_word,
                                 set_page_op[i].word_len);
    }
}

void draw_sensor_param_page(void)
{
    uint8_t i;
    uint8_t oled_num[64] = {0};

    if ((oled_par.pre_index < SENSOR_PARAM_PAGE_1) || (oled_par.pre_index > SENSOR_PARAM_PAGE_3)) {
        clear_oled();
    }
    oled_par.pre_index = oled_par.current_index;

    for (i = 0; i < sizeof(sensor_param_page_op) / sizeof(sensor_param_page_op[0]) * 2 + 1; i += 3) {
        if (((oled_par.current_index - SENSOR_PARAM_PAGE_1) * 3) != i) {
            clear_half_part(i, 0, 0, 1);
        }
    }

    draw_half_part((oled_par.current_index - SENSOR_PARAM_PAGE_1) * 3, 0, ">", 1);
    for (i = 0; i < sizeof(sensor_param_page_op) / sizeof(sensor_param_page_op[0]); i++) {
        sensor_param_page_op[i].dram_func(sensor_param_page_op[i].page_offset, sensor_param_page_op[i].column_offset,
                                          sensor_param_page_op[i].ptr_word, sensor_param_page_op[i].word_len);
    }

    if (oled_par.add_flag) {
        switch (oled_par.current_index) {
        case SENSOR_PARAM_PAGE_1:
            oled_par.distance++;
            if (200 <= oled_par.distance) {
                oled_par.distance = 0;
            }
            break;

        case SENSOR_PARAM_PAGE_2:
            oled_par.sensitivity++;
            if (100 <= oled_par.sensitivity) {
                oled_par.sensitivity = 0;
            }
            break;

        case SENSOR_PARAM_PAGE_3:
            oled_par.transmitted_power++;
            if (200 <= oled_par.transmitted_power) {
                oled_par.transmitted_power = 0;
            }
            break;

        default:
            break;
        }
        oled_par.add_flag = 0;
    }

    val_to_str(oled_par.distance, oled_num);
    draw_half_words(0, 10, oled_num, 4);
    val_to_str(oled_par.sensitivity, oled_num);
    draw_half_words(3, 10, oled_num, 4);
    val_to_str(oled_par.transmitted_power, oled_num);
    draw_half_words(6, 10, oled_num, 4);
}

void draw_oled_param_page(void)
{
    uint8_t i;
    uint8_t oled_num[64] = {0};

    if ((oled_par.pre_index < OLED_PARAM_PAGE_1) || (oled_par.pre_index > OLED_PARAM_PAGE_2)) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;

    for (i = 0; i < (sizeof(oled_param_page_op) / sizeof(oled_param_page_op[0])) * 2 + 1; i += 3) {
        if (((oled_par.current_index - OLED_PARAM_PAGE_1) * 3) != i) {
            clear_half_part(i, 0, 0, 1);
        }
    }

    draw_half_part((oled_par.current_index - OLED_PARAM_PAGE_1) * 3, 0, ">", 1);
    for (i = 0; i < sizeof(oled_param_page_op) / sizeof(oled_param_page_op[0]); i++) {
        oled_param_page_op[i].dram_func(oled_param_page_op[i].page_offset, oled_param_page_op[i].column_offset,
                                        oled_param_page_op[i].ptr_word, oled_param_page_op[i].word_len);
    }

    if (oled_par.add_flag) {
        switch (oled_par.current_index) {
        case OLED_PARAM_PAGE_1:
            oled_par.oled_light++;
            if (255 <= oled_par.oled_light) {
                oled_par.oled_light = 0;
            }
            break;

        case OLED_PARAM_PAGE_2:
            oled_par.oled_close_time++;
            if (999 <= oled_par.oled_close_time) {
                oled_par.oled_close_time = 0;
            }
            break;

        default:
            break;
        }
        oled_par.add_flag = 0;
    }

    val_to_str(oled_par.oled_light, oled_num);
    draw_half_words(0, 10, oled_num, 4);
    val_to_str(oled_par.oled_close_time, oled_num);
    draw_half_words(3, 10, oled_num, 4);
}

void draw_vision_page(void)
{
    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;
    // draw_part(3, 2, "\xB0\xE6\xB1\xBE\xBA\xC5", 3);  // 版本号
    draw_part(3, 2, "版本号", 3);
    draw_half_part(3, 8, ":V1.0", 5);
}

void draw_provisioning_url_page(void)
{
    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;
    draw_part(3, 2, "组网中", 3);  
    draw_half_part(3, 8, "...", 3);
}

void draw_param_confirm_page(void)
{
    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;
    // draw_part(3, 2, "\xC8\xB7\xB6\xA8\xB8\xFC\xB8\xC4\xB2\xCE\xCA\xFD", 6);  // 确定更改参数
    draw_part(3, 2, "确定更改参数", 6);
    draw_half_part(3, 14, "?", 1);
}

void draw_fatory_confirm_page(void)
{
    if (oled_par.pre_index != oled_par.current_index) {
        clear_oled();
    }

    oled_par.pre_index = oled_par.current_index;
    // draw_part(3, 2, "\xC8\xB7\xC8\xCF\xBB\xD6\xB8\xB4\xC4\xAC\xC8\xCF", 6);  // 确认恢复默认
    draw_part(3, 2, "确认恢复默认", 6);  // 确认恢复默认
    draw_half_part(3, 14, "?", 1);
}

void oled_init(void)
{
    uint8_t i;
    const uint8_t oled_init_cmd[25] = {
        0xAE,  // 关闭显示
        0xD5,  // 设置时钟分频因子,震荡频率
        0x80,  //[3:0],分频因子;[7:4],震荡频率
        0xA8,  // 设置驱动路数
        0X3F,  // 默认0X3F(1/64)
        0xD3,  // 设置显示偏移
        0X00,  // 默认为0
        0x40,  // 设置显示开始行 [5:0],行数.
        0x8D,  // 电荷泵设置
        0x14,  // bit2，开启/关闭
        0x20,  // 设置内存地址模式
        0x02,  //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
        0xA1,  // 段重定义设置,bit0:0,0->0;1,0->127;
        0xC8,  // 设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
        0xDA,  // 设置COM硬件引脚配置
        0x12,  //[5:4]配置
        0x81,  // 对比度设置
        0xEF,  // 1~255;默认0X7F (亮度设置,越大越亮)
        0xD9,  // 设置预充电周期
        0xf1,  //[3:0],PHASE 1;[7:4],PHASE 2;
        0xDB,  // 设置VCOMH 电压倍率
        0x30,  //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;
        0xA4,  // 全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
        0xA6,  // 设置显示方式;bit0:1,反相显示;0,正常显示
        0xAF,  // 开启显示
    };

    for (i = 0; i < 25; i++) {
        oled_send_cmd(oled_init_cmd[i]);
    }

    clear_oled();
}

void oled_thread(void)
{
    if (!i2c_is_ready_dt(&oled_ssd1306_spec)) {
        printk("hrbo not ready\n");
        return;
    }

    oled_init();

    for (;;) {
        k_mutex_lock(&i2c_mutex, K_FOREVER);
        if (oled_par.key_on) {
            oled_par.key_on = 0;
            table_op[oled_par.current_index].table_operation();
        }
        k_mutex_unlock(&i2c_mutex);
        k_msleep(50);
    }
}
K_THREAD_DEFINE(oled_thread_id, 2048, oled_thread, NULL, NULL, NULL, 3, 0, 0);
