#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__

/* 最小配置文件 */

#include "stm32f1xx_hal.h"

/* Flash 分区 */
#define BL_FLASH_START_ADDR      0x08000000U
#define BL_FLASH_SIZE            0x00004000U   /* 16KB */

#define APP_FLASH_START_ADDR     0x08004000U
#define APP_FLASH_SIZE           0x0001A000U   /* 104KB */

#define META_FLASH_START_ADDR    0x0801E000U
#define META_FLASH_SIZE          0x00002000U   /* 8KB */

/* Boot 延迟多久跳转 */
#define BL_JUMP_DELAY_MS         1000U

#endif
