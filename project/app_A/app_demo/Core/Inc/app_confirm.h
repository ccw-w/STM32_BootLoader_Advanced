/**
 * @file    app_confirm.h
 * @brief   Application-side boot confirmation interface
 */
#ifndef __APP_CONFIRM_H__
#define __APP_CONFIRM_H__

#include "main.h"

#define APP_META_MAGIC          0x4D455441U   /* 'META' */

#define APP_META_STATUS_IDLE     0U
#define APP_META_STATUS_UPDATING 1U
#define APP_META_STATUS_OK       2U
#define APP_META_STATUS_ERROR    3U
#define APP_META_STATUS_TESTING  4U

#define APP_SLOT_NONE  0U
#define APP_SLOT_A     'A'
#define APP_SLOT_B     'B'

#define APP_META_FLASH_START_ADDR  0x0801C000U

typedef struct
{
    uint32_t magic;         /* metadata magic */
    uint32_t status;        /* boot/update status */
    uint32_t active_slot;   /* current active slot */
    uint32_t target_slot;   /* next target slot */
    uint32_t rollback_slot; /* rollback slot */
    uint32_t boot_pending;  /* first boot flag */
    uint32_t confirmed;     /* boot confirmed flag */
    uint32_t size;          /* firmware size */
    uint32_t crc32;         /* firmware CRC32 */
    uint32_t version;       /* firmware version */
} APP_MetaInfo_t;

/* This macro must match the current application build target */
#define APP_SELF_SLOT   APP_SLOT_A

/**
 * @brief Confirm boot success if current image is under TESTING state
 */
void APP_ConfirmBootIfNeeded(void);

#endif
