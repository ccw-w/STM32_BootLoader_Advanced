/**
 * @file    bl_meta.h
 * @brief   Metadata definition for boot status, slot selection, and rollback
 */
#ifndef __BL_META_H__
#define __BL_META_H__

#include "stm32f1xx_hal.h"

#define BL_META_MAGIC 0x4D455441U /* 'META' */

typedef enum {
  BL_META_STATUS_IDLE = 0,     /* idle state */
  BL_META_STATUS_UPDATING = 1, /* firmware is being updated */
  BL_META_STATUS_OK = 2,       /* current image is confirmed and valid */
  BL_META_STATUS_ERROR = 3,    /* update failed */
  BL_META_STATUS_TESTING = 4   /* new image is under first-boot test */
} BL_MetaStatus_t;

typedef enum {
  BL_SLOT_NONE = 0, /* invalid slot */
  BL_SLOT_A = 'A',  /* slot A */
  BL_SLOT_B = 'B'   /* slot B */
} BL_Slot_t;

typedef struct {
  uint32_t magic;         /* metadata magic */
  uint32_t status;        /* current boot/update status */
  uint32_t active_slot;   /* slot selected for current boot */
  uint32_t target_slot;   /* slot to receive next firmware image */
  uint32_t rollback_slot; /* previous slot used for rollback */
  uint32_t boot_pending;  /* 1: first boot not tried yet, 0: already tried */
  uint32_t confirmed;     /* 1: image confirmed, 0: not confirmed */
  uint32_t size;          /* firmware size in bytes */
  uint32_t crc32;         /* firmware CRC32 */
  uint32_t version;       /* firmware version */
} BL_MetaInfo_t;

/**
 * @brief Write a complete metadata record into Flash
 */
void BL_Meta_Set(BL_MetaStatus_t status, BL_Slot_t active_slot,
                 BL_Slot_t target_slot, BL_Slot_t rollback_slot,
                 uint32_t boot_pending, uint32_t confirmed, uint32_t size,
                 uint32_t crc32, uint32_t version);

/**
 * @brief Erase metadata area
 */
void BL_Meta_Clear(void);

/**
 * @brief Read metadata from Flash
 * @retval 1: valid metadata, 0: invalid metadata
 */
uint8_t BL_Meta_Read(BL_MetaInfo_t *info);

#endif
