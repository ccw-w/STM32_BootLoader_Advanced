/**
 * @file    bl_jump.c
 * @brief   Validate application image and jump to target application
 *
 * Main responsibilities:
 * - check whether stack pointer and reset handler are valid
 * - switch vector table to target application
 * - set MSP to application stack top
 * - jump to application reset handler
 */
#include "bl_jump.h"
#include "usart.h"

typedef void (*pFunction)(void);

/**
 * @brief Check whether an application image is valid
 *
 * Validation rules:
 * - initial stack pointer must be inside SRAM
 * - reset handler must not be empty
 * - reset handler must be inside current slot range
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
 * @brief Jump from Bootloader to application
 *
 * Steps:
 * - validate target application
 * - disable interrupts
 * - stop SysTick
 * - de-initialize HAL
 * - clear NVIC enable/pending bits
 * - switch VTOR
 * - load MSP
 * - jump to reset handler
 */
void BL_JumpToApp(uint32_t app_addr, uint32_t app_size) {
  uint32_t app_sp;
  uint32_t app_reset;
  pFunction app_entry;

  /* Check whether target application is valid */
  if (BL_IsAppValid(app_addr, app_size) == 0U) {
    printf("Invalid app in slot: addr=0x%08lX\r\n", app_addr);
    return;
  }

  /* Read initial stack pointer and reset handler from vector table */
  app_sp = *(volatile uint32_t *)app_addr;
  app_reset = *(volatile uint32_t *)(app_addr + 4U);
  app_entry = (pFunction)app_reset;

  /* Disable interrupts and stop SysTick */
  __disable_irq();
  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;

  HAL_DeInit();

  /* Clear NVIC enable and pending state */
  NVIC->ICER[0] = 0xFFFFFFFFU;
  NVIC->ICER[1] = 0xFFFFFFFFU;
  NVIC->ICPR[0] = 0xFFFFFFFFU;
  NVIC->ICPR[1] = 0xFFFFFFFFU;

  /* Switch vector table and set application MSP */
  SCB->VTOR = app_addr;
  __set_MSP(app_sp);

  __DSB();
  __ISB();

  /* Jump to application reset handler */
  app_entry();

  while (1) {
  }
}
