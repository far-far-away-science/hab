Compilation flags:

TRIG_SLOW
- defined:     use sinf/cosf/asinf functions to create PWM signal
- not defined: use SIN/ASIN tables instead of trig functions (COS 
               is calculated from SIN as Keil doesn't allow code 
               more than 32K in free version)
               
DEBUG
- defined:     watchdog timer is disabled
- not defined: watchdog timer has to be reset every 2 seconds or board will reboot (not good for debugging)

DUMP_DATA_TO_UART0
- defined:     will output GPS/Temperature/Voltage to UART0
- not defined: won't
