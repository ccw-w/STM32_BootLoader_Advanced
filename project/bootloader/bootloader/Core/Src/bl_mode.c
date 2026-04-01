#include "bl_mode.h"

/*
 * 当前先用最简单策略：
 * 0 = 不主动进入升级模式
 * 后面再改成按键 / 串口命令 / Flash标志位
 */
uint8_t BL_ShouldEnterUpdateMode(void)
{
    return 1;
}
