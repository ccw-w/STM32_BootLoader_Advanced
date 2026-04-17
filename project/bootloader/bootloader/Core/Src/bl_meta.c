/**
 * @file    bl_meta.c
 * @brief   Bootloader 元信息读写实现
 *
 * Meta 区用于保存：
 * - 升级状态
 * - 活动槽 / 目标槽 / 回滚槽
 * - 首启与确认标志
 * - 固件大小 / CRC / 版本号
 */
#include "bl_meta.h"
#include "bl_config.h"

/**
 * @brief 向元信息区域写入一个 32 位字
 */
static HAL_StatusTypeDef BL_Meta_WriteWord(uint32_t addr, uint32_t value) {
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);
}

/**
 * @brief 擦除元信息页并写入一整份元信息
 */
void BL_Meta_Set(BL_MetaStatus_t status, BL_Slot_t active_slot,
                 BL_Slot_t target_slot, BL_Slot_t rollback_slot,
                 uint32_t boot_pending, uint32_t confirmed, uint32_t size,
                 uint32_t crc32, uint32_t version) {
  FLASH_EraseInitTypeDef erase_init = {0};
  uint32_t page_error = 0;

  HAL_FLASH_Unlock();

  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.PageAddress = META_FLASH_START_ADDR;
  erase_init.NbPages = 1;

  HAL_FLASHEx_Erase(&erase_init, &page_error);

  BL_Meta_WriteWord(META_FLASH_START_ADDR + 0U, BL_META_MAGIC);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 4U, (uint32_t)status);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 8U, (uint32_t)active_slot);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 12U, (uint32_t)target_slot);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 16U, (uint32_t)rollback_slot);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 20U, boot_pending);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 24U, confirmed);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 28U, size);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 32U, crc32);
  BL_Meta_WriteWord(META_FLASH_START_ADDR + 36U, version);

  HAL_FLASH_Lock();
}

/**
 * @brief 清空元信息页
 */
void BL_Meta_Clear(void) {
  FLASH_EraseInitTypeDef erase_init = {0};
  uint32_t page_error = 0;

  HAL_FLASH_Unlock();

  erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  erase_init.PageAddress = META_FLASH_START_ADDR;
  erase_init.NbPages = 1;

  HAL_FLASHEx_Erase(&erase_init, &page_error);

  HAL_FLASH_Lock();
}

/**
 * @brief 读取并检查元信息是否有效
 * @retval 1 表示有效
 * @retval 0 表示无效
 */
uint8_t BL_Meta_Read(BL_MetaInfo_t *info) {
  if (info == 0) {
    return 0;
  }

  info->magic = *(volatile uint32_t *)(META_FLASH_START_ADDR + 0U);
  info->status = *(volatile uint32_t *)(META_FLASH_START_ADDR + 4U);
  info->active_slot = *(volatile uint32_t *)(META_FLASH_START_ADDR + 8U);
  info->target_slot = *(volatile uint32_t *)(META_FLASH_START_ADDR + 12U);
  info->rollback_slot = *(volatile uint32_t *)(META_FLASH_START_ADDR + 16U);
  info->boot_pending = *(volatile uint32_t *)(META_FLASH_START_ADDR + 20U);
  info->confirmed = *(volatile uint32_t *)(META_FLASH_START_ADDR + 24U);
  info->size = *(volatile uint32_t *)(META_FLASH_START_ADDR + 28U);
  info->crc32 = *(volatile uint32_t *)(META_FLASH_START_ADDR + 32U);
  info->version = *(volatile uint32_t *)(META_FLASH_START_ADDR + 36U);

  if (info->magic != BL_META_MAGIC) {
    return 0;
  }

  return 1;
}
