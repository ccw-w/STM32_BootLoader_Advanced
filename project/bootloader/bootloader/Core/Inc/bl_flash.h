#ifndef __BL_FLASH_H__
#define __BL_FLASH_H__

#include "stm32f1xx_hal.h"

/* Flash操作接口 */

HAL_StatusTypeDef BL_Flash_Erase_Area(uint32_t start_addr, uint32_t size);
HAL_StatusTypeDef BL_Flash_Write(uint32_t addr, const uint8_t *data, uint32_t len);

#endif
