#include "bl_flash.h"
#include "bl_config.h"

/* 封装FLash擦除、写入、读取 */

HAL_StatusTypeDef BL_Flash_Erase_Area(uint32_t start_addr, uint32_t size)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = start_addr;
    erase_init.NbPages = size / 1024U;

    status = HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();

    return status;
}

HAL_StatusTypeDef BL_Flash_Write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t i;
    uint16_t halfword;

    if ((data == 0U) || (len == 0U))
    {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();

    for (i = 0; i < len; i += 2U)
    {
        halfword = data[i];

        if ((i + 1U) < len)
        {
            halfword |= ((uint16_t)data[i + 1U] << 8);
        }
        else
        {
            halfword |= 0xFF00U;
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, halfword);
        if (status != HAL_OK)
        {
            break;
        }
    }

    HAL_FLASH_Lock();

    return status;
}
