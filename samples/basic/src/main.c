/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_SENSOR_LOG_LEVEL);

BUILD_ASSERT(DT_NODE_EXISTS(DT_NODELABEL(tsl2591)), "tsl2591 is not defined in DT");

const struct device *tsl2591_sensor = DEVICE_DT_GET(DT_NODELABEL(tsl2591));
struct sensor_value val;

static void fetch_and_display(const struct device *sensor)
{
	if (!sensor) {
		LOG_ERR("Failed to init tsl2591!");
	}
	int err = sensor_sample_fetch(sensor);
	if (!err) {
		err = sensor_channel_get(sensor, SENSOR_CHAN_ALL, &val);
	}
	if (err < 0) {
		LOG_ERR("ERROR: tsl2591 Update failed: %d\n", err);
	} else {
		LOG_INF("ch0: %d ch1: %d", val.val1, val.val2);
	}
}

void main(void)
{
	if (!tsl2591_sensor) {
		LOG_ERR("Failed to init tsl2591!");
		return;
	} else {
		LOG_INF("Bind to tsl2591.");
	}

	if (!device_is_ready(tsl2591_sensor)) {
		LOG_ERR("Device %s is not ready", tsl2591_sensor->name);
		return;
	} else {
		LOG_INF("Device %s is ready", tsl2591_sensor->name);
	}

	while (1) {

		fetch_and_display(tsl2591_sensor);
		k_msleep(1000);
	}
}
