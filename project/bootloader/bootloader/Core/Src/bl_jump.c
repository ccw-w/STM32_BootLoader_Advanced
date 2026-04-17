/**
 * @file    bl_jump.c
 * @brief   应用镜像有效性检查与跳转实现
 *
 * 主要职责：
 * - 检查栈顶和复位向量是否合法
 * - 切换中断向量表到目标应用
 * - 设置 MSP
 * - 跳转到应用复位入口
 */
#include "bl_jump.h"
#include "usart.h"

typedef void (*pFunction)(void);

/**
 * @brief 检查指定应用镜像是否有效
 *
 * 检查规则：
 * - 初始栈顶地址必须位于 SRAM 区间
 * - 复位向量不能是空值
 * - 复位向量必须落在当前槽地址范围内
 */
uint8_t BL_IsAppValid(uint32_t app_addr, uint32_t app_size) {
  uint32_t app_sp;
  uint32_t app_reset;

  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);

  if ((app_sp < 0x20000000U) || (app_sp > 0x20005000U)) {
    return 0;
  }

  if ((app_reset == 0xFFFFFFFFU) || (app_reset == 0x00000000U)) {
    return 0;
  }

  if ((app_reset < app_addr) || (app_reset >= (app_addr + app_size))) {
    return 0;
  }

  return 1;
}

/**
 * @brief 从 Bootloader 跳转到应用程序
 *
 * 跳转步骤：
 * - 检查目标应用是否有效
 * - 关闭中断
 * - 停止 SysTick
 * - 反初始化 HAL
 * - 清除 NVIC 使能与挂起状态
 * - 切换 VTOR
 * - 设置 MSP
 * - 跳转到复位向量
 */
void BL_JumpToApp(uint32_t app_addr, uint32_t app_size) {
  uint32_t app_sp;
  uint32_t app_reset;
  pFunction app_entry;

  /* 检查目标应用是否有效 */
  if (BL_IsAppValid(app_addr, app_size) == 0U) {
    printf("Invalid app in slot: addr=0x%08lX\r\n", app_addr);
    return;
  }

  /* 读取应用向量表中的栈顶地址和复位入口 */
  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);
  app_entry = (pFunction)app_reset;

  /* 关闭中断并停止 SysTick */
  __disable_irq();
  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;

  HAL_DeInit();

  /* 清除 NVIC 使能和挂起状态 */
  NVIC->ICER[0] = 0xFFFFFFFFU;
  NVIC->ICER[1] = 0xFFFFFFFFU;
  NVIC->ICPR[0] = 0xFFFFFFFFU;
  NVIC->ICPR[1] = 0xFFFFFFFFU;

  /* 切换向量表并设置应用栈顶 */
  SCB->VTOR = app_addr;
  __set_MSP(app_sp);

  __DSB();
  __ISB();

  /* 跳转到应用复位入口 */
  app_entry();

  while (1) {
  }
}
