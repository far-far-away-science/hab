#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <sercx\2.0\Sercx.h>

#include <DeviceDefinitions.h>

#define AUX_ENABLES 0x04
    #define AUX_ENABLES_UART (1 << 0)

#define AUX_MU_IO_REGISTER 0x40

#define AUX_MU_IER_REGISTER 0x44 // interrupt enable register
    #define AUX_MU_IER_REGISTER_W_INT_ENABLE_RECEIVE_DATA_AVAILABLE (1 << 0)
    #define AUX_MU_IER_REGISTER_W_INT_ENABLE_LINE_STATUS_CHANGE     (1 << 2)

#define AUX_MU_IIR_REGISTER 0x48 // interrupt identity register
    #define AUX_MU_IIR_REGISTER_W_RESET_RECEIVE_FIFO  (1 << 1)
    #define AUX_MU_IIR_REGISTER_W_RESET_TRANSMIT_FIFO (1 << 2)

#define AUX_MU_LCR_REGISTER 0x4C
    #define AUX_MU_LCR_REGISTER_W_DLAB (1 << 7) // this is a flag which chooses what some flags in other registers mean

#define AUX_MU_MCR_REGISTER 0x50
    #define AUX_MU_MCR_REGISTER_W_REQUEST_TO_SEND (1 << 1) // (if 1 then RTS line is low)

#define AUX_MU_LSR_REGISTER 0x54
    #define AUX_MU_LSR_REGISTER_R_DATA_READY        (1 << 0)
    #define AUX_MU_LSR_REGISTER_R_OVERRUN_ERROR     (1 << 1)
    #define AUX_MU_LSR_REGISTER_R_TRANSMITTER_EMPTY (1 << 5)
    #define AUX_MU_LSR_REGISTER_R_TRANSMITTER_IDLE  (1 << 6)

#define AUX_MU_MSR_REGISTER 0x58

        // only works if DLAB is 1
#define AUX_MU_BAUD_RATE_LSB_REGISTER AUX_MU_IO_REGISTER // least significant byte
        // only works if DLAB is 1
#define AUX_MU_BAUD_RATE_MSB_REGISTER AUX_MU_IER_REGISTER // most significan byte
