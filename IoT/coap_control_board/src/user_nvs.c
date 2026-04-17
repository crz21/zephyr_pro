
#include "user_nvs.h"

#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>

struct __packed _white_list_par {
    uint8_t mac[6];
    char url[10];
    uint8_t light_lev;
};

struct _param {
    uint8_t first_flag_par[2];
    uint8_t oled_config_par[2];
    struct _white_list_par white_list_par[10];
};
struct _param g_par;
static const struct _param origin_par = {
    {0xaa, 0x55},  //
    {1, 1},        //
    {
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128},  //
        {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 128}   //
    }  //
};

static struct nvs_fs fs;

static void save_parameter(uint8_t id, uint8_t* buf, uint16_t len)
{
    ssize_t rc = 0;
    rc = nvs_write(&fs, id, buf, len);
    if (rc < 0) {
        printk("nvs_write fail : %d\n", rc);
    }
}

static void load_parameter(uint8_t id, uint8_t* buf, uint16_t len)
{
    ssize_t rc = 0;
    rc = nvs_read(&fs, id, buf, len);
    if (rc < 0) {
        printk("nvs_read fail : %d\n", rc);
    }
}

void read_oled_light(void)
{
    uint8_t oled_buf[2] = {0};
    struct __packed _white_list_par buf_list[10] = {0};

    load_parameter(OLED_CONFIG_ID, oled_buf, 2);
    load_parameter(NVS_ID_FULL_WHITELIST, (uint8_t*)buf_list, sizeof(buf_list));
    printk("OLED_CONFIG_ID:0x%x, 0x%x \n\r NVS_ID_FULL_WHITELIST1:%x \n\r NVS_ID_FULL_WHITELIST10:%x \n\r", oled_buf[0],
           oled_buf[1], buf_list[0].light_lev, buf_list[9].light_lev);
}

// void init_list_storage(void)
// {
//     struct flash_pages_info info;

//     fs.flash_device = FLASH_AREA_DEVICE(storage_partition);
//     if (!device_is_ready(fs.flash_device)) {
//         printk("Flash device %s is not ready\n", fs.flash_device->name);
//     }
//     fs.offset = FLASH_AREA_OFFSET(storage_partition);
//     int rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
//     if (rc) {
//         printk("Unable to get page info, rc=%d\n", rc);
//     }
//     fs.sector_size = info.size;
//     fs.sector_count = FLASH_AREA_SIZE(storage_partition) / fs.sector_size;

//     rc = nvs_mount(&fs);
//     if (rc) {
// 		printk("Flash Init failed, rc=%d\n", rc);
// 	}
// }
#define USER_STORAGE_NODE DT_NODELABEL(user_storage)
int init_list_storage(void)
{
//     struct flash_pages_info info;
//     int rc;

//     // 1. 获取 flash 设备
//     // 注意：这里要获取分区所属的 flash 控制器设备
//     fs.flash_device = DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_STORAGE_NODE));

//     if (!device_is_ready(fs.flash_device)) {
//         return -ENODEV;
//     }

//     // 2. 使用 DT API 获取偏移和大小 (这将绕过 PM 宏的干扰)
//     fs.offset = DT_REG_ADDR(USER_STORAGE_NODE);
//     uint32_t storage_size = DT_REG_SIZE(USER_STORAGE_NODE);

//     // 3. 获取页面信息
//     rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
//     if (rc) {
//         return rc;
//     }

//     fs.sector_size = info.size;
//     fs.sector_count = storage_size / fs.sector_size;
// printk("User NVS start address: 0x%lx\n", fs.offset);
//     // 4. 挂载 NVS
//     rc = nvs_mount(&fs);
//     if (rc) {
//         return rc;
//     }

//     return 0;

    struct flash_pages_info info;
    int rc;

    fs.flash_device = DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(USER_STORAGE_NODE));

    if (!device_is_ready(fs.flash_device)) {
        printk("Flash device not ready\n");
        return -ENODEV;
    }


    fs.offset = DT_REG_ADDR(USER_STORAGE_NODE);
    uint32_t storage_size = DT_REG_SIZE(USER_STORAGE_NODE);

    rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    if (rc) {
        printk("Page info fail: %d\n", rc);
        return rc;
    }

    fs.sector_size = info.size;
    fs.sector_count = storage_size / fs.sector_size;

    printk("NVS Backup Plan -> Addr: 0x%lx, Size: %d\n", (long)fs.offset, storage_size);

    rc = nvs_mount(&fs);
    if (rc) {
        printk("NVS Mount failed: %d\n", rc);
    }

    return rc;
}

void poweron_read_param(void)
{
    uint8_t i = 0;

    load_parameter(FIRST_USAGE_FLAG_ID, g_par.first_flag_par, 2);

    if ((g_par.first_flag_par[0] != 0xaa) || (g_par.first_flag_par[1] != 0x55)) {
        for (i = 0; i < 2; i++) {
            g_par.first_flag_par[i] = origin_par.first_flag_par[i];
            g_par.oled_config_par[i] = origin_par.oled_config_par[i];
        }
        printk("test1:%x %x\n", g_par.first_flag_par[0], g_par.first_flag_par[1]);
        save_parameter(FIRST_USAGE_FLAG_ID, g_par.first_flag_par, 2);
        save_parameter(OLED_CONFIG_ID, g_par.oled_config_par, 2);

        for (i = 0; i < 10; i++) {
            g_par.white_list_par[i] = origin_par.white_list_par[i];
        }
        save_parameter(NVS_ID_FULL_WHITELIST, (uint8_t*)g_par.white_list_par, sizeof(g_par.white_list_par));
    } else {
        // for (i = 0; i < DEVICE_PARAM_NUM; i++) {
        //     if ((param_arr[i] > g_param[i].max_par) && (param_arr[i] < g_param[i].min_par)) {
        //         param_arr[i] = g_param[i].origin_par;
        //         save_parameter(param_arr + i, 1);
        //     }
        // }
    }
}
