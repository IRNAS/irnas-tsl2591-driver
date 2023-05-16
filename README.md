# tsl2591 sensor driver

IRNAS Zephyr compatible implementation of `tsl2591` lux driver with `i2c` support.

**IMPORTANT NOTE: for some reason first i2c transaction with sensor fails, hence we have a dummy rad inside init function that will fail. Ignore this message as init should go trough!**

Interrupt support is not yet supported in the driver!
## Setup

If you do not already have them you will need to:

* [install west](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#install-west)
* [install east](https://github.com/IRNAS/irnas-east-software)

Then follow these steps:

```bash
west init -m https://github.com/IRNAS/irnas-tsl2591-driver irnas-tsl2591-driver
cd irnas-tsl2591-driver/project
west update

# set up east globally (this only needs to be done once on each machine)
east sys-setup
# install toolchain for the version of NCS used in this project
east update toolchain
```

## Building and flashing samples

To build the application samples:

```bash
cd samples/sample_name
east build -b irnas-tsl2591-driver
```

To flash the firmware:

```bash
east flash
```

To view RTT logs:

```bash
# Run in first terminal window
east util connect

# Run in second, new terminal window
east util rtt
```

## Driver usage

This driver was written and tested for nrf-sdk v2.3.0

To install, first modify `.../ncs/nrf/west.yml` and add the following sections:

1. In `remotes`, add the following if not already added:

```yaml
 - name: irnas
   url-base: https://github.com/irnas
```

2. In the `projects` section add at the bottom (select revision you need):

```yaml
- name: irnas-tsl2591-driver 
    repo-path: irnas-tsl2591-driver
    path: irnas/irnas-tsl2591-driver
    remote: irnas
    revision: dev
```

Then run `west update` in your freshly created bash/command prompt session.

Above command will clone `irnas-tsl2591-driver` repository inside of `ncs/irnas/`. You can now use the driver in your application projects.

In your boards DTS file or overlay file, add tsl2591 device, under selected I2C peripheral. Example of overlay implementation can be found in samples folder.

```
&i2c0 { 
	tsl2591: tsl2591@29 {
		compatible = "irnas,tsl2591";
		reg = <0x29>;
            gain = <1>;
		atime = <0>;
	};
};
```

Only reg property is required. For description of properties see the `irnas,tsl2591.yaml` file or refer to sensor configuration section.
Several senor instances are supported, provided different addresses are specified for each sensor.

User can set `gain` property, defining ADC gain with default value 1 (Medium gain (25x)):

* 0  - Low gain (1x)
* 1  - Medium gain (25x)
* 2  - High gain (428x)
* 3  - Max gain (9876x)

User can set `atime` property, defining ADC integration time with default value 0 (100 ms):

* 0  - 100 millis
* 1  - 200 millis
* 2  - 300 millis
* 3  - 400 millis
* 4  - 500 millis
* 5  - 600 millis

To enable driver functionality `CONFIG_IRNAS_TSL2591=y` must be selected. Keep in mid, it depends on `CONFIG_I2C=y`.

## Sensor API

Standard sensor api is supported. To access functionality bind to the sensor from DT definition:

```
const struct device *tsl2591_sensor = DEVICE_DT_GET(DT_NODELABEL(tsl2591));
```

then fetch new data with fetch function - this will update data for both `chanel0` and `chanel1`:

```
sensor_sample_fetch(sensor);
```

Access data with `get` function:

```
struct sensor_value val;
sensor_channel_get(sensor, SENSOR_CHAN_ALL, &val);
```

where `val1` field of `sensor_value` represents `chanel0` as `uint16_t` value and `val2` field represents `chanel1` as `uint16_t` value. For usage and conversion to LUX value, refer to datasheet.