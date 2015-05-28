#pragma once

#include <stdint.h>

#define MESSAGE_MAX_LEN 128

struct Message
{
    uint8_t size;
    // to make sure there is no overflow check last 2 characters are 0x0D,0x0A.
    uint8_t message[MESSAGE_MAX_LEN + 1];
};
