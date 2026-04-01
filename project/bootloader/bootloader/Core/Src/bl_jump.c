#include "bl_jump.h"
#include "usart.h"

typedef void (*pFunction)(void);

uint8_t BL_IsAppValid(uint32_t app_addr) {
  uint32_t app_sp;
  uint32_t app_reset;

  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);

  /* 1. 栈顶地址必须落在 SRAM 区间 */
  if ((app_sp < 0x20000000U) || (app_sp > 0x20005000U)) {
    return 0;
  }

  /* 2. 复位向量不能是空白 */
  if ((app_reset == 0xFFFFFFFFU) || (app_reset == 0x00000000U)) {
    return 0;
  }

  /* 3. 复位向量最好落在 App 分区范围内 */
  if ((app_reset < APP_FLASH_START_ADDR) ||
      (app_reset > (APP_FLASH_START_ADDR + APP_FLASH_SIZE))) {
    return 0;
  }

  return 1;
}

void BL_JumpToApp(uint32_t app_addr) {
  uint32_t app_sp;
  uint32_t app_reset;
  pFunction app_entry;

  if (BL_IsAppValid(app_addr) == 0U) {
    return;
  }

  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);
  app_entry = (pFunction)app_reset;

  __disable_irq();

  /* 关闭 SysTick */
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  /* 关闭所有中断，清 pending */
  for (uint32_t i = 0; i < 8; i++) {
    NVIC->ICER[i] = 0xFFFFFFFFU;
    NVIC->ICPR[i] = 0xFFFFFFFFU;
  }

  /* 反初始化 HAL 外设 */
  HAL_UART_DeInit(&huart1);
  HAL_RCC_DeInit();
  HAL_DeInit();

  /* 切中断向量表到 App */
  SCB->VTOR = app_addr;

  /* 设置主栈指针 */
  __set_MSP(app_sp);

  __DSB();
  __ISB();

  /* 跳转到 App Reset_Handler */
  app_entry();

  while (1) {
  }
}
