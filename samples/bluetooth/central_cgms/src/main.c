/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Central CGMS Client sample
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>
#include <dk_buttons_and_leds.h>
#include <sfloat.h>
#include <math.h>

static struct bt_conn *default_conn;
static struct bt_gatt_subscribe_params subscribe_params;
static struct bt_gatt_read_params read_feature_params;
static struct bt_gatt_read_params read_status_params;

static uint16_t cgm_measurement_handle;
static uint16_t cgm_measurement_ccc_handle;
static uint16_t cgm_feature_handle;
static uint16_t cgm_status_handle;

static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match,
			      bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	printk("CGMS device found: %s connectable: %s\n",
		addr, connectable ? "yes" : "no");
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	printk("Connecting failed\n");
}

static void scan_connecting(struct bt_scan_device_info *device_info,
			    struct bt_conn *conn)
{
	default_conn = bt_conn_ref(conn);
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL,
		scan_connecting_error, scan_connecting);

static uint8_t notify_cgm_measurement_cb(struct bt_conn *conn,
					 struct bt_gatt_subscribe_params *params,
					 const void *data, uint16_t length)
{
	if (!data) {
		printk("CGM Measurement unsubscribed\n");
		params->value_handle = 0U;
		return BT_GATT_ITER_STOP;
	}

	if (length < 6) {
		printk("CGM Measurement notification too short: %d bytes\n", length);
		return BT_GATT_ITER_CONTINUE;
	}

	const uint8_t *meas_data = data;
	
	/* Parse CGM Measurement structure:
	 * Byte 0: Size (should be 6)
	 * Byte 1: Flags
	 * Bytes 2-3: Glucose Concentration (SFLOAT)
	 * Bytes 4-5: Time Offset
	 */
	uint8_t flags = meas_data[1];
	struct sfloat glucose_sfloat;
	
	/* Extract SFLOAT value (little endian) */
	glucose_sfloat.val = meas_data[2] | (meas_data[3] << 8);
	
	/* Manually decode SFLOAT: 4-bit exponent + 12-bit mantissa */
	int16_t mantissa = glucose_sfloat.val & 0x0FFF;
	int8_t exponent = (glucose_sfloat.val >> 12) & 0x0F;
	
	/* Sign extend mantissa (12-bit to 16-bit) */
	if (mantissa & 0x0800) {
		mantissa |= 0xF000;
	}
	
	/* Sign extend exponent (4-bit to 8-bit) */
	if (exponent & 0x08) {
		exponent |= 0xF0;
	}
	
	/* Calculate the actual value */
	double glucose_value = mantissa * pow(10.0, exponent);
	
	/* Extract time offset */
	uint16_t time_offset = meas_data[4] | (meas_data[5] << 8);
	
	printk("Glucose notification: %d.%01d mg/dL (time offset: %u secs, flags: 0x%02x)\n",
	       (int)glucose_value, (int)(glucose_value * 10) % 10, time_offset, flags);

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t read_cgm_feature_cb(struct bt_conn *conn, uint8_t err,
				   struct bt_gatt_read_params *params,
				   const void *data, uint16_t length)
{
	if (err) {
		printk("CGM Feature read failed (err %d)\n", err);
		return BT_GATT_ITER_STOP;
	}

	if (!data) {
		return BT_GATT_ITER_STOP;
	}

	if (length >= 6) {
		const uint8_t *feature_data = data;
		uint32_t feature = feature_data[0] | (feature_data[1] << 8) | 
				   (feature_data[2] << 16);
		uint8_t type = feature_data[3];
		uint8_t sample_location = feature_data[4];
		
		printk("CGM Feature: Features=0x%06X, Type=%u, Location=%u\n",
		       feature, type, sample_location);
	} else {
		printk("CGM Feature data too short: %d bytes\n", length);
	}

	return BT_GATT_ITER_STOP;
}

static uint8_t read_cgm_status_cb(struct bt_conn *conn, uint8_t err,
				  struct bt_gatt_read_params *params,
				  const void *data, uint16_t length)
{
	if (err) {
		printk("CGM Status read failed (err %d)\n", err);
		return BT_GATT_ITER_STOP;
	}

	if (!data) {
		return BT_GATT_ITER_STOP;
	}

	if (length >= 5) {
		const uint8_t *status_data = data;
		uint16_t time_offset = status_data[0] | (status_data[1] << 8);
		uint32_t status = status_data[2] | (status_data[3] << 8) | 
				  (status_data[4] << 16);
		
		printk("CGM Status: Time offset=%u min, Status=0x%06X\n",
		       time_offset, status);
	} else {
		printk("CGM Status data too short: %d bytes\n", length);
	}

	return BT_GATT_ITER_STOP;
}

static void discovery_completed_cb(struct bt_gatt_dm *dm, void *context)
{
	int err;
	const struct bt_gatt_dm_attr *gatt_chrc;
	const struct bt_gatt_dm_attr *gatt_desc;

	printk("Service discovery completed\n");
	bt_gatt_dm_data_print(dm);

	/* Find CGM Measurement characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_CGM_MEASUREMENT);
	if (!gatt_chrc) {
		printk("CGM Measurement characteristic not found\n");
		goto release;
	}

	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_CGM_MEASUREMENT);
	if (!gatt_desc) {
		printk("CGM Measurement descriptor not found\n");
		goto release;
	}

	cgm_measurement_handle = gatt_desc->handle;

	/* Find CCC descriptor for CGM Measurement */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		printk("CGM Measurement CCC not found\n");
		goto release;
	}

	cgm_measurement_ccc_handle = gatt_desc->handle;

	printk("CGM Measurement handle: %u, CCC handle: %u\n",
	       cgm_measurement_handle, cgm_measurement_ccc_handle);

	/* Find CGM Feature characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_CGM_FEATURE);
	if (gatt_chrc) {
		gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_CGM_FEATURE);
		if (gatt_desc) {
			cgm_feature_handle = gatt_desc->handle;
			printk("CGM Feature handle: %u\n", cgm_feature_handle);
		}
	}

	/* Find CGM Status characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_CGM_STATUS);
	if (gatt_chrc) {
		gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_CGM_STATUS);
		if (gatt_desc) {
			cgm_status_handle = gatt_desc->handle;
			printk("CGM Status handle: %u\n", cgm_status_handle);
		}
	}

	/* Subscribe to CGM Measurement notifications */
	subscribe_params.notify = notify_cgm_measurement_cb;
	subscribe_params.value = BT_GATT_CCC_NOTIFY;
	subscribe_params.value_handle = cgm_measurement_handle;
	subscribe_params.ccc_handle = cgm_measurement_ccc_handle;

	err = bt_gatt_subscribe(default_conn, &subscribe_params);
	if (err && err != -EALREADY) {
		printk("Subscribe to CGM Measurement failed (err %d)\n", err);
	} else {
		printk("Subscribed to CGM Measurement notifications\n");
	}

	/* Read CGM Feature */
	if (cgm_feature_handle) {
		read_feature_params.func = read_cgm_feature_cb;
		read_feature_params.handle_count = 1;
		read_feature_params.single.handle = cgm_feature_handle;
		read_feature_params.single.offset = 0;

		err = bt_gatt_read(default_conn, &read_feature_params);
		if (err) {
			printk("CGM Feature read failed (err %d)\n", err);
		}
	}

	/* Read CGM Status */
	if (cgm_status_handle) {
		read_status_params.func = read_cgm_status_cb;
		read_status_params.handle_count = 1;
		read_status_params.single.handle = cgm_status_handle;
		read_status_params.single.offset = 0;

		err = bt_gatt_read(default_conn, &read_status_params);
		if (err) {
			printk("CGM Status read failed (err %d)\n", err);
		}
	}

release:
	err = bt_gatt_dm_data_release(dm);
	if (err) {
		printk("Could not release discovery data (err %d)\n", err);
	}
}

static void discovery_service_not_found_cb(struct bt_conn *conn, void *context)
{
	printk("CGMS service not found!\n");
}

static void discovery_error_found_cb(struct bt_conn *conn, int err, void *context)
{
	printk("Service discovery failed (err %d)\n", err);
}

static struct bt_gatt_dm_cb discovery_cb = {
	.completed = discovery_completed_cb,
	.service_not_found = discovery_service_not_found_cb,
	.error_found = discovery_error_found_cb,
};

static void gatt_discover(struct bt_conn *conn)
{
	int err;

	err = bt_gatt_dm_start(conn, BT_UUID_CGMS, &discovery_cb, NULL);
	if (err) {
		printk("Failed to start service discovery (err %d)\n", err);
	}
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		printk("Security changed: %s level %u\n", addr, level);
	} else {
		printk("Security failed: %s level %u err %d\n", addr, level, err);
	}
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		printk("Failed to connect to %s (err %u)\n", addr, conn_err);

		bt_conn_unref(default_conn);
		default_conn = NULL;

		/* Restart scanning */
		bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
		return;
	}

	printk("Connected: %s\n", addr);

	if (conn == default_conn) {
		/* Don't manually set security - let GATT operations trigger it */
		gatt_discover(conn);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason %u)\n", addr, reason);

	if (default_conn != conn) {
		return;
	}

	bt_conn_unref(default_conn);
	default_conn = NULL;

	/* Reset handles */
	cgm_measurement_handle = 0;
	cgm_measurement_ccc_handle = 0;
	cgm_feature_handle = 0;
	cgm_status_handle = 0;

	/* Restart scanning */
	bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u (displaying for Numeric Comparison)\n", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u (auto-confirming)\n", addr, passkey);

	/* Automatically confirm passkey */
	bt_conn_auth_passkey_confirm(conn);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
};

static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static int scan_init(void)
{
	int err;
	struct bt_scan_init_param scan_init = {
		.connect_if_match = true,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_CGMS);
	if (err) {
		printk("Failed to add UUID filter (err %d)\n", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
	if (err) {
		printk("Failed to enable UUID filter (err %d)\n", err);
		return err;
	}

	return 0;
}

int main(void)
{
	int err;

	printk("Starting Central CGMS Client\n");

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	bt_conn_auth_cb_register(&conn_auth_callbacks);

	err = scan_init();
	if (err) {
		printk("Scan init failed (err %d)\n", err);
		return 0;
	}

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return 0;
	}

	printk("Scanning started successfully\n");

	return 0;
}
