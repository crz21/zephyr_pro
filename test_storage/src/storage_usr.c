#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>

static struct nvs_fs fs;
#define FS_SECTOR_COUNT (3)
#define CONFIG_ID (1)

void save_parameter(uint8_t* buf, uint16_t len) { nvs_write(&fs, CONFIG_ID, buf, len); }

void load_parameter(uint8_t* buf, uint16_t len) { ssize_t rc = nvs_read(&fs, CONFIG_ID, buf, len); }

void init_storage(void)
{
    int rc;
    struct flash_pages_info info;

    // 获取 storage 分区信息
    fs.flash_device = FLASH_AREA_DEVICE(storage_partition);
    fs.offset = FLASH_AREA_OFFSET(storage_partition);

    rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
    fs.sector_size = info.size;
    fs.sector_count = FS_SECTOR_COUNT;  // 根据分区大小设置扇区数量

    rc = nvs_mount(&fs);
}