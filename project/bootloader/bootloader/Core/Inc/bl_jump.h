#ifndef __BL_JUMP_H__
#define __BL_JUMP_H__

/* 跳转头文件 */

#include "bl_config.h"

uint8_t BL_IsAppValid(uint32_t app_addr);
void BL_JumpToApp(uint32_t app_addr);

#endif
