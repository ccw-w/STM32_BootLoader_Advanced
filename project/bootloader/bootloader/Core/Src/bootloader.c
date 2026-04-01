#include "bootloader.h"
#include "bl_jump.h"
#include "bl_protocol.h"
#include "bl_state.h"
#include "usart.h"
#include <stdio.h>


static BL_Context_t g_bl_ctx;

void Bootloader_Run(void) {
  BL_State_Init(&g_bl_ctx); // 初始化状态机为空闲状态，已经初始化则return

  printf("==== Bootloader Start ====\r\n");
  printf("BL start: 0x%08lX\r\n", BL_FLASH_START_ADDR);
  printf("APP addr: 0x%08lX\r\n", APP_FLASH_START_ADDR);

  HAL_Delay(BL_JUMP_DELAY_MS);

  /* 当前阶段：判断App分区有没有可以启动的程序，有就直接跳
   * App，没有打印输出修改状态 */
  if (BL_IsAppValid(APP_FLASH_START_ADDR)) {
    printf("App valid, jump now...\r\n");

    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET) {
    }

    BL_JumpToApp(APP_FLASH_START_ADDR);
  } else {
    printf("No valid app found, stay in bootloader.\r\n");
    g_bl_ctx.state = BL_STATE_WAIT_HEADER;
  }

  while (1) {
    switch (g_bl_ctx.state) {
    case BL_STATE_WAIT_HEADER:
      /* 后面这里等接收固件头 */
      break;

    case BL_STATE_RECV_DATA:
      /* 后面这里接收固件数据 */
      break;

    case BL_STATE_VERIFY_CRC:
      /* 后面这里做 CRC 校验 */
      break;

    case BL_STATE_PROGRAM_DONE:
      printf("Program done.\r\n");
      g_bl_ctx.state = BL_STATE_IDLE;
      break;

    case BL_STATE_ERROR:
      printf("Bootloader error.\r\n");
      g_bl_ctx.state = BL_STATE_IDLE;
      break;

    case BL_STATE_IDLE:
    default:
      HAL_Delay(100);
      break;
    }
  }
}
