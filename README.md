# INA3221 I2C Driver

A Mongoose library for Texas Instruments' 3-channel current sensor.

## Implementation details

The `INA3221` is a simple I2C device that measures current through shunt
resistors on three separate channels. It is able to measure the voltage
on each channel (max 26V, in 8mV increments) and the voltage drop over the
shunt resistor in 40uV increments with a range of 163.8mV, and is quite
precise.

Using a 0.1Ohm shunt resistor, the maximum current that can be measured
is 1.638A. Each channel can be enabled/disabled and polled individually.

## API Description

Create an `INA3221` object using `mgos_ina3221_create()`, set the shunt
resistor values for each channel using `mgos_ina3221_set_shunt_resistance()`,
then call `mgos_ina3221_get_bus_voltage()` to get the bus voltage,
call `mgos_ina3221_get_shunt_voltage()` to get the voltage drop over
the shunt resistor, and `mgos_ina3221_get_current()` to get the current
flowing through the channel.

Valid channels are `1`, `2` or `3` (ie. they are 1-based).

## Example application

```
#include "mgos.h"
#include "mgos_config.h"
#include "mgos_ina3221.h"

static void ina3221_timer_cb(void *user_data) {
  struct mgos_ina3221 *sensor = (struct mgos_ina3221 *)user_data;

  if (!sensor) return;

  for (uint8_t i=1; i<4; i++) {
    float bus, shunt, current, res;
    mgos_ina3221_get_bus_voltage(sensor, i, &bus);
    mgos_ina3221_get_shunt_resistance(sensor, i, &res);
    mgos_ina3221_get_shunt_voltage(sensor, i, &shunt);
    mgos_ina3221_get_current(sensor, i, &current);
    LOG(LL_INFO, ("Chan[%u]: Vbus=%.3f V Vshunt=%.0f uV Rshunt=%.3f Ohm Ishunt=%.1f mA",
      i, bus, shunt*1e6, res, current*1e3));
  }
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_ina3221 *sensor;

  sensor = mgos_ina3221_create(mgos_i2c_get_global(), mgos_sys_config_get_ina3221_i2caddr());
  if (!sensor) {
    LOG(LL_ERROR, ("Could not create INA3221 sensor"));
    return false;
  }

  mgos_set_timer(1000, true, ina3221_timer_cb, sensor);

  return MGOS_APP_INIT_SUCCESS;
}
```

# Disclaimer

This project is not an official Google project. It is not supported by Google
and Google specifically disclaims all warranties as to its quality,
merchantability, or fitness for a particular purpose.
