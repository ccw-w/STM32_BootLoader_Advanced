/**
 * @file    bl_config.h
 * @brief   Bootloader flash layout and common configuration
 */
#ifndef __BL_CONFIG_H__
#define __BL_CONFIG_H__

#include "stm32f1xx_hal.h"

/* =========================
 * Flash layout (128 KB)
 * =========================
 *
 * 0x08000000 ~ 0x08003FFF   Bootloader   16 KB
 * 0x08004000 ~ 0x0800FFFF   Slot A       48 KB
 * 0x08010000 ~ 0x0801BFFF   Slot B       48 KB
 * 0x0801C000 ~ 0x0801CFFF   Meta         4 KB
 * 0x0801D000 ~ 0x0801FFFF   Reserved    12 KB
 */

/* Bootloader area */
#define BL_FLASH_START_ADDR 0x08000000U
#define BL_FLASH_SIZE 0x00004000U /* 16 KB */

/* Application Slot A */
#define APP_SLOT_A_ADDR 0x08004000U
#define APP_SLOT_A_SIZE 0x0000C000U /* 48 KB */

/* Application Slot B */
#define APP_SLOT_B_ADDR 0x08010000U
#define APP_SLOT_B_SIZE 0x0000C000U /* 48 KB */

/* Default app start definition (kept for compatibility) */
#define APP_FLASH_START_ADDR APP_SLOT_A_ADDR
#define APP_FLASH_SIZE APP_SLOT_A_SIZE

/* Metadata area */
#define META_FLASH_START_ADDR 0x0801C000U
#define META_FLASH_SIZE 0x00001000U /* 4 KB */

/* Reserved area */
#define RESERVED_FLASH_START_ADDR 0x0801D000U
#define RESERVED_FLASH_SIZE 0x00003000U /* 12 KB */

/* Boot delay before decision */
#define BL_JUMP_DELAY_MS 1000U

#endif
