/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "mgos.h"
#include "mgos_i2c.h"

struct mgos_ina3221;

/*
 * Initialize a INA3221 on the I2C bus `i2c` at address specified in `i2caddr`
 * parameter (default INA3221 is on address 0x40). The sensor will be polled for
 * validity, upon success a new `struct mgos_ina3221` is allocated and
 * returned. If the device could not be found, NULL is returned.
 */
struct mgos_ina3221 *mgos_ina3221_create(struct mgos_i2c *i2c, uint8_t i2caddr);

/*
 * Destroy the data structure associated with a INA3221 device. The reference
 * to the pointer of the `struct mgos_ina3221` has to be provided, and upon
 * successful destruction, its associated memory will be freed and the pointer
 * set to NULL.
 */
void mgos_ina3221_destroy(struct mgos_ina3221 **sensor);


/* Measure the bus voltage on channel `chan` (which can be 1, 2, or 3). Return the
 * measured voltage in `*volts`, in units of Volts.
 * Returns true on success and false on failure, in which case `*volts` is unspecified.
 */
bool mgos_ina3221_get_bus_voltage(struct mgos_ina3221 *sensor, uint8_t chan, float *volts);

/* Measure the shunt voltage on channel `chan` (which can be 1, 2, or 3). Return the
 * measured voltage in `*volts`, in units of Volts.
 * Returns true on success and false on failure, in which case `*volts` is unspecified.
 */
bool mgos_ina3221_get_shunt_voltage(struct mgos_ina3221 *sensor, uint8_t chan, float *volts);

/* Set the shunt resistor value in units of Ohms on channel `chan` (which can be 1, 2 or 3).
 * Typical values is ohms=0.100 (which yields a 1.6A range on the current sensor).
 * Returns true on success and false on failure.
 */
bool mgos_ina3221_set_shunt_resistance(struct mgos_ina3221 *sensor, uint8_t chan, float ohms);

/* Get the shunt resistor value in units of Ohms on channel `chan` (which can be 1, 2 or 3).
 * Returns true on success and false on failure, in which case `*ohms` is unspecified.
 */
bool mgos_ina3221_get_shunt_resistance(struct mgos_ina3221 *sensor, uint8_t chan, float *ohms);

/* Get the current in units of Amperes on channel `chan` (which can be 1, 2 or 3).
 * Returns true on success and false on failure, in which case `*ampere` is unspecified.
 * Note: Current is simply (Vshunt / Rshunt) due to Ohms law.
 */
bool mgos_ina3221_get_current(struct mgos_ina3221 *sensor, uint8_t chan, float *ampere);
