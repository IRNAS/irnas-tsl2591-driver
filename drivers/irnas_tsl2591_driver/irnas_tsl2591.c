/** @file irnas_tsl2591.c
 *
 * @brief TSL2591 sensor driver.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas. All rights reserved.
 */

#define DT_DRV_COMPAT irnas_tsl2591

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

#include "irnas_tsl2591.h"

LOG_MODULE_REGISTER(TSL2591, CONFIG_SENSOR_LOG_LEVEL);

#define TSL2591_DEVICE_ID 0x50

static int prv_tsl2591_set_timing(const struct device *dev, enum tsl2591_adc_integration_time atime)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	if (i2c_reg_update_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONFIG,
				   TSL2591_ATIME_MASK,
				   ((uint8_t)atime << TSL2591_ATIME_SHIFT)) < 0) {
		LOG_DBG("Failed to set ATIME.");
		return -EIO;
	}

	uint8_t tmp = 0x00;
	if (i2c_reg_read_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONFIG,
				 &tmp) < 0) {
		LOG_DBG("Failed to read ATIME.");
		return -EIO;
	}
	if (((tmp & TSL2591_ATIME_MASK) >> TSL2591_ATIME_SHIFT) != atime) {
		LOG_ERR("Failed to set integration timing! Should be: %x but is: %x", atime,
			(tmp & TSL2591_ATIME_MASK) >> TSL2591_ATIME_SHIFT);
	}
	LOG_DBG("Integration time set to: %x", (tmp & TSL2591_ATIME_MASK) >> TSL2591_ATIME_SHIFT);

	return 0;
}

static int prv_tsl2591_set_gain(const struct device *dev, enum tsl2591_gain again)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	if (i2c_reg_update_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONFIG,
				   TSL2591_AGAIN_MASK,
				   ((uint8_t)again) << TSL2591_AGAIN_SHIFT) < 0) {
		LOG_ERR("Failed to set AGAIN.");
		return -EIO;
	}

	uint8_t tmp = 0x00;
	if (i2c_reg_read_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONFIG,
				 &tmp) < 0) {
		LOG_DBG("Failed to read AGAIN.");
		return -EIO;
	}
	if (((tmp & TSL2591_AGAIN_MASK) >> TSL2591_AGAIN_SHIFT) != again) {
		LOG_ERR("Failed to set gain! Should be: %x but is: %x", again,
			(tmp & TSL2591_AGAIN_MASK) >> TSL2591_AGAIN_SHIFT);
	}
	LOG_DBG("Gain set to: %x", (tmp & TSL2591_AGAIN_MASK) >> TSL2591_AGAIN_SHIFT);

	return 0;
}

static int prv_tsl2591_power_on(const struct device *dev)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	if (i2c_reg_update_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,
				   TSL2591_ENABLE_POWERON, 0xff) < 0) {
		return -EIO;
	}

	return 0;
}

static int prv_tsl2591_power_off(const struct device *dev)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	if (i2c_reg_update_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,
				   TSL2591_ENABLE_POWERON, 0x00) < 0) {
		return -EIO;
	}

	return 0;
}

static int prv_tsl2591_enable_als(const struct device *dev)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	if (i2c_reg_update_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,
				   TSL2591_ENABLE_AEN, 0xff) < 0) {
		return -EIO;
	}

	return 0;
}

static int prv_tsl2591_dummy_transfer(const struct device *dev)
{
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	/* Make test i2c transaction - for some reason first i2c transaction fails */
	struct i2c_msg msgs[1];
	uint8_t dst = 1;
	/* Send the address to read from */
	msgs[0].buf = &dst;
	msgs[0].len = 1U;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	return i2c_transfer_dt(&config->i2c, &msgs[0], 1);
}

static int tsl2591_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct tsl2591_data *data = dev->data;
	const struct tsl2591_config *config = dev->config;

	uint8_t ch0[2], ch1[2];

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL);

	/* Power on */
	prv_tsl2591_power_on(dev);

	/* Calculate minimum sleep time after power on - we need to sleep for a bit more than
	 * integration time value*/
	uint32_t sleep = 100 * config->atime + 110;
	k_sleep(K_MSEC(sleep));

	if (i2c_burst_read_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW, ch0,
			      2)) {
		LOG_ERR("Failed to read CH0 data.");
	}

	data->chan0 = (ch0[1] << 8) + ch0[0];

	LOG_DBG("CH0 lsb: %x msb: %x combined: %d", ch0[0], ch0[1], data->chan0);

	if (i2c_burst_read_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW, ch1,
			      2)) {
		LOG_ERR("Failed to read CH1 data.");
	}

	data->chan1 = (ch1[1] << 8) + ch1[0];

	LOG_DBG("CH1 lsb: %x msb: %x combined: %d", ch1[0], ch1[1], data->chan1);

	/* Power off */
	prv_tsl2591_power_off(dev);

	return 0;
}

static int tsl2591_channel_get(const struct device *dev, enum sensor_channel chan,
			       struct sensor_value *val)
{
	struct tsl2591_data *data = dev->data;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL);

	val->val1 = data->chan0;
	val->val2 = data->chan1;

	return 0;
}

static const struct sensor_driver_api tsl2591_api = {
	.sample_fetch = &tsl2591_sample_fetch,
	.channel_get = &tsl2591_channel_get,
};

static int tsl2591_init(const struct device *dev)
{
	struct tsl2591_data *data = dev->data;
	const struct tsl2591_config *config = dev->config;

	if (!device_is_ready(config->i2c.bus)) {
		LOG_ERR("I2C bus device not ready");
		return -ENODEV;
	}

	LOG_DBG("We did find TSL2591 device with address: %x and name: %s!", config->i2c.addr,
		config->i2c.bus->name);

	data->chan0 = 0U;
	data->chan1 = 0U;

	/* Make test i2c transaction - for some reason first i2c transaction fails */
	if (prv_tsl2591_dummy_transfer(dev)) {
		LOG_DBG("Test transaction failed!");
	}

	/* Read device ID */
	uint8_t id;
	if (i2c_reg_read_byte_dt(&config->i2c, TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID,
				 &id) < 0) {
		LOG_ERR("TSL2591 failed to read device ID.");
		return -EIO;
	}

	if (id != TSL2591_DEVICE_ID) {
		LOG_ERR("Invalid device id: %x, should be: %x", id, TSL2591_DEVICE_ID);
		return -EIO;
	}

	LOG_DBG("Read device ID: %x of TSL2591 sensor.", id);

	/* Set default timing */
	if (prv_tsl2591_set_timing(dev, config->atime)) {
		return -EIO;
	}

	/* Set default gain */
	if (prv_tsl2591_set_gain(dev, config->gain)) {
		return -EIO;
	}

	/* Enable ALS */
	if (prv_tsl2591_enable_als(dev)) {
		return -EIO;
	}

	/* Power off */
	prv_tsl2591_power_off(dev);

	return 0;
}

#define TSL2591_DEFINE(inst)                                                                       \
	static struct tsl2591_data tsl2591_data_##inst;                                            \
                                                                                                   \
	static const struct tsl2591_config tsl2591_config_##inst = {                               \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
		COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, gain),                                     \
			    (.gain = DT_INST_PROP(inst, gain), ),                                  \
			    (.gain = TSL2591_GAIN_DEFAULT, ))                                      \
			COND_CODE_1(DT_INST_NODE_HAS_PROP(inst, atime),                            \
				    (.atime = DT_INST_PROP(inst, atime), ),                        \
				    (.atime = TSL2591_ATIME_DEFAULT, ))};                          \
                                                                                                   \
	SENSOR_DEVICE_DT_INST_DEFINE(inst, &tsl2591_init, NULL, &tsl2591_data_##inst,              \
				     &tsl2591_config_##inst, POST_KERNEL,                          \
				     CONFIG_SENSOR_INIT_PRIORITY, &tsl2591_api);

DT_INST_FOREACH_STATUS_OKAY(TSL2591_DEFINE)