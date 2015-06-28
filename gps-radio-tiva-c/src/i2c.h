// Not canonical C. Will replace with #ifdef/#define guards
#pragma once

#include <stdbool.h>
#include <stdint.h>

// The address adopted by the Tiva for backup communications
#define I2C_ADDRESS 0x2AU
// Our software version, major (API compatible)
#define SW_VERSION_MAJOR 1
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
#define REG_LAT_0 0x04
#define REG_LAT_1 0x05
#define REG_LAT_2 0x06
#define REG_LAT_3 0x07
#define REG_LON_0 0x08
#define REG_LON_1 0x09
#define REG_LON_2 0x0A
#define REG_LON_3 0x0B
#define REG_VEL_0 0x0C
#define REG_VEL_1 0x0D
#define REG_VEL_2 0x0E
#define REG_VEL_3 0x0F
#define REG_HDG_0 0x10
#define REG_HDG_1 0x11
// Must be last register address + 1
#define I2C_NUM_REGS 0x12

// Initialize I2C module as slave and configures pin muxes to I2C3
void initializeI2C(void);
// Submits parsed GPS data to the I2C subsystem
// lat and lon are 32-bit signed decimal degrees * 1E6
// vel TBD
// head is 16-bit unsigned degrees (32-bit parameter will be cut down to 16-bit)
void submitI2CData(int32_t lat, int32_t lon, uint32_t vel, uint32_t head);
