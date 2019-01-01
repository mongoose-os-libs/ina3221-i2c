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

#include "mgos_ina3221_internal.h"

static bool mgos_ina3221_detect(struct mgos_i2c *i2c, uint8_t i2caddr) {
  int man_id, die_id;

  man_id = mgos_i2c_read_reg_w(i2c, i2caddr, MGOS_INA3221_REG_MANID);
  die_id = mgos_i2c_read_reg_w(i2c, i2caddr, MGOS_INA3221_REG_DIEID);

  if (man_id != 0x5449) {
    return false;
  }
  if (die_id != 0x3220) {
    return false;
  }
  return true;
}

static bool mgos_ina3221_reset(struct mgos_ina3221 *s) {
  uint16_t val;

  if (!s) {
    return false;
  }

  // Reset
  if (!mgos_i2c_write_reg_w(s->i2c, s->i2caddr, MGOS_INA3221_REG_CONFIG, 0x8000)) {
    return false;
  }
  mgos_usleep(2000);

  // Set config register:
  // - Enable all three channels (default)
  // - 1 sample per measurement (default)
  // - 1.1ms conversion time (default)
  // - Continuous bus and shut measurement (default)
  val  = MGOS_INA3221_CONF_CH1_EN | MGOS_INA3221_CONF_CH2_EN | MGOS_INA3221_CONF_CH3_EN;
  val |= AVG_1 << 9;
  val |= CT_1100US << 6;
  val |= CT_1100US << 3;
  val |= MGOS_INA3221_CONF_MODE_CONT_SHUNT_BUS;

  if (!mgos_i2c_write_reg_w(s->i2c, s->i2caddr, MGOS_INA3221_REG_CONFIG, val)) {
    return false;
  }
  mgos_usleep(2000);

  for (uint8_t chan = 1; chan < 4; chan++) {
    mgos_ina3221_set_shunt_resistance(s, chan, MGOS_INA3221_DEFAULT_SHUNT_OHMS);
  }
  return true;
}

struct mgos_ina3221 *mgos_ina3221_create(struct mgos_i2c *i2c, uint8_t i2caddr) {
  struct mgos_ina3221 *sensor = NULL;

  if (!i2c) {
    return NULL;
  }

  if (!mgos_ina3221_detect(i2c, i2caddr)) {
    LOG(LL_ERROR, ("I2C 0x%02x is not an INA3221", i2caddr));
    return NULL;
  }

  sensor = calloc(1, sizeof(struct mgos_ina3221));
  if (!sensor) {
    return NULL;
  }

  memset(sensor, 0, sizeof(struct mgos_ina3221));
  sensor->i2caddr = i2caddr;
  sensor->i2c     = i2c;

  if (!mgos_ina3221_reset(sensor)) {
    LOG(LL_INFO, ("Could not reset INA3221 at I2C 0x%02x", sensor->i2caddr));
    free(sensor);
    return NULL;
  }
  LOG(LL_INFO, ("INA3221 initialized at I2C 0x%02x", sensor->i2caddr));
  return sensor;
}

void mgos_ina3221_destroy(struct mgos_ina3221 **sensor) {
  if (!*sensor) {
    return;
  }

  free(*sensor);
  *sensor = NULL;
  return;
}

bool mgos_ina3221_get_bus_voltage(struct mgos_ina3221 *sensor, uint8_t chan, float *volts) {
  int16_t val;

  if (!sensor || !volts || chan == 0 || chan > 3) {
    return false;
  }
  val = mgos_i2c_read_reg_w(sensor->i2c, sensor->i2caddr, MGOS_INA3221_REG_BUSVOLTS_B + (chan - 1) * 2);
  if (val == -1) {
    return false;
  }
  // LOG(LL_DEBUG, ("bus[%u] = %d", chan, val));
  *volts  = val / 8;    // first 3 bits are not used
  *volts *= 8.f / 1e3;  // 8mV per LSB
  return true;
}

bool mgos_ina3221_get_shunt_voltage(struct mgos_ina3221 *sensor, uint8_t chan, float *volts) {
  int16_t val;

  if (!sensor || !volts || chan == 0 || chan > 3) {
    return false;
  }
  val = mgos_i2c_read_reg_w(sensor->i2c, sensor->i2caddr, MGOS_INA3221_REG_SHUNTVOLTS_B + (chan - 1) * 2);
  if (val == -1) {
    return false;
  }
  // LOG(LL_DEBUG, ("shunt[%u] = %d", chan, val));
  *volts  = val / 8;    // first 3 bits are not used
  *volts *= 40.f / 1e6; // 40uV per LSB
  return true;
}

bool mgos_ina3221_get_current(struct mgos_ina3221 *sensor, uint8_t chan, float *ampere) {
  float shunt_volts;

  if (!sensor || !ampere || chan == 0 || chan > 3) {
    return false;
  }
  if (!mgos_ina3221_get_shunt_voltage(sensor, chan, &shunt_volts)) {
    return false;
  }
  *ampere = shunt_volts / sensor->shunt_resistance[chan - 1];
  // LOG(LL_DEBUG, ("Rshunt[%u]=%.2fOhms, Vshunt[%u]=%.3fV, Ishunt[%u]=%.3fA", chan, sensor->shunt_resistance[chan], chan, shunt_volts, chan, *ampere));
  return false;
}

bool mgos_ina3221_set_shunt_resistance(struct mgos_ina3221 *sensor, uint8_t chan, float ohms) {
  if (!sensor || chan == 0 || chan > 3) {
    return false;
  }
  sensor->shunt_resistance[chan - 1] = ohms;
  return true;
}

bool mgos_ina3221_get_shunt_resistance(struct mgos_ina3221 *sensor, uint8_t chan, float *ohms) {
  if (!sensor || !ohms || chan == 0 || chan > 3) {
    return false;
  }
  *ohms = sensor->shunt_resistance[chan - 1];
  return true;
}

// Mongoose OS library initialization
bool mgos_ina3221_i2c_init(void) {
  return true;
}
