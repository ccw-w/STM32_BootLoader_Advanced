#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__

#include "stm32f1xx_hal.h"

/* =========================
 * Flash layout for 128KB
 * =========================
 *
 * 0x08000000 ~ 0x08003FFF   Bootloader   16KB
 * 0x08004000 ~ 0x0800FFFF   Slot A       48KB
 * 0x08010000 ~ 0x0801BFFF   Slot B       48KB
 * 0x0801C000 ~ 0x0801CFFF   Meta         4KB
 * 0x0801D000 ~ 0x0801FFFF   Reserved    12KB
 */

/* Bootloader */
#define BL_FLASH_START_ADDR      0x08000000U
#define BL_FLASH_SIZE            0x00004000U   /* 16KB */

/* Slot A */
#define APP_SLOT_A_ADDR          0x08004000U
#define APP_SLOT_A_SIZE          0x0000C000U   /* 48KB */

/* Slot B */
#define APP_SLOT_B_ADDR          0x08010000U
#define APP_SLOT_B_SIZE          0x0000C000U   /* 48KB */

/* 当前先默认从 A 启动，后面再做动态切换 */
#define APP_FLASH_START_ADDR     APP_SLOT_A_ADDR
#define APP_FLASH_SIZE           APP_SLOT_A_SIZE

/* Meta */
#define META_FLASH_START_ADDR    0x0801C000U
#define META_FLASH_SIZE          0x00001000U   /* 4KB */

/* Reserved */
#define RESERVED_FLASH_START_ADDR 0x0801D000U
#define RESERVED_FLASH_SIZE       0x00003000U  /* 12KB */

/* Boot delay */
#define BL_JUMP_DELAY_MS         1000U

#endif
