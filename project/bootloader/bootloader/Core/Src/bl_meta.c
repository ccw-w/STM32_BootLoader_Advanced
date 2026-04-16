/**
 * @file    bl_meta.c
 * @brief   Metadata read/write implementation
 *
 * The metadata area is used to store:
 * - upgrade status
 * - active / target / rollback slot
 * - boot confirmation flags
 * - firmware size / CRC / version
 */
#include "bl_meta.h"
#include "bl_config.h"

/**
 * @brief Program one 32-bit word into metadata area
 */
static HAL_StatusTypeDef BL_Meta_WriteWord(uint32_t addr, uint32_t value) {
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, value);
}

/**
 * @brief Erase metadata page and write a full metadata record
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
 * @brief Clear metadata page
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
 * @brief Read and validate metadata from Flash
 * @retval 1: metadata is valid
 * @retval 0: metadata is invalid
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
