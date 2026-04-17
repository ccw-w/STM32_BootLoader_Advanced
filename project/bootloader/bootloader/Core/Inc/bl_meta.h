/**
 * @file    bl_meta.h
 * @brief   Bootloader 元信息定义，包含启动状态、槽位选择和回滚信息
 */
#ifndef __BL_META_H__
#define __BL_META_H__

#include "stm32f1xx_hal.h"

#define BL_META_MAGIC 0x4D455441U /* 'META' */

typedef enum {
  BL_META_STATUS_IDLE = 0,     /* 空闲状态 */
  BL_META_STATUS_UPDATING = 1, /* 正在升级 */
  BL_META_STATUS_OK = 2,       /* 当前镜像已确认可正常运行 */
  BL_META_STATUS_ERROR = 3,    /* 升级失败 */
  BL_META_STATUS_TESTING = 4   /* 新镜像处于首启测试状态 */
} BL_MetaStatus_t;

typedef enum {
  BL_SLOT_NONE = 0, /* 无效槽 */
  BL_SLOT_A = 'A',  /* A 槽 */
  BL_SLOT_B = 'B'   /* B 槽 */
} BL_Slot_t;

typedef struct {
  uint32_t magic;         /* 元信息魔术字 */
  uint32_t status;        /* 当前启动/升级状态 */
  uint32_t active_slot;   /* 当前活动槽 */
  uint32_t target_slot;   /* 下一次升级目标槽 */
  uint32_t rollback_slot; /* 回滚槽 */
  uint32_t boot_pending;  /* 首启标志：1=尚未试启动，0=已试启动 */
  uint32_t confirmed;     /* 确认标志：1=已确认，0=未确认 */
  uint32_t size;          /* 固件大小 */
  uint32_t crc32;         /* 固件 CRC32 */
  uint32_t version;       /* 固件版本号 */
} BL_MetaInfo_t;

/**
 * @brief 将完整元信息写入 Flash
 */
void BL_Meta_Set(BL_MetaStatus_t status, BL_Slot_t active_slot,
                 BL_Slot_t target_slot, BL_Slot_t rollback_slot,
                 uint32_t boot_pending, uint32_t confirmed, uint32_t size,
                 uint32_t crc32, uint32_t version);

/**
 * @brief 清空元信息区域
 */
void BL_Meta_Clear(void);

/**
 * @brief 从 Flash 读取元信息
 * @retval 1 表示有效，0 表示无效
 */
uint8_t BL_Meta_Read(BL_MetaInfo_t *info);

#endif
