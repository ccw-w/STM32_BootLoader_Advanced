#include "bl_meta.h"
#include "bl_config.h"

static HAL_StatusTypeDef BL_Meta_WriteWord(uint32_t addr, uint32_t value)
{
    HAL_StatusTypeDef status;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);
    return status;
}

void BL_Meta_Set(BL_MetaStatus_t status, uint32_t size, uint32_t crc32, uint32_t version)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = META_FLASH_START_ADDR;
    erase_init.NbPages = 1;

    HAL_FLASHEx_Erase(&erase_init, &page_error);

    BL_Meta_WriteWord(META_FLASH_START_ADDR + 0,  BL_META_MAGIC);
    BL_Meta_WriteWord(META_FLASH_START_ADDR + 4,  (uint32_t)status);
    BL_Meta_WriteWord(META_FLASH_START_ADDR + 8,  size);
    BL_Meta_WriteWord(META_FLASH_START_ADDR + 12, crc32);
    BL_Meta_WriteWord(META_FLASH_START_ADDR + 16, version);

    HAL_FLASH_Lock();
}

void BL_Meta_Clear(void)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = META_FLASH_START_ADDR;
    erase_init.NbPages = 1;

    HAL_FLASHEx_Erase(&erase_init, &page_error);

    HAL_FLASH_Lock();
}

uint8_t BL_Meta_Read(BL_MetaInfo_t *info)
{
    if (info == 0)
    {
        return 0;
    }

    info->magic   = *(volatile uint32_t *)(META_FLASH_START_ADDR + 0);
    info->status  = *(volatile uint32_t *)(META_FLASH_START_ADDR + 4);
    info->size    = *(volatile uint32_t *)(META_FLASH_START_ADDR + 8);
    info->crc32   = *(volatile uint32_t *)(META_FLASH_START_ADDR + 12);
    info->version = *(volatile uint32_t *)(META_FLASH_START_ADDR + 16);

    if (info->magic != BL_META_MAGIC)
    {
        return 0;
    }

    return 1;
}
