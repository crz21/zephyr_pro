// #include <zephyr/drivers/flash.h>
// #include <zephyr/net/net_ip.h>
// #include <zephyr/storage/stream/nvs.h>

// #define NVS_PARTITION_ID FIXED_PARTITION_ID(storage_partition)
// #define LIST_STORAGE_ID 1
// #define MAX_LIST_ENTRIES 100

// // 定义名单条目
// struct device_entry {
//     struct in6_addr addr;  // 存储 IPv6 地址
//     bool is_whitelisted;   // true 为白名单，false 为黑名单
//     bool is_active;        // 槽位是否在使用
// };

// static struct nvs_fs fs;
// static struct device_entry device_list[MAX_LIST_ENTRIES];

// // 初始化 NVS 存储
// int init_list_storage(void)
// {
//     struct flash_pages_info info;
//     fs.flash_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
//     fs.offset = DT_REG_ADDR(DT_NODE_BY_FIXED_PARTITION_ID(NVS_PARTITION_ID));
//     flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
//     fs.sector_size = info.size;
//     fs.sector_count = 3;

//     return nvs_mount(&fs);
// }

// /**
//  * @brief 校验地址是否允许操作
//  * @return 1: 白名单通过, -1: 黑名单拦截, 0: 未知设备
//  */
// int check_address_auth(const struct in6_addr* src_addr)
// {
//     for (int i = 0; i < MAX_LIST_ENTRIES; i++) {
//         if (device_list[i].is_active && net_ipv6_addr_cmp(&device_list[i].addr, src_addr)) {
//             return device_list[i].is_whitelisted ? 1 : -1;
//         }
//     }
//     return 0;  // 未定义设备
// }

// void send_light_control(struct in6_addr* target_ip, uint8_t action)
// {
//     // 权限检查
//     int auth_status = check_address_auth(target_ip);

//     if (auth_status == -1) {
//         LOG_ERR("Target is in Blacklist! Access Denied.");
//         return;
//     } else if (auth_status == 0) {
//         LOG_WRN("Unknown device, proceeding with caution...");
//         // 这里可以选择是否允许控制未知设备
//     }

//     // 正常的 CoAP 构建与发送逻辑
//     struct coap_packet request;
//     // ... coap_packet_init ...
//     // ... sendto ...
//     LOG_INF("Command sent to whitelisted device.");
// }

// int add_to_list(struct in6_addr* new_addr, bool whitelist)
// {
//     for (int i = 0; i < MAX_LIST_ENTRIES; i++) {
//         if (!device_list[i].is_active) {
//             memcpy(&device_list[i].addr, new_addr, sizeof(struct in6_addr));
//             device_list[i].is_whitelisted = whitelist;
//             device_list[i].is_active = true;

//             // 持久化到 Flash
//             nvs_write(&fs, LIST_STORAGE_ID, &device_list, sizeof(device_list));
//             return 0;
//         }
//     }
//     return -ENOMEM;  // 名单已满
// }

// void get_mac_from_ip(struct in6_addr* ip_addr)
// {
//     struct otInstance* instance = openthread_get_default_instance();
//     otNeighborInfo info;
//     otNeighborInfoIterator iter = OT_NEIGHBOR_INFO_ITERATOR_INIT;

//     // 遍历邻居表
//     while (otThreadGetNextNeighborInfo(instance, &iter, &info) == OT_ERROR_NONE) {
//         // 将邻居的 IPv6 地址与目标进行比对（这里需要额外逻辑匹配地址）
//         // 如果匹配成功，info.mExtAddress 就是你要的 MAC 地址
//         LOG_INF("Neighbor MAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", info.mExtAddress.m8[0],
//         info.mExtAddress.m8[1],
//                 info.mExtAddress.m8[7]);
//     }
// }

// void local_my_mac(void)
// {
//     struct otInstance* instance = openthread_get_default_instance();
//     const otExtAddress* extAddr = otLinkGetExtendedAddress(instance);
//     // 这里的 extAddr 就是本机的 8 字节 MAC
// }

// #include <openthread/link.h>
// #include <openthread/thread.h>
// #include <zephyr/net/openthread.h>

// void send_announcement_to_panel(void)
// {
//     struct otInstance* instance = openthread_get_default_instance();

//     // 1. 获取本地 8 字节 Extended MAC 地址
//     const otExtAddress* extAddr = otLinkGetExtendedAddress(instance);

//     // 2. 构造 Payload (简单二进制格式或 JSON)
//     // 格式：[MAC(8字节)] + [DeviceType(1字节)]
//     uint8_t payload[9];
//     memcpy(payload, extAddr->m8, 8);
//     payload[8] = 0x01;  // 假设 0x01 代表照明设备

//     // 3. 发送 CoAP POST 到面板的单播地址或多播地址
//     // 如果不知道面板 IP，可以发到 ff03::1 (Mesh-local all nodes)
//     struct coap_packet request;
//     // ... 标准 CoAP 初始化与发送 ...
// }

// static int register_res_handler(struct coap_resource* resource, struct coap_packet* request, struct sockaddr* from,
//                                 socklen_t addrlen)
// {
//     uint8_t payload[16];
//     uint16_t payload_len;
//     const uint8_t* data;

//     data = coap_packet_get_payload(request, &payload_len);

//     if (payload_len >= 8) {
//         // 1. 获取发送者的 IP 地址 (从底层网络信息获取)
//         char ip_str[INET6_ADDRSTRLEN];
//         net_addr_ntop(AF_INET6, &net_sin6(from)->sin6_addr, ip_str, sizeof(ip_str));

//         // 2. 提取 MAC 地址 (从 Payload 获取)
//         LOG_INF("New Device Registered:");
//         LOG_INF("  IP: %s", ip_str);
//         LOG_INF("  MAC: %02x%02x%02x%02x%02x%02x%02x%02x", data[0], data[1], data[2], data[3], data[4], data[5],
//                 data[6], data[7]);

//         // 3. 将其存入白名单数组或 NVS 闪存
//         add_to_list(&net_sin6(from)->sin6_addr, data);
//     }

//     return coap_reply_ack(resource, request, from, addrlen);
// }

#include <zephyr/net/net_ip.h>
#include <zephyr/storage/stream/nvs.h>

#define MAX_DEV_COUNT 100
#define NVS_ID_WHITELIST 1
#define NVS_PARTITION_ID FIXED_PARTITION_ID(storage_partition)
#define LIST_STORAGE_ID 1
#define MAX_LIST_ENTRIES 100
bool search_addr_timer_flag = 1;
struct device_entry {
    uint8_t mac[8];        // 固定的 MAC 地址 (Extended Address)
    struct in6_addr ipv6;  // 动态的 IPv6 地址
    bool is_active;        // 是否在名单中
};

// 内存中的副本，用于快速查询
static struct device_entry whitelist[MAX_DEV_COUNT];
static struct nvs_fs fs;

// 初始化 NVS 存储
// int init_list_storage(void)
// {
//     struct flash_pages_info info;
//     fs.flash_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_flash_controller));
//     fs.offset = DT_REG_ADDR(DT_NODE_BY_FIXED_PARTITION_ID(NVS_PARTITION_ID));
//     flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
//     fs.sector_size = info.size;
//     fs.sector_count = 3;

//     return nvs_mount(&fs);
// }

/**
 * @brief 更新或添加设备到白名单
 * @param mac 设备的 8 字节 MAC
 * @param new_ip 设备的当前 IPv6 地址
 */
void update_whitelist(const uint8_t* mac, const struct in6_addr* new_ip)
{
    int empty_slot = -1;

    for (int i = 0; i < MAX_DEV_COUNT; i++) {
        // 1. 匹配 MAC 地址
        if (whitelist[i].is_active && memcmp(whitelist[i].mac, mac, 8) == 0) {
            // 如果 IP 变了，更新它
            if (!net_ipv6_addr_cmp(&whitelist[i].ipv6, new_ip)) {
                memcpy(&whitelist[i].ipv6, new_ip, sizeof(struct in6_addr));
                nvs_write(&fs, NVS_ID_WHITELIST, &whitelist, sizeof(whitelist));
                printk("Updated IP for MAC %02x...\n", mac[0]);
            }
            return;
        }
        if (!whitelist[i].is_active && empty_slot == -1) {
            empty_slot = i;
        }
    }

    // 2. 如果是新设备且有空位，则添加
    if (empty_slot != -1) {
        whitelist[empty_slot].is_active = true;
        memcpy(whitelist[empty_slot].mac, mac, 8);
        memcpy(whitelist[empty_slot].ipv6, new_ip, sizeof(struct in6_addr));

        nvs_write(&fs, NVS_ID_WHITELIST, &whitelist, sizeof(whitelist));
        printk("Added new device to whitelist.\n");
    }
}
static int register_handler(struct coap_resource* res, struct coap_packet* pkt, struct sockaddr* from,
                            socklen_t addrlen)
{
    uint16_t len;
    const uint8_t* payload = coap_packet_get_payload(pkt, &len);

    // 假设 Payload 前 8 字节是 MAC 地址
    if (len >= 8 && from->sa_family == AF_INET6) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)from;

        // 调用更新逻辑
        update_whitelist(payload, &addr6->sin6_addr);
    }

    return coap_reply_ack(res, pkt, from, addrlen);
}

bool is_authorized(const struct in6_addr* target_ip)
{
    for (int i = 0; i < MAX_DEV_COUNT; i++) {
        if (whitelist[i].is_active && net_ipv6_addr_cmp(&whitelist[i].ipv6, target_ip)) {
            return true;
        }
    }
    return false;
}

static int on_provisioning_reply(const struct coap_packet* response, struct coap_reply* reply,
                                 const struct sockaddr* from)
{
    int ret = 0;
    const uint8_t* payload;
    uint16_t payload_size = 0u;

    ARG_UNUSED(reply);
    ARG_UNUSED(from);

    payload = coap_packet_get_payload(response, &payload_size);

    if (payload == NULL || payload_size != sizeof(unique_local_addr.sin6_addr)) {
        LOG_ERR("Received data is invalid");
        ret = -EINVAL;
        goto exit;
    }

    memcpy(&unique_local_addr.sin6_addr, payload, payload_size);

    if (!zsock_inet_ntop(AF_INET6, payload, unique_local_addr_str, INET6_ADDRSTRLEN)) {
        LOG_ERR("Received data is not IPv6 address: %d", errno);
        ret = -errno;
        goto exit;
    }

    LOG_INF("Received peer address: %s", unique_local_addr_str);

exit:
    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        poll_period_restore();
    }

    return ret;
}

static void send_provisioning_request(struct k_work* item)
{
    ARG_UNUSED(item);

    if (IS_ENABLED(CONFIG_OPENTHREAD_MTD_SED)) {
        /* decrease the polling period for higher responsiveness */
        poll_period_response_set();
    }

    LOG_INF("Send 'provisioning' request");

    coap_send_request(COAP_METHOD_GET, (const struct sockaddr*)&multicast_local_addr, provisioning_option, NULL, 0u,
                      on_provisioning_reply);
}

void search_addr_handle(void)
{
    k_timer_stop(&search_addr_timer);
    k_timer_stop(&send_provisioning_request_timer);
}

void list_thread(void)
{
    k_timer_start(&search_addr_timer, K_MSEC(180000), K_NO_WAIT);
    k_timer_start(&send_provisioning_request_timer, K_MSEC(1000), K_NO_WAIT);
    for (;;) {
        if (setting_flag == 1) {
            if (setting_ok_flag == 1) {
            }
        }
    }
}
K_TIMER_DEFINE(search_addr_timer, search_addr_handle, NULL);
K_TIMER_DEFINE(send_provisioning_request_timer, send_provisioning_request, NULL);
K_THREAD_DEFINE(list_thread_id, 1024, list_thread, NULL, NULL, NULL, LIST_PRIORITY);
