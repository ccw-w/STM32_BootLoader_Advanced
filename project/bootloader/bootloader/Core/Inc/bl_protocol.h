#ifndef __BL_PROTOCOL_H__
#define __BL_PROTOCOL_H__

/* 协议头 */

#include "stm32f1xx_hal.h"

#define BL_FW_MAGIC      0x4657424CU   /* 'FWBL' */

typedef struct
{
    uint32_t magic;      /* 固定标志，判断是不是合法升级包 */
    uint32_t size;       /* 固件实际大小，单位：字节 */
    uint32_t crc32;      /* 整包 CRC32 */
    uint32_t version;    /* 固件版本号 */
} BL_FirmwareHeader_t;

uint8_t BL_CheckFirmwareHeader(const BL_FirmwareHeader_t *header);

#endif
