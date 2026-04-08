#include <stdint.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#define OLED_SSD1306_PRIORITY (5)

#define OLED_SSD1306_NODE DT_NODELABEL(oled_ssd1306)
const struct i2c_dt_spec oled_ssd1306_spec = I2C_DT_SPEC_GET(OLED_SSD1306_NODE);

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

    oled_clear();
}

void oled_thread(void)
{
    if (!i2c_is_ready_dt(&oled_ssd1306_spec)) {
        printk("hrbo not ready\n");
        return;
    }

    oled_init();

    for (;;) {
        table_op[current_index].table_operation();
    }
}
K_THREAD_DEFINE(oled_thread_id, 1024, oled_thread, NULL, NULL, NULL, OLED_SSD1306_PRIORITY, 0, 0);