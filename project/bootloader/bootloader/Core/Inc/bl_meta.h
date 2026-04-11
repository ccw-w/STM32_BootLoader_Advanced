#ifndef __BL_META_H__
#define __BL_META_H__

#include "stm32f1xx_hal.h"

#define BL_META_MAGIC   0x4D455441U   /* 'META' */

typedef enum
{
    BL_META_STATUS_IDLE = 0,
    BL_META_STATUS_UPDATING = 1,
    BL_META_STATUS_OK = 2,
    BL_META_STATUS_ERROR = 3
} BL_MetaStatus_t;

typedef struct
{
    uint32_t magic;
    uint32_t status;
    uint32_t size;
    uint32_t crc32;
    uint32_t version;
} BL_MetaInfo_t;

void BL_Meta_Set(BL_MetaStatus_t status, uint32_t size, uint32_t crc32, uint32_t version);
void BL_Meta_Clear(void);
uint8_t BL_Meta_Read(BL_MetaInfo_t *info);


#endif
