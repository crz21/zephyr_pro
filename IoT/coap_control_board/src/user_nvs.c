#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>

#define MAX_DEV_COUNT 100
#define NVS_ID_WHITELIST 1
#define NVS_PARTITION_ID FIXED_PARTITION_ID(storage_partition)
#define LIST_STORAGE_ID 1
#define MAX_LIST_ENTRIES 100
#define FS_SECTOR_COUNT (3)
#define DEVICE_PARAM_NUM (10)

enum { FIRST_USAGE };
struct _param {
    uint8_t origin_par;
    uint8_t min_par;
    uint8_t max_par;
};
static struct _param g_param[DEVICE_PARAM_NUM] = {
    {0xaa, 0xaa, 0xaa},  //
    {0x55, 0x55, 0x55},  //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
    {0, 0, 100},         //
};
#define CONFIG_ID (1)
static struct nvs_fs fs;

static void save_parameter(uint8_t* buf, uint16_t len) { nvs_write(&fs, CONFIG_ID, buf, len); }

static void load_parameter(uint8_t* buf, uint16_t len) { ssize_t rc = nvs_read(&fs, CONFIG_ID, buf, len); }

// 初始化 NVS 存储
int init_list_storage(void)
{
    struct flash_pages_info info;
    // fs.flash_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
    // fs.offset = DT_REG_ADDR(DT_NODE_BY_FIXED_PARTITION_ID(NVS_PARTITION_ID));
    fs.flash_device = FLASH_AREA_DEVICE(storage_partition);
    fs.offset = FLASH_AREA_OFFSET(storage_partition);
    flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    fs.sector_size = info.size;
    fs.sector_count = FS_SECTOR_COUNT;  // 12kb

    return nvs_mount(&fs);
}

uint8_t read_ip_addr(uint8_t* ip_buf, uint8_t ip_len)
{
    load_parameter(ip_buf, (uint8_t)ip_len);

    return 0;
}


uint8_t write_ip_addr(uint8_t* ip_buf, uint8_t ip_len)
{
    save_parameter(ip_buf, (uint8_t)ip_len);

    return 0;
}

uint8_t read_first_time_usage_flag(void)
{
    uint8_t flag;

    load_parameter(&flag, 1);
    return flag;
}

void save_first_time_usage_flag(uint8_t flag) { save_parameter(&flag, 1); }

void read_param(uint8_t* param_arr)
{
    uint8_t i = 0;

    load_parameter(param_arr, DEVICE_PARAM_NUM);

    if ((param_arr[0] != 0xaa) || (param_arr[1] != 0x55)) {
        for (i = 0; i < DEVICE_PARAM_NUM; i++) {
            param_arr[i] = g_param[i].origin_par;
        }
        save_parameter(param_arr, DEVICE_PARAM_NUM);
    } else {
        for (i = 0; i < DEVICE_PARAM_NUM; i++) {
            if ((param_arr[i] > g_param[i].max_par) && (param_arr[i] < g_param[i].min_par)) {
                param_arr[i] = g_param[i].origin_par;
                save_parameter(param_arr + i, 1);
            }
        }
    }
}
