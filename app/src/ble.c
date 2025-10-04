
#include <zephyr.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>

#include "app_uuids.h"

/* UUIDs */
static struct bt_uuid_128 app_service_uuid = BT_UUID_INIT_128(
    0x78,0x56,0x34,0x12, 0x34,0x12, 0x78,0x56, 0x9a,0xbc, 0xde,0xf0,0x12,0x34,0x56,0x78
);

/* Local value storage */
static uint32_t last_voltage_mv = 0;
static uint32_t sample_interval_ms = 0;

/* CCC state */
static uint8_t ccc_cfg[BT_GATT_CCC_MAX];

/* Connection */
static struct bt_conn *current_conn;

/* Forward to allow adc sampler to call into BLE notify */
void ble_notify_voltage(uint32_t mv)
{
    last_voltage_mv = mv;
    if (current_conn && bt_gatt_is_subscribed(current_conn, &voltage_attr, BT_GATT_CCC_NOTIFY)) {
        uint8_t buf[4];
        sys_put_le32(mv, buf);
        bt_gatt_notify(current_conn, &voltage_attr, buf, sizeof(buf));
    }
}

/* Read handlers */
static ssize_t read_voltage(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                void *buf, uint16_t len, uint16_t offset)
{
    uint8_t val[4];
    sys_put_le32(last_voltage_mv, val);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, val, sizeof(val));
}

static ssize_t read_interval(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                void *buf, uint16_t len, uint16_t offset)
{
    uint8_t val[4];
    sys_put_le32(sample_interval_ms, val);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, val, sizeof(val));
}

/* CCC changed callback */
static void ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    ARG_UNUSED(attr);
    LOG_INF("CCC changed: %u", value);
}

/* GATT attributes (service + characteristics) */
static struct bt_gatt_attr voltage_attr;

/* We'll build the GATT service dynamically in bt_ready callback */
static struct bt_gatt_attr attrs[8];

static ssize_t bt_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
    } else {
        LOG_INF("Connected");
        current_conn = bt_conn_ref(conn);
    }
    return 0;
}

static void bt_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
}

static struct bt_conn_cb conn_callbacks = {
    .connected = bt_connected,
    .disconnected = bt_disconnected,
};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, "FW-CHALLENGE", 12),
};

static void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed: %d", err);
        return;
    }
    LOG_INF("Bluetooth initialized");
    bt_conn_cb_register(&conn_callbacks);

    /* create & register GATT service */
    /* For clarity we use the BT_GATT_SERVICE_DEFINE macro */

    BT_GATT_SERVICE_DEFINE(app_svc,
        BT_GATT_PRIMARY_SERVICE(&app_service_uuid),
        BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_128(
            0x78,0x56,0x34,0x12, 0x34,0x12, 0x78,0x56, 0x9a,0xbc, 0xde,0xf0,0x12,0x34,0x56,0x78),
            BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
            BT_GATT_PERM_READ,
            read_voltage, NULL, NULL),
        BT_GATT_CCC(ccc_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
        BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_128(
            0x78,0x56,0x34,0x12, 0x34,0x12, 0x78,0x56, 0x9a,0xbc, 0xde,0xf0,0x12,0x34,0x56,0x79),
            BT_GATT_CHRC_READ,
            BT_GATT_PERM_READ,
            read_interval, NULL, NULL),
    );
    /* advertising */
    bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
}

int ble_init(void)
{
    int rc = bt_enable(bt_ready);
    if (rc) {
        LOG_ERR("bt_enable returned %d", rc);
        return rc;
    }
    /* sample interval: read from app config */
#if defined(CONFIG_APP_SAMPLE_INTERVAL_MS) && (CONFIG_APP_SAMPLE_INTERVAL_MS > 0)
    sample_interval_ms = CONFIG_APP_SAMPLE_INTERVAL_MS;
#elif DT_NODE_HAS_PROP(DT_PATH(app), sample_interval_ms)
    sample_interval_ms = DT_PROP(DT_PATH(app), sample_interval_ms);
#else
    sample_interval_ms = 1000;
#endif
    return 0;
}
