/**
 * @file    app_confirm.c
 * @brief   Application-side boot confirmation implementation
 *
 * This module is used by the application to confirm
 * that the new image booted successfully.
 */
#include "app_confirm.h"

/**
 * @brief Read metadata from Flash
 * @retval 1: valid metadata
 * @retval 0: invalid metadata
 */
static uint8_t APP_Meta_Read(APP_MetaInfo_t *info)
{
    if (info == 0)
    {
        return 0;
    }

    info->magic         = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 0U);
    info->status        = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 4U);
    info->active_slot   = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 8U);
    info->target_slot   = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 12U);
    info->rollback_slot = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 16U);
    info->boot_pending  = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 20U);
    info->confirmed     = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 24U);
    info->size          = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 28U);
    info->crc32         = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 32U);
    info->version       = *(volatile uint32_t *)(APP_META_FLASH_START_ADDR + 36U);

    if (info->magic != APP_META_MAGIC)
    {
        return 0;
    }

    return 1;
}

/**
 * @brief Program one 32-bit word into metadata area
 */
static HAL_StatusTypeDef APP_Meta_WriteWord(uint32_t addr, uint32_t value)
{
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);
}

/**
 * @brief Rewrite metadata page with confirmed boot result
 */
static void APP_Meta_WriteConfirmed(const APP_MetaInfo_t *info)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = APP_META_FLASH_START_ADDR;
    erase_init.NbPages = 1;

    HAL_FLASHEx_Erase(&erase_init, &page_error);

    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 0U,  info->magic);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 4U,  info->status);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 8U,  info->active_slot);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 12U, info->target_slot);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 16U, info->rollback_slot);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 20U, info->boot_pending);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 24U, info->confirmed);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 28U, info->size);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 32U, info->crc32);
    APP_Meta_WriteWord(APP_META_FLASH_START_ADDR + 36U, info->version);

    HAL_FLASH_Lock();
}

/**
 * @brief Confirm boot success when current image is the testing image
 *
 * If metadata shows:
 * - status == TESTING
 * - active_slot == APP_SELF_SLOT
 * - confirmed == 0
 *
 * then the application rewrites metadata as:
 * - status = OK
 * - boot_pending = 0
 * - confirmed = 1
 */
void APP_ConfirmBootIfNeeded(void)
{
    APP_MetaInfo_t meta;

    /* Return directly if metadata is invalid */
    if (!APP_Meta_Read(&meta))
    {
        return;
    }

    /* Confirm boot only for the current testing image */
    if ((meta.status == APP_META_STATUS_TESTING) &&
        (meta.active_slot == APP_SELF_SLOT) &&
        (meta.confirmed == 0U))
    {
        /* Mark current image as confirmed and stable */
        meta.status = APP_META_STATUS_OK;
        meta.boot_pending = 0U;
        meta.confirmed = 1U;

        APP_Meta_WriteConfirmed(&meta);
    }
}
