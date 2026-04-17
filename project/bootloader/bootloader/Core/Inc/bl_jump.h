/**
 * @file    bl_jump.h
 * @brief   应用有效性检查与跳转接口
 */
#ifndef __BL_JUMP_H__
#define __BL_JUMP_H__

#include "bl_config.h"

/**
 * @brief 检查指定地址处的应用镜像是否有效
 * @param app_addr 应用起始地址
 * @param app_size 应用槽大小
 * @retval 1 表示有效
 * @retval 0 表示无效
 */
uint8_t BL_IsAppValid(uint32_t app_addr, uint32_t app_size);

/**
 * @brief 从 Bootloader 跳转到应用程序
 * @param app_addr 应用起始地址
 * @param app_size 应用槽大小
 */
void BL_JumpToApp(uint32_t app_addr, uint32_t app_size);

#endif
