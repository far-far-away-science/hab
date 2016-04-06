// Not canonical C. Will replace with #ifdef/#define guards
#pragma once

#include <stdbool.h>
#include <stdint.h>

// Initialize EEPROM -- if erase is true, clears everything
// Do not use when the watchdog is running
void initializeEEPROM(bool erase);
// Read one word from EEPROM
uint32_t eepromRead(uint32_t address);
// Write one word to EEPROM
void eepromWrite(uint32_t address, uint32_t *data);
