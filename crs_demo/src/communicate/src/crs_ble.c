#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/crs.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/mgmt/mcumgr/transport/smp_bt.h>  //dfu

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_bt_sample);

#include "main.h"

static bool crf_ntf_enabled;
static struct k_work advertise_work;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_CRS_VAL), BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
                  BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, SMP_BT_SVC_UUID_VAL),

};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

enum {
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_BITS,
};

static ATOMIC_DEFINE(state, STATE_BITS);

static void advertise(struct k_work* work)
{
    int rc;

    rc = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (rc) {
        LOG_ERR("Advertising failed to start (rc %d)", rc);
        return;
    }

    LOG_INF("Advertising successfully started");
}

int set_ble_tx_power(uint16_t handle, int8_t dbm)
{
    struct bt_hci_cp_vs_write_tx_power_level* cp;
    struct bt_hci_rp_vs_write_tx_power_level* rp;
    struct net_buf *buf, *rsp = NULL;
    int err;

    buf = bt_hci_cmd_alloc(K_FOREVER);
    if (!buf) {
        return -ENOBUFS;
    }
    cp = net_buf_add(buf, sizeof(*cp));
    cp->handle_type = BT_HCI_VS_LL_HANDLE_TYPE_CONN;  
    cp->handle = handle;
    cp->tx_power_level = dbm;  

    err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf, &rsp);
    if (err) {
        printk("Set TX Power failed (err %d)\n", err);
        return err;
    }

    rp = (void*)rsp->data;
    printk("TX Power set to %d dBm (Actual: %d)\n", dbm, rp->selected_tx_power);

    net_buf_unref(rsp);
    return 0;
}

void on_le_data_len_updated(struct bt_conn* conn, struct bt_conn_le_data_len_info* info)
{
    printk("Step 1 Complete: DLE is now %u. Now starting Step 2: PHY Update...\n", info->tx_max_len);

    uint16_t handle;
    int err = bt_hci_get_conn_handle(conn, &handle);

    if (!err) {
        set_ble_tx_power(handle, 8);
    }
}

void on_le_phy_updated(struct bt_conn* conn, struct bt_conn_le_phy_info* param)
{
    printk("LE PHY Updated:Tx 0x%x, Rx 0x%x\n", param->tx_phy, param->rx_phy);

    struct bt_conn_le_data_len_param data_len_param = {
        .tx_max_len = 251,
        .tx_max_time = 17040,
    };
    bt_conn_le_data_len_update(conn, &data_len_param);
}

static void connected(struct bt_conn* conn, uint8_t err)
{
    if (err) {
        printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
        k_work_submit(&advertise_work);
    } else {
        printk("Connected\n");
        (void)atomic_set_bit(state, STATE_CONNECTED);
    }

    const struct bt_conn_le_phy_param phy_param = {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };
    bt_conn_le_phy_update(conn, &phy_param);

    // struct bt_conn_le_data_len_param data_len_param = {
    //     .tx_max_len = 251,
    //     .tx_max_time = 17040,
    // };
    // bt_conn_le_data_len_update(conn, &data_len_param);
}

static void disconnected(struct bt_conn* conn, uint8_t reason)
{
    printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));

    (void)atomic_set_bit(state, STATE_DISCONNECTED);
}

static void on_conn_recycled(void) { k_work_submit(&advertise_work); }

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
    .le_data_len_updated = on_le_data_len_updated,
    .le_phy_updated = on_le_phy_updated,
    .recycled = on_conn_recycled,
};

static void disconnect_option(void)
{
    int err;

    if (atomic_test_and_clear_bit(state, STATE_DISCONNECTED)) {
        printk("Starting Legacy Advertising (connectable and scannable)\n");
        err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
        if (err) {
            printk("Advertising failed to start (err %d)\n", err);
            return;
        }
    }
}

static void crs_ntf_changed(bool enabled)
{
    crf_ntf_enabled = enabled;

    printk("CRS notification status changed: %s\n", enabled ? "enabled" : "disabled");
}

static struct bt_crs_cb crs_cb = {
    .ntf_changed = crs_ntf_changed,
};

// static void auth_cancel(struct bt_conn* conn)
// {
//     char addr[BT_ADDR_LE_STR_LEN];

//     bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

//     printk("Pairing cancelled: %s\n", addr);
// }

// static struct bt_conn_auth_cb auth_cb_display = {
//     .cancel = auth_cancel,
// };

void crs_notify(uint8_t* data, uint16_t len)
{
    if (crf_ntf_enabled) {
        bt_crs_notify(data, len);
    }
}

static void bt_ready(int err)
{
    if (err != 0) {
        LOG_ERR("Bluetooth failed to initialise: %d", err);
    } else {
        k_work_submit(&advertise_work);
    }
}

static void crs_ble_init(void)
{
    int err;

    k_work_init(&advertise_work, advertise);
    err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    // bt_conn_auth_cb_register(&auth_cb_display);
    bt_crs_cb_register(&crs_cb);
}

void ble_thread(void)
{
    crs_ble_init();

    for (;;) {
        disconnect_option();
        k_msleep(1);
    }
}

K_THREAD_DEFINE(ble_thread_id, 1024, ble_thread, NULL, NULL, NULL, BLE_PRIORITY, 0, 0);
