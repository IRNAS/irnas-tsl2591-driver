/** @file irnas_tsl2591.h
 *
 * @brief TSL2591 sensor driver.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas. All rights reserved.
 */

#ifndef IRNAS_TSL2591_H_
#define IRNAS_TSL2591_H_

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

/* Command register */
#define TSL2591_COMMAND_BIT		 0xA0 // 1010 0000: bits 7 and 5 for 'command normal'
#define TSL2591_COMMAND_INT_SET		 0xE4 // Interrupt set – forces an interrupt
#define TSL2591_COMMAND_INT_CLEAR_ALS	 0xE6 // Clears ALS interrupt
#define TSL2591_COMMAND_INT_CLEAR	 0xE7 // Clears ALS and no persist ALS interrupt
#define TSL2591_COMMAND_INT_CLEAR_ALS	 0xE6 // Clears ALS interrupt
#define TSL2591_COMMAND_INT_CLEAR_ALS_NP 0xEA // Clears no persist ALS interrupt

/* Registers */
#define TSL2591_REGISTER_ENABLE		   0x00	 // Enables states and interrupts
#define TSL2591_REGISTER_CONFIG		   0x01	 // ALS gain and integration time configuration
#define TSL2591_REGISTER_THRESHOLD_AILTL   0x04	 // ALS low threshold lower byte
#define TSL2591_REGISTER_THRESHOLD_AILTH   0x05	 // ALS low threshold upper byte
#define TSL2591_REGISTER_THRESHOLD_AIHTL   0x06	 // ALS high threshold lower byte
#define TSL2591_REGISTER_THRESHOLD_AIHTH   0x07	 // ALS high threshold upper byte
#define TSL2591_REGISTER_THRESHOLD_NPAILTL 0x08	 // No Persist ALS low threshold lower byte
#define TSL2591_REGISTER_THRESHOLD_NPAILTH 0x09	 // No Persist ALS low threshold higher byte
#define TSL2591_REGISTER_THRESHOLD_NPAIHTL 0x0A	 // No Persist ALS high threshold lower byte
#define TSL2591_REGISTER_THRESHOLD_NPAIHTH 0x0B, // No Persist ALS high threshold higher byte
#define TSL2591_REGISTER_PERSIST_FILTER	   0x0C	 // Interrupt persistence filter
#define TSL2591_REGISTER_PACKAGE_PID	   0x11	 // Package Identification
#define TSL2591_REGISTER_DEVICE_ID	   0x12	 // Device Identification
#define TSL2591_REGISTER_DEVICE_STATUS	   0x13	 // Internal Status
#define TSL2591_REGISTER_CHAN0_LOW	   0x14	 // Channel 0 data, low byte
#define TSL2591_REGISTER_CHAN0_HIGH	   0x15	 // Channel 0 data, high byte
#define TSL2591_REGISTER_CHAN1_LOW	   0x16	 // Channel 1 data, low byte
#define TSL2591_REGISTER_CHAN1_HIGH	   0x17	 // Channel 1 data, high byte

/* Bit masks */
#define TSL2591_ATIME_MASK  0x07 // ADC integration time mask for TSL2591_REGISTER_CONFIG
#define TSL2591_ATIME_SHIFT 0	 // ADC integration bit shift TSL2591_REGISTER_CONFIG
#define TSL2591_AGAIN_MASK  0x30 // ALS gain mask for TSL2591_REGISTER_CONFIG
#define TSL2591_AGAIN_SHIFT 4	 // Bit shift of ALS gain

/* ENABLE mask */
#define TSL2591_ENABLE_POWEROFF 0x00 // Flag for ENABLE register to disable
#define TSL2591_ENABLE_POWERON	0x01 // Flag for ENABLE register to enable
#define TSL2591_ENABLE_AEN                                                                         \
	0x02 // ALS Enable. This field activates ALS function. Writing a one activates the ALS.
	     // Writing a zero disables the ALS.
#define TSL2591_ENABLE_AIEN                                                                        \
	0x10 // ALS Interrupt Enable. When asserted permits ALS interrupts to be generated, subject
	     // to the persist filter.
#define TSL2591_ENABLE_NPIEN                                                                       \
	0x80 // No Persist Interrupt Enable. When asserted NP Threshold conditions will generate an
	     // interrupt, bypassing the persist filter

/* LUX calculation coefficients */
#define TSL2591_LUX_DF	  (408.0F) // Lux cooefficient
#define TSL2591_LUX_COEFB (1.64F)  // CH0 coefficient
#define TSL2591_LUX_COEFC (0.59F)  // CH1 coefficient A
#define TSL2591_LUX_COEFD (0.86F)  // CH2 coefficient B

/* Enumeration for the sensor gain */
enum tsl2591_gain {
	TSL2591_GAIN_LOW = 0x00,  // low gain (1x)
	TSL2591_GAIN_MED = 0x01,  // medium gain (25x)
	TSL2591_GAIN_HIGH = 0x02, // medium gain (428x)
	TSL2591_GAIN_MAX = 0x03,  // max gain (9876x)
};

#define TSL2591_GAIN_DEFAULT TSL2591_GAIN_MED;

/* Enumeration for the sensor integration timing */
enum tsl2591_adc_integration_time {
	TSL2591_INTEGRATIONTIME_100MS = 0x00, // 100 millis
	TSL2591_INTEGRATIONTIME_200MS = 0x01, // 200 millis
	TSL2591_INTEGRATIONTIME_300MS = 0x02, // 300 millis
	TSL2591_INTEGRATIONTIME_400MS = 0x03, // 400 millis
	TSL2591_INTEGRATIONTIME_500MS = 0x04, // 500 millis
	TSL2591_INTEGRATIONTIME_600MS = 0x05, // 600 millis
};

#define TSL2591_ATIME_DEFAULT TSL2591_INTEGRATIONTIME_100MS

struct tsl2591_data {
	uint16_t chan0; // Channel 0 data IR+Visible
	uint16_t chan1; // Channel 1 data IR
};

struct tsl2591_config {
	struct i2c_dt_spec i2c;
	enum tsl2591_gain gain;
	enum tsl2591_adc_integration_time atime;
};

#endif // IRNAS_TSL2591_H_