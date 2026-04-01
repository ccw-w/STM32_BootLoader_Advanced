#ifndef __BL_CRC_H__
#define __BL_CRC_H__

#include "stm32f1xx_hal.h"

uint32_t BL_CRC32_Calculate(const uint8_t *data, uint32_t length);

#endif
