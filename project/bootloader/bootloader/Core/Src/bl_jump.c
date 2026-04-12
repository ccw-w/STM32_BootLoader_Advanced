#include "bl_jump.h"
#include "usart.h"

typedef void (*pFunction)(void);

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

void BL_JumpToApp(uint32_t app_addr, uint32_t app_size) {
  uint32_t app_sp;
  uint32_t app_reset;
  pFunction app_entry;

  if (BL_IsAppValid(app_addr, app_size) == 0U) {
    printf("Invalid app in slot: addr=0x%08lX\r\n", app_addr);
    return;
  }

  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);
  app_entry = (pFunction)app_reset;

  __disable_irq();

  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;

  HAL_DeInit();

  NVIC->ICER[0] = 0xFFFFFFFFU;
  NVIC->ICER[1] = 0xFFFFFFFFU;
  NVIC->ICPR[0] = 0xFFFFFFFFU;
  NVIC->ICPR[1] = 0xFFFFFFFFU;

  SCB->VTOR = app_addr;
  __set_MSP(app_sp);

  __DSB();
  __ISB();

  app_entry();

  while (1) {
  }
}
