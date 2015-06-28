/*
 * i2c.c - I2C backup communications of telemetry MCU with Raspberry PI 2
 *
 * Acts as an I2C slave with the address set in i2c.h. Supports 100 KHz and 400 KHz. The
 * signals appear on PD0 (I2C3SCL) and PD1 (I2C3SDA). These are pins 61 and 62 respectively
 * on the MCU, corresponding to header pins J2.07 and J2.06 on the Tiva C LaunchPad.
 *
 * Register map:
 * [0x00] - WHO_AM_I - always returns the I2C slave address
 * [0x01] - SW_VERSION_MAJOR - returns the major revision defined in i2c.h
 * [0x02] - SW_VERSION_MINOR - returns the minor revision defined in i2c.h
 * [0x03] - DATA_AVAILABLE - returns 1 if data has been updated since last read of any DR, 0 otherwise
 * [0x04-0x07] - LAT - Latitude as signed 32 bit integer in degrees * 1E6 LSB first
 * [0x08-0x0B] - LON - Longitude as signed 32 bit integer in degrees * 1E6 LSB first
 * [0x0C-0x0F] - VEL - Velocity as signed 32 bit integer [TBD] LSB first
 * [0x10-0x11] - HDG - Heading as unsigned 16 bit integer in degrees LSB first
 */

#include "i2c.h"
#include <string.h>

#include <driverlib/i2c.h>
#include <driverlib/rom.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

static struct {
	// I2C register data
	uint8_t regs[I2C_NUM_REGS];
	// Address pointer
	volatile uint8_t address;
} i2cData;

static void i2cSlaveRequestHandler(void) {
	const uint32_t action = MAP_I2CSlaveStatus(I2C_MODULE);
	bool ack = false;
	// Shut off the alarm clock to prevent us from being called again
	MAP_I2CSlaveIntClear(I2C_MODULE);
	switch (action) {
	case I2C_SLAVE_ACT_RREQ_FBR:
	{
		// This is the address
		uint32_t newAddress = MAP_I2CSlaveDataGet(I2C_MODULE);
		if (newAddress >= I2C_NUM_REGS)
			// Prevent array access out of bounds
			newAddress = I2C_NUM_REGS - 1U;
		i2cData.address = (uint8_t)newAddress;
		ack = true;
		break;
	}
	case I2C_SLAVE_ACT_RREQ:
		// This is not allowed, all registers are currently read only
		break;
	case I2C_SLAVE_ACT_TREQ:
	{
		// Data has been requested from us
		uint32_t address = (uint32_t)i2cData.address;
		// Clear data available flag if necessary
		if (address >= REG_LON_0 && address <= REG_HDG_1)
			i2cData.regs[REG_DATA_AVAILABLE] = 0U;
		// Store data with auto increment
		MAP_I2CSlaveDataPut(I2C_MODULE, i2cData.regs[address++]);
		if (address >= I2C_NUM_REGS)
			// Prevent array access out of bounds
			address = 0U;
		i2cData.address = (uint8_t)address;
		ack = true;
		break;
	}
	default:
		// No action, or an invalid action (No QCMD, 2nd address on this device)
		break;
	}
	// Send ACK/NACK
	MAP_I2CSlaveACKValueSet(I2C_MODULE, ack);
	MAP_I2CSlaveACKOverride(I2C_MODULE, true);
}

void submitI2CData(uint32_t lat, uint32_t lon, uint32_t vel, uint32_t head) {
	union {
		uint8_t bytes[4];
		uint32_t word;
	} dataLE;
	// Mask I2C interrupts while we update
	MAP_I2CSlaveIntDisable(I2C_MODULE);
	// Latitude update
	dataLE.word = lat;
	memcpy(&(i2cData.regs)[REG_LAT_0], dataLE.bytes, sizeof(uint32_t));
	// Longitude update
	dataLE.word = lon;
	memcpy(&(i2cData.regs)[REG_LON_0], dataLE.bytes, sizeof(uint32_t));
	// Velocity update
	dataLE.word = vel;
	memcpy(&(i2cData.regs)[REG_VEL_0], dataLE.bytes, sizeof(uint32_t));
	// Heading update
	dataLE.word = head;
	memcpy(&(i2cData.regs)[REG_HDG_0], &(dataLE.bytes)[2], sizeof(uint16_t));
	// Data is available
	i2cData.regs[REG_DATA_AVAILABLE] = 1U;
	MAP_I2CSlaveIntEnableEx(I2C_MODULE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START);
}

void initializeI2C(void) {
	// Peripheral enable: the I/O port and the I2C module
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3);
	// Set up pins to I2C mode
	ROM_GPIOPinConfigure(GPIO_PD0_I2C3SCL);
	ROM_GPIOPinConfigure(GPIO_PD1_I2C3SDA);
	ROM_GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);
	ROM_GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0);
	// Set up in slave mode with correct address
	// NOTE TM4C123 does not have slave functions in its ROM, do not try, you will be sad
	MAP_I2CSlaveEnable(I2C_MODULE);
	MAP_I2CSlaveInit(I2C_MODULE, I2C_ADDRESS);
	// Register IRQ and clear spurious conditions
	I2CIntRegister(I2C_MODULE, &i2cSlaveRequestHandler);
	MAP_I2CSlaveIntEnableEx(I2C_MODULE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START);
	MAP_I2CSlaveIntClear(I2C_MODULE);
	// Init register file
	i2cData.address = 0U;
	memset(i2cData.regs, 0, sizeof(i2cData.regs));
	i2cData.regs[REG_WHO_AM_I] = I2C_ADDRESS;
	i2cData.regs[REG_SW_VERSION_MAJOR] = SW_VERSION_MAJOR;
	i2cData.regs[REG_SW_VERSION_MINOR] = SW_VERSION_MINOR;
	// Should we use the uDMA?
	// XXX Delete me
	submitI2CData(128000000, -64000000, 5, 359);
}