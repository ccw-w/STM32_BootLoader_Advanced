#include "bootloader.h"
#include "bl_crc.h"
#include "bl_flash.h"
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
static uint8_t g_fw_data_buf[256];    // 固件数据接收缓冲区，测试先接收256字节
static uint32_t g_fw_data_len = 0;    // 实际收到了多少字节
static uint32_t g_calc_crc32 = 0;     // 已经接收数据的 CRC32

static uint8_t BL_ReceiveHeader(BL_FirmwareHeader_t *header);
static void BL_PrintHeader(const BL_FirmwareHeader_t *header);
static uint8_t BL_ReceiveData(uint8_t *buf, uint32_t len);

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
      printf("Waiting firmware header...\r\n");

      if (BL_ReceiveHeader(
              &g_fw_header)) { // 串口超时就打印Timeout，成功接收就打印Header内容并检查合法性
        BL_PrintHeader(&g_fw_header); // 调试用，打印接收到的固件头信息

        if (BL_CheckFirmwareHeader(&g_fw_header)) {
          printf("Header OK.\r\n");

          g_bl_ctx.expected_size = g_fw_header.size;
          g_bl_ctx.expected_crc32 = g_fw_header.crc32;
          g_bl_ctx.version = g_fw_header.version;
          g_bl_ctx.recv_size = 0;

          g_bl_ctx.state = BL_STATE_RECV_DATA;
        } else {
          printf("Header invalid.\r\n");
          g_bl_ctx.state = BL_STATE_ERROR;
        }
      } else {
        printf("Receive header timeout.\r\n");
        g_bl_ctx.state = BL_STATE_ERROR;
      }
      break;

    case BL_STATE_RECV_DATA:
      /* 这里接收固件数据 */
      uint32_t recv_len;

      recv_len = g_bl_ctx.expected_size;
      if (recv_len >
          sizeof(
              g_fw_data_buf)) { // 现在如果头里写的是 4096 字节，因为缓冲区只有
                                // 256 字节，所以这一步只先收前 256 字节
        recv_len = sizeof(g_fw_data_buf);
      }
      printf("Recv data stage. need=%lu bytes\r\n", recv_len);

      if (BL_ReceiveData(g_fw_data_buf, recv_len)) {
        g_fw_data_len = recv_len;
        g_bl_ctx.recv_size = recv_len;
        g_calc_crc32 = BL_CRC32_Calculate(g_fw_data_buf, g_fw_data_len);

        printf("Recv data OK. recv_size=%lu, calc_crc=0x%08lX\r\n",
               g_bl_ctx.recv_size, g_calc_crc32);

        if (BL_Flash_Erase_AppArea() != HAL_OK) {
          printf("Flash erase failed.\r\n");
          g_bl_ctx.state = BL_STATE_ERROR;
        } else {
          printf("Flash erase OK.\r\n");
          g_bl_ctx.state = BL_STATE_VERIFY_CRC;
        }
      } else {
        printf("Recv data timeout.\r\n");
        g_bl_ctx.state = BL_STATE_ERROR;
      }
      break;

    case BL_STATE_VERIFY_CRC:
      /* 这里做 CRC 校验 */
      printf("Verify CRC stage. calc=0x%08lX, expected=0x%08lX\r\n",
             g_calc_crc32, g_bl_ctx.expected_crc32);

      if (g_calc_crc32 != g_bl_ctx.expected_crc32) {
        printf("CRC mismatch.\r\n");
        g_bl_ctx.state = BL_STATE_ERROR;
      } else {
        printf("CRC OK.\r\n");

        if (BL_Flash_Write(APP_FLASH_START_ADDR, g_fw_data_buf,
                           g_fw_data_len) == HAL_OK) {
          printf("Flash write OK. addr=0x%08lX, len=%lu\r\n",
                 APP_FLASH_START_ADDR, g_fw_data_len);
          g_bl_ctx.state = BL_STATE_PROGRAM_DONE;
        } else {
          printf("Flash write failed.\r\n");
          g_bl_ctx.state = BL_STATE_ERROR;
        }
      }
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

/**
 * @brief 打印固件头信息，调试用
 */
static void BL_PrintHeader(const BL_FirmwareHeader_t *header) {
  if (header == 0) {
    return;
  }

  printf("Header: magic=0x%08lX, size=%lu, crc=0x%08lX, ver=%lu\r\n",
         header->magic, header->size, header->crc32, header->version);
}

/**
 * @brief 从 UART 接收固件头，阻塞函数，超时返回失败，固定16字节，5秒钟超时
 * @param header 输出参数，接收成功后填充固件头信息
 */
static uint8_t BL_ReceiveHeader(BL_FirmwareHeader_t *header) {
  HAL_StatusTypeDef status;

  if (header == 0) {
    return 0;
  }

  memset(header, 0, sizeof(BL_FirmwareHeader_t));

  status = HAL_UART_Receive(&huart1, (uint8_t *)header,
                            sizeof(BL_FirmwareHeader_t), 5000); // 5秒超时

  if (status != HAL_OK) {
    return 0;
  }

  return 1;
}

static uint8_t BL_ReceiveData(uint8_t *buf, uint32_t len) {
  HAL_StatusTypeDef status;

  if ((buf == 0) || (len == 0)) {
    return 0;
  }

  status = HAL_UART_Receive(&huart1, buf, len, 5000);

  if (status != HAL_OK) {
    return 0;
  }

  return 1;
}
