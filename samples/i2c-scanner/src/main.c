#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/types.h>

/**
 * @brief Perform i2c scan for given peripheral.
 *
 * @param i2c_dev i2c dev
 * @param found list of found addresses
 * @return int nr of found addresses
 */
static int i2c_scan(const struct device *i2c_dev, uint8_t *found)
{
	int count = 0;
	int error = 0;
	// Start scan
	for (uint16_t i = 4; i <= 255; i++) {
		struct i2c_msg msgs[1];
		uint8_t dst = 1;
		/* Send the address to read from */
		msgs[0].buf = &dst;
		msgs[0].len = 1U;
		msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;
		error = i2c_transfer(i2c_dev, &msgs[0], 1, i);
		if (error == 0) {
			found[count] = i;
			count++;
			printk("Found device on address: 0x%x\n", i);
		}
	}

	return count;
}

void main(void)
{
	int count0 = 0;
	uint8_t found0[256];

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c0), okay)
	// Init device
	const struct device *i2c0_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));
	if (i2c0_dev) {
		while (true) {
			count0 = i2c_scan(i2c0_dev, found0);
			printk("Found: %d devices in total.\n\n", count0);
			k_sleep(K_SECONDS(1));
		}
	}
#endif
}