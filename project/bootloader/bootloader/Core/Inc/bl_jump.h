/**
 * @file    bl_jump.h
 * @brief   Application validation and jump interface
 */
#ifndef __BL_JUMP_H__
#define __BL_JUMP_H__

#include "bl_config.h"

/**
 * @brief Check whether an application image is valid
 * @param app_addr Application start address
 * @param app_size Application slot size
 * @retval 1: valid
 * @retval 0: invalid
 */
uint8_t BL_IsAppValid(uint32_t app_addr, uint32_t app_size);

/**
 * @brief Jump from Bootloader to application
 * @param app_addr Application start address
 * @param app_size Application slot size
 */
void BL_JumpToApp(uint32_t app_addr, uint32_t app_size);

#endif
