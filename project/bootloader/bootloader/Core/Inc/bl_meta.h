#ifndef __BL_META_H__
#define __BL_META_H__

#include "stm32f1xx_hal.h"

#define BL_META_MAGIC   0x4D455441U   /* 'META' */

typedef enum
{
    BL_META_STATUS_IDLE = 0,
    BL_META_STATUS_UPDATING = 1,
    BL_META_STATUS_OK = 2,
    BL_META_STATUS_ERROR = 3,
    BL_META_STATUS_TESTING = 4
} BL_MetaStatus_t;

typedef enum
{
    BL_SLOT_NONE = 0,
    BL_SLOT_A = 'A',
    BL_SLOT_B = 'B'
} BL_Slot_t;

typedef struct
{
    uint32_t magic;          /* 固定标志 */
    uint32_t status;         /* 升级状态 */
    uint32_t active_slot;    /* 当前活动槽 */
    uint32_t target_slot;    /* 下次升级目标槽 */
    uint32_t rollback_slot;  /* 回滚槽 */
    uint32_t boot_pending;   /* 1=还没试启动新镜像，0=已经试过一次 */
    uint32_t confirmed;      /* 0=未确认，1=已确认 */
    uint32_t size;           /* 固件大小 */
    uint32_t crc32;          /* 固件 CRC32 */
    uint32_t version;        /* 固件版本 */
} BL_MetaInfo_t;

void BL_Meta_Set(BL_MetaStatus_t status,
                 BL_Slot_t active_slot,
                 BL_Slot_t target_slot,
                 BL_Slot_t rollback_slot,
                 uint32_t boot_pending,
                 uint32_t confirmed,
                 uint32_t size,
                 uint32_t crc32,
                 uint32_t version);

void BL_Meta_Clear(void);
uint8_t BL_Meta_Read(BL_MetaInfo_t *info);

#endif
