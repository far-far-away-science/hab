Compilation flags:

DEBUG
- defined:     watchdog timer is disabled
- not defined: watchdog timer has to be reset every 2 seconds or board will reboot (not good for debugging)

DUMP_DATA_TO_UART0
- defined:     will output GPS/Temperature/Voltage to UART0
- not defined: won't

EEPROM_ENABLED
- defined:     data will be stored to EEPROM
- not defined: won't
