// Not canonical C. Will replace with #ifdef/#define guards
#pragma once

#include "nmea_messages.h"
#include <stdbool.h>
#include <stdint.h>

// The address adopted by the Tiva for backup communications
#define I2C_ADDRESS 0x2AU
// Our software version, major (API compatible)
#define SW_VERSION_MAJOR 2
// Our software version, minor (revision)
#define SW_VERSION_MINOR 0

// I2C module to use
// NOTE If I2C_MODULE is changed, check initializeI2C to update pin mappings/clocks!
#define I2C_MODULE I2C1_BASE

// Virtual register addresses (the Pi can use these values)
// For documentation see i2c.c header comment
#define REG_WHO_AM_I 0x00
#define REG_SW_VERSION_MAJOR 0x01
#define REG_SW_VERSION_MINOR 0x02
#define REG_DATA_AVAILABLE 0x03
#define REG_LAT_0 0x00
#define REG_LAT_1 0x01
#define REG_LAT_2 0x02
#define REG_LAT_3 0x03
#define REG_LON_0 0x04
#define REG_LON_1 0x05
#define REG_LON_2 0x06
#define REG_LON_3 0x07
#define REG_VEL_0 0x08
#define REG_VEL_1 0x09
#define REG_HDG_0 0x0A
#define REG_HDG_1 0x0B
#define REG_ALT_0 0x0C
#define REG_ALT_1 0x0D
#define REG_ALT_2 0x0E
#define REG_ALT_3 0x0F
#define REG_SAT 0x10
// 2 GPS data sets
#define REG_BANK_1 0x10
#define REG_BANK_2 0x30
// Must be last register address + 1
#define I2C_NUM_REGS 0x41

// Initialize I2C module as slave and configures pin muxes to I2C1
void initializeI2C(void);
// Submits parsed GPS data to the I2C subsystem
// index is the GPS (0 = Venus, 1 = Copernicus) to update
void submitI2CData(uint32_t index, GpsData *data);
