/**
 * @file    app_confirm.c
 * @brief   应用侧首启确认实现
 *
 * 本模块用于在新镜像首启成功后，
 * 由应用主动回写 Meta，确认该镜像可以稳定运行。
 */
#include "app_confirm.h"

/**
 * @brief 从 Flash 读取元信息
 * @retval 1 表示元信息有效
 * @retval 0 表示元信息无效
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
 * @brief 向元信息区域写入一个 32 位字
 */
static HAL_StatusTypeDef APP_Meta_WriteWord(uint32_t addr, uint32_t value)
{
    return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);
}

/**
 * @brief 擦除并重写元信息页，保存确认后的状态
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
 * @brief 如有需要，确认当前测试镜像启动成功
 *
 * 当满足以下条件时：
 * - 当前状态为 TESTING
 * - 当前活动槽就是本应用所在槽
 * - confirmed == 0
 *
 * 则将元信息更新为：
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

    /* 仅对当前测试镜像执行启动确认 */
    if ((meta.status == APP_META_STATUS_TESTING) &&
        (meta.active_slot == APP_SELF_SLOT) &&
        (meta.confirmed == 0U))
    {
        /* 将当前镜像标记为已确认、可稳定运行 */
        meta.status = APP_META_STATUS_OK;
        meta.boot_pending = 0U;
        meta.confirmed = 1U;

        APP_Meta_WriteConfirmed(&meta);
    }
}
