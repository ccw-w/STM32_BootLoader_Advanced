/**
 * @file    app_confirm.h
 * @brief   应用侧首启确认接口
 */
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
    uint32_t magic;         /* 元信息魔术字 */
    uint32_t status;        /* 当前启动/升级状态 */
    uint32_t active_slot;   /* 当前活动槽 */
    uint32_t target_slot;   /* 下一次升级目标槽 */
    uint32_t rollback_slot; /* 回滚槽 */
    uint32_t boot_pending;  /* 首启标志 */
    uint32_t confirmed;     /* 启动确认标志 */
    uint32_t size;          /* 固件大小 */
    uint32_t crc32;         /* 固件 CRC32 */
    uint32_t version;       /* 固件版本号 */
} APP_MetaInfo_t;

/* 该宏必须与当前应用工程对应的槽位保持一致 */
#define APP_SELF_SLOT   APP_SLOT_B

/**
 * @brief 如有需要，确认当前测试镜像启动成功
 */
void APP_ConfirmBootIfNeeded(void);

#endif
