#include <zephyr/storage/stream/nvs.h>

#define MAX_DEV_COUNT 100
#define NVS_ID_WHITELIST 1
#define NVS_PARTITION_ID FIXED_PARTITION_ID(storage_partition)
#define LIST_STORAGE_ID 1
#define MAX_LIST_ENTRIES 100

static struct nvs_fs fs;

// 初始化 NVS 存储
int init_list_storage(void)
{
    struct flash_pages_info info;
    fs.flash_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
    fs.offset = DT_REG_ADDR(DT_NODE_BY_FIXED_PARTITION_ID(NVS_PARTITION_ID));
    flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    fs.sector_size = info.size;
    fs.sector_count = 3;  // 12kb

    return nvs_mount(&fs);
}

int write_flash_section(uint8_t* data_buf, uint16_t len)
{
    nvs_write(&fs, LIST_STORAGE_ID, data_buf, len);
    return 0;
}

int read_flash_section(uint8_t* data_buf, uint16_t len)
{
    nvs_read(&fs, LIST_STORAGE_ID, data_buf, len);
    return 0;
}
