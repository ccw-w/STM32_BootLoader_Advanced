#ifndef __BL_STATE_H__
#define __BL_STATE_H__

#include "stm32f1xx_hal.h"

/* 升级状态机 */

typedef enum
{
    BL_STATE_IDLE = 0,            /* 空闲 */
    BL_STATE_WAIT_HEADER,         /* 等待固件头 */
    BL_STATE_RECV_DATA,           /* 接收固件数据 */
    BL_STATE_VERIFY_CRC,          /* 校验整包 CRC */
    BL_STATE_PROGRAM_DONE,        /* 升级完成 */
    BL_STATE_ERROR                /* 出错 */
} BL_State_t;

typedef struct
{
    BL_State_t state;
    uint32_t recv_size;           /* 已接收字节数 */
    uint32_t expected_size;       /* 期望接收总字节数 */
    uint32_t expected_crc32;      /* 期望 CRC32 */
    uint32_t version;             /* 固件版本 */
} BL_Context_t;

void BL_State_Init(BL_Context_t *ctx);

#endif
