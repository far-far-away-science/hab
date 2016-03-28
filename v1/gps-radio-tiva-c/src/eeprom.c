#include "eeprom.h"
#include "timer.h"

#include <driverlib/eeprom.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/rom_map.h>

#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>

// Initialize EEPROM -- if erase is true, clears everything
void initializeEEPROM(bool erase)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    MAP_IntDisable(INT_FLASH);
    MAP_EEPROMInit();
    if (erase)
        MAP_EEPROMMassErase();
}

// Read one word from EEPROM
uint32_t eepromRead(uint32_t address)
{
    uint32_t data;
    MAP_EEPROMRead(&data, address, 4U);
    return data;
}

// Write one word to EEPROM
void eepromWrite(uint32_t address, uint32_t *data)
{
    feedWatchdog();
    MAP_EEPROMProgram(data, address, 4U);
}
