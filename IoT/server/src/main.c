#include "main.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/logging/log.h>
#include <zephyr/stats/stats.h>

#ifdef CONFIG_MCUMGR_GRP_FS
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#endif

#ifdef CONFIG_MCUMGR_GRP_STAT
#include <zephyr/mgmt/mcumgr/grp/stat_mgmt/stat_mgmt.h>
#endif

LOG_MODULE_REGISTER(smp_sample);

struct k_mutex i2c_mutex;

#ifdef CONFIG_MCUMGR_GRP_FS
#define STORAGE_PARTITION_LABEL storage_partition
#define STORAGE_PARTITION_ID FIXED_PARTITION_ID(STORAGE_PARTITION_LABEL)
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(cstorage);
static struct fs_mount_t littlefs_mnt = {
    .type = FS_LITTLEFS, .fs_data = &cstorage, .storage_dev = (void*)STORAGE_PARTITION_ID, .mnt_point = "/lfs1"};
#endif

#ifdef CONFIG_MCUMGR_GRP_STAT
/* Define an example stats group; approximates seconds since boot. */
STATS_SECT_START(smp_svr_stats)
STATS_SECT_ENTRY(ticks)
STATS_SECT_END;

/* Assign a name to the `ticks` stat. */
STATS_NAME_START(smp_svr_stats)
STATS_NAME(smp_svr_stats, ticks)
STATS_NAME_END(smp_svr_stats);

/* Define an instance of the stats group. */
STATS_SECT_DECL(smp_svr_stats) smp_svr_stats;
#endif

int main(void)
{
    printk("Address of sample %p\n", (void*)__rom_region_start);

#ifdef CONFIG_MCUMGR_GRP_STAT
    int rc = STATS_INIT_AND_REG(smp_svr_stats, STATS_SIZE_32, "smp_svr_stats");
    if (rc < 0) {
        LOG_ERR("Error initializing stats system [%d]", rc);
    }
#endif

#ifdef CONFIG_MCUMGR_GRP_FS
    rc = fs_mount(&littlefs_mnt);
    if (rc < 0) {
        LOG_ERR("Error mounting littlefs [%d]", rc);
    }
#endif

    k_mutex_init(&i2c_mutex);

    while (1) {
        k_sleep(K_MSEC(10));
#ifdef CONFIG_MCUMGR_GRP_STAT
        STATS_INC(smp_svr_stats, ticks);
#endif
    }
    return 0;
}
