#ifndef __APP_CONFIRM_H__
#define __APP_CONFIRM_H__

#include "main.h"

/* 和 bootloader 保持一致 */
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
    uint32_t magic;
    uint32_t status;
    uint32_t active_slot;
    uint32_t target_slot;
    uint32_t rollback_slot;
    uint32_t boot_pending;
    uint32_t confirmed;
    uint32_t size;
    uint32_t crc32;
    uint32_t version;
} APP_MetaInfo_t;

/* B工程 */
#define APP_SELF_SLOT   APP_SLOT_B

void APP_ConfirmBootIfNeeded(void);

#endif
