#include "bootloader.h"
#include "bl_jump.h"
#include "bl_mode.h"
#include "bl_protocol.h"
#include "bl_state.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

static BL_Context_t g_bl_ctx;
static BL_FirmwareHeader_t g_fw_header; // 后面收到16个字节后解析出固件头结果
static uint8_t g_header_buf[sizeof(BL_FirmwareHeader_t)]; // 接收固件头的缓冲区
static uint8_t g_header_received = 0; // 已经接收的固件头字节数

static void
BL_LoadTestHeader(void); // 测试函数，加载一个假的固件头用于调试，后面删除
/**
 * @brief 测试函数，加载一个假的固件头用于调试，后面删除
 */
static void BL_LoadTestHeader(void) {
  g_fw_header.magic = BL_FW_MAGIC;
  g_fw_header.size = 4096;
  g_fw_header.crc32 = 0x12345678;
  g_fw_header.version = 1;
}

void Bootloader_Run(void) {
  memset(&g_fw_header, 0, sizeof(g_fw_header)); // 固件头清零
  BL_State_Init(&g_bl_ctx); // 初始化状态机为空闲状态，已经初始化则return

  printf("==== Bootloader Start ====\r\n");
  printf("BL start: 0x%08lX\r\n", BL_FLASH_START_ADDR);
  printf("APP addr: 0x%08lX\r\n", APP_FLASH_START_ADDR);

  HAL_Delay(BL_JUMP_DELAY_MS);

  /* 当前阶段：实现简单 标志位(1进入，0不进入) 升级模式入口判断 */
  if (BL_ShouldEnterUpdateMode()) {
    printf("Enter update mode.\r\n");
    g_bl_ctx.state = BL_STATE_WAIT_HEADER;
  } else if (BL_IsAppValid(APP_FLASH_START_ADDR)) {
    printf("App valid, jump now...\r\n");

    // 等待 UART 发送完成，避免跳转后数据丢失
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
      /* 这里等接收固件头 */
      printf("Waiting firmware header...\r\n");
      BL_LoadTestHeader(); // 测试函数，加载一个假的固件头用于调试，后面删除

      if (BL_CheckFirmwareHeader(&g_fw_header)) {
        printf("Header OK: size=%lu, crc=0x%08lX, version=%lu\r\n",
               g_fw_header.size, g_fw_header.crc32, g_fw_header.version);

        g_bl_ctx.expected_size = g_fw_header.size;
        g_bl_ctx.expected_crc32 = g_fw_header.crc32;
        g_bl_ctx.version = g_fw_header.version;
        g_bl_ctx.recv_size = 0;

        g_bl_ctx.state = BL_STATE_RECV_DATA;
      } else {
        printf("Header invalid.\r\n");
        g_bl_ctx.state = BL_STATE_ERROR;
      }
      break;

    case BL_STATE_RECV_DATA:
      /* 这里接收固件数据 */
      printf("Recv data stage. expected_size=%lu\r\n", g_bl_ctx.expected_size);
      g_bl_ctx.state = BL_STATE_VERIFY_CRC;
      break;

    case BL_STATE_VERIFY_CRC:
      /* 这里做 CRC 校验 */
      printf("Verify CRC stage. expected_crc=0x%08lX\r\n",
             g_bl_ctx.expected_crc32);
      g_bl_ctx.state = BL_STATE_PROGRAM_DONE;
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
