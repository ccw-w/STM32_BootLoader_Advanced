#include "bootloader.h"
#include "bl_crc.h"
#include "bl_flash.h"
#include "bl_jump.h"
#include "bl_meta.h"
#include "bl_mode.h"
#include "bl_protocol.h"
#include "bl_state.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

static BL_Context_t g_bl_ctx;
static BL_FirmwareHeader_t g_fw_header; // 后面收到16个字节后解析出固件头结果
static uint8_t g_fw_data_buf[256];      // 固件数据接收缓冲区，测试先接收256字节
static uint32_t g_fw_data_len = 0;      // 实际收到了多少字节
static uint32_t g_calc_crc32 = 0;       // 已经接收数据的 CRC32
static uint32_t g_fw_write_addr = 0;    // 当前写到 Flash 的地址
static uint32_t g_fw_total_size = 0;    // 这次固件大小
static uint32_t g_fw_remaining_size = 0; // 还剩下多少字节没有接收
static BL_MetaInfo_t g_meta_info;
static uint8_t g_idle_printed = 0;
static uint32_t BL_GetSlotAddress(BL_Slot_t slot);
static uint32_t BL_GetActiveAppAddress(void);

static uint8_t BL_ReceiveHeader(BL_FirmwareHeader_t *header);
static void BL_PrintHeader(const BL_FirmwareHeader_t *header);
static uint8_t BL_ReceiveData(uint8_t *buf, uint32_t len);
static BL_Slot_t BL_GetInactiveSlot(BL_Slot_t active_slot);
static uint32_t BL_GetTargetAppAddress(void);
static uint32_t BL_GetSlotSize(BL_Slot_t slot);
static uint32_t BL_GetTargetAppSize(void);
static void BL_HandleBootDecision(uint8_t meta_valid);
static void BL_HandleWaitHeader(void);
static void BL_HandleRecvData(void);
static void BL_HandleVerifyCrc(void);

void Bootloader_Run(void) {
  memset(&g_fw_header, 0, sizeof(g_fw_header)); // 固件头清零
  memset(&g_meta_info, 0, sizeof(g_meta_info));
  uint8_t meta_valid = 0;
  BL_State_Init(&g_bl_ctx); // 初始化状态机为空闲状态，已经初始化则return
  printf("==== Bootloader Start ====\r\n");
  printf("BL start: 0x%08lX\r\n", BL_FLASH_START_ADDR);
  meta_valid = BL_Meta_Read(&g_meta_info);

  if (meta_valid) {
    printf("Meta: status=%lu, active=%c, target=%c, rollback=%c, pending=%lu, "
           "confirmed=%lu, size=%lu, crc=0x%08lX, ver=%lu\r\n",
           g_meta_info.status, g_meta_info.active_slot, g_meta_info.target_slot,
           g_meta_info.rollback_slot, g_meta_info.boot_pending,
           g_meta_info.confirmed, g_meta_info.size, g_meta_info.crc32,
           g_meta_info.version);
  } else {
    printf("Meta: empty, default slot A\r\n");

    g_meta_info.active_slot = BL_SLOT_A;
    g_meta_info.target_slot = BL_SLOT_B;
    g_meta_info.rollback_slot = BL_SLOT_A;
    g_meta_info.boot_pending = 0U;
    g_meta_info.confirmed = 1U;
  }
  printf("Active app addr: 0x%08lX\r\n", BL_GetActiveAppAddress());

  HAL_Delay(BL_JUMP_DELAY_MS);

  BL_HandleBootDecision(meta_valid); // 启动决策

  while (1) {
    switch (g_bl_ctx.state) {
    case BL_STATE_WAIT_HEADER:
      BL_HandleWaitHeader();
      break;
    case BL_STATE_RECV_DATA:
      BL_HandleRecvData();
      break;
    case BL_STATE_VERIFY_CRC:
      BL_HandleVerifyCrc();
      break;
    case BL_STATE_PROGRAM_DONE:
      printf("Update success. Active slot switched to %c. You may reset and "
             "run app.\r\n",
             g_meta_info.active_slot);
      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_IDLE;
      break;

    case BL_STATE_ERROR:
      printf("Bootloader error.\r\n");
      printf("Update failed. Waiting firmware header...\r\n");
      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_WAIT_HEADER;
      break;

    case BL_STATE_IDLE:
      if (g_idle_printed == 0U) {
        printf("Bootloader idle. Reset to run app.\r\n");
        g_idle_printed = 1U;
      }
      HAL_Delay(100);
      break;

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

static uint32_t BL_GetSlotAddress(BL_Slot_t slot) {
  if (slot == BL_SLOT_A) {
    return APP_SLOT_A_ADDR;
  } else if (slot == BL_SLOT_B) {
    return APP_SLOT_B_ADDR;
  }

  return APP_SLOT_A_ADDR;
}

static uint32_t BL_GetActiveAppAddress(void) {
  if (g_meta_info.active_slot == BL_SLOT_B) {
    return APP_SLOT_B_ADDR;
  }

  return APP_SLOT_A_ADDR;
}

static BL_Slot_t BL_GetInactiveSlot(BL_Slot_t active_slot) {
  if (active_slot == BL_SLOT_A) {
    return BL_SLOT_B;
  } else if (active_slot == BL_SLOT_B) {
    return BL_SLOT_A;
  }

  return BL_SLOT_B;
}

static uint32_t BL_GetTargetAppAddress(void) {
  return BL_GetSlotAddress((BL_Slot_t)g_meta_info.target_slot);
}

static uint32_t BL_GetSlotSize(BL_Slot_t slot) {
  if (slot == BL_SLOT_A) {
    return APP_SLOT_A_SIZE;
  } else if (slot == BL_SLOT_B) {
    return APP_SLOT_B_SIZE;
  }

  return APP_SLOT_A_SIZE;
}

static uint32_t BL_GetTargetAppSize(void) {
  return BL_GetSlotSize((BL_Slot_t)g_meta_info.target_slot);
}

static void BL_HandleBootDecision(uint8_t meta_valid) {
  if (meta_valid && (g_meta_info.status == BL_META_STATUS_TESTING) &&
      (g_meta_info.confirmed == 0U) && (g_meta_info.boot_pending == 1U)) {
    uint32_t app_addr = BL_GetActiveAppAddress();
    uint32_t app_size = BL_GetSlotSize((BL_Slot_t)g_meta_info.active_slot);

    printf("Testing new app in slot %c...\r\n", g_meta_info.active_slot);

    if (!BL_IsAppValid(app_addr, app_size)) {
      printf("Test app invalid in slot %c.\r\n", g_meta_info.active_slot);

      g_meta_info.active_slot = g_meta_info.rollback_slot;
      g_meta_info.target_slot =
          BL_GetInactiveSlot((BL_Slot_t)g_meta_info.active_slot);
      g_meta_info.boot_pending = 0U;
      g_meta_info.confirmed = 1U;

      BL_Meta_Set(BL_META_STATUS_ERROR, (BL_Slot_t)g_meta_info.active_slot,
                  (BL_Slot_t)g_meta_info.target_slot,
                  (BL_Slot_t)g_meta_info.rollback_slot,
                  g_meta_info.boot_pending, g_meta_info.confirmed,
                  g_meta_info.size, g_meta_info.crc32, g_meta_info.version);

      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_WAIT_HEADER;
    } else {
      g_meta_info.boot_pending = 0U;

      BL_Meta_Set(BL_META_STATUS_TESTING, (BL_Slot_t)g_meta_info.active_slot,
                  (BL_Slot_t)g_meta_info.target_slot,
                  (BL_Slot_t)g_meta_info.rollback_slot,
                  g_meta_info.boot_pending, 0U, g_meta_info.size,
                  g_meta_info.crc32, g_meta_info.version);

      while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET) {
      }

      BL_JumpToApp(app_addr, app_size);

      printf("Test jump failed, back to bootloader.\r\n");
      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_WAIT_HEADER;
    }
  } else if (meta_valid && (g_meta_info.status == BL_META_STATUS_TESTING) &&
             (g_meta_info.confirmed == 0U) &&
             (g_meta_info.boot_pending == 0U)) {
    uint32_t app_addr;
    uint32_t app_size;

    printf("New app not confirmed, rollback to slot %c.\r\n",
           g_meta_info.rollback_slot);

    g_meta_info.active_slot = g_meta_info.rollback_slot;
    g_meta_info.target_slot =
        BL_GetInactiveSlot((BL_Slot_t)g_meta_info.active_slot);
    g_meta_info.rollback_slot = g_meta_info.active_slot;
    g_meta_info.boot_pending = 0U;
    g_meta_info.confirmed = 1U;

    BL_Meta_Set(BL_META_STATUS_OK, (BL_Slot_t)g_meta_info.active_slot,
                (BL_Slot_t)g_meta_info.target_slot,
                (BL_Slot_t)g_meta_info.rollback_slot, g_meta_info.boot_pending,
                g_meta_info.confirmed, g_meta_info.size, g_meta_info.crc32,
                g_meta_info.version);

    app_addr = BL_GetActiveAppAddress();
    app_size = BL_GetSlotSize((BL_Slot_t)g_meta_info.active_slot);

    if (BL_IsAppValid(app_addr, app_size)) {
      printf("Rollback done. Jump back to slot %c.\r\n",
             g_meta_info.active_slot);

      while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET) {
      }

      BL_JumpToApp(app_addr, app_size);
    } else {
      printf("Rollback slot invalid, stay in bootloader.\r\n");
      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_WAIT_HEADER;
    }
  } else if (meta_valid && ((g_meta_info.status == BL_META_STATUS_UPDATING) ||
                            (g_meta_info.status == BL_META_STATUS_ERROR))) {
    printf("Meta says update not finished, stay in bootloader.\r\n");
    g_idle_printed = 0U;
    g_bl_ctx.state = BL_STATE_WAIT_HEADER;
  } else if (BL_ShouldEnterUpdateMode()) {
    printf("Enter update mode.\r\n");
    g_idle_printed = 0U;
    g_bl_ctx.state = BL_STATE_WAIT_HEADER;
  } else {
    uint32_t app_addr = BL_GetActiveAppAddress();
    uint32_t app_size = BL_GetSlotSize((BL_Slot_t)g_meta_info.active_slot);

    if (BL_IsAppValid(app_addr, app_size)) {
      printf("App valid in slot %c, jump now...\r\n", g_meta_info.active_slot);

      while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET) {
      }

      BL_JumpToApp(app_addr, app_size);
    } else {
      printf("No valid app found in active slot, stay in bootloader.\r\n");
      g_idle_printed = 0U;
      g_bl_ctx.state = BL_STATE_WAIT_HEADER;
    }
  }
}

static void BL_HandleWaitHeader(void) {
  printf("Waiting firmware header...\r\n");

  if (BL_ReceiveHeader(&g_fw_header)) {
    BL_PrintHeader(&g_fw_header);

    if (BL_CheckFirmwareHeader(&g_fw_header)) {
      printf("Header OK.\r\n");

      g_bl_ctx.expected_size = g_fw_header.size;
      g_bl_ctx.expected_crc32 = g_fw_header.crc32;
      g_bl_ctx.version = g_fw_header.version;
      g_bl_ctx.recv_size = 0;

      g_fw_total_size = g_fw_header.size;
      g_fw_remaining_size = g_fw_header.size;
      g_meta_info.target_slot =
          BL_GetInactiveSlot((BL_Slot_t)g_meta_info.active_slot);
      g_fw_write_addr = BL_GetTargetAppAddress();
      g_calc_crc32 = 0;

      printf("Target slot=%c, write_addr=0x%08lX\r\n", g_meta_info.target_slot,
             g_fw_write_addr);

      if (BL_Flash_Erase_Area(BL_GetTargetAppAddress(),
                              BL_GetTargetAppSize()) != HAL_OK) {
        printf("Flash erase failed.\r\n");
        g_bl_ctx.state = BL_STATE_ERROR;
      } else {
        printf("Flash erase OK. target_slot=%c, addr=0x%08lX\r\n",
               g_meta_info.target_slot, g_fw_write_addr);

        BL_Meta_Set(BL_META_STATUS_UPDATING, (BL_Slot_t)g_meta_info.active_slot,
                    (BL_Slot_t)g_meta_info.target_slot,
                    (BL_Slot_t)g_meta_info.active_slot, 0U, 0U,
                    g_fw_header.size, g_fw_header.crc32, g_fw_header.version);

        g_bl_ctx.state = BL_STATE_RECV_DATA;
      }
    } else {
      printf("Header invalid.\r\n");
      g_bl_ctx.state = BL_STATE_ERROR;
    }
  } else {
    printf("Receive header timeout.\r\n");
    g_bl_ctx.state = BL_STATE_ERROR;
  }
}

static void BL_HandleRecvData(void) {
  uint32_t recv_len;

  if (g_fw_remaining_size == 0U) {
    g_bl_ctx.state = BL_STATE_VERIFY_CRC;
    return;
  }

  recv_len = g_fw_remaining_size;
  if (recv_len > sizeof(g_fw_data_buf)) {
    recv_len = sizeof(g_fw_data_buf);
  }

  printf("Recv chunk. need=%lu, remaining=%lu\r\n", recv_len,
         g_fw_remaining_size);

  if (BL_ReceiveData(g_fw_data_buf, recv_len)) {
    g_fw_data_len = recv_len;

    if (BL_Flash_Write(g_fw_write_addr, g_fw_data_buf, g_fw_data_len) ==
        HAL_OK) {
      g_fw_write_addr += g_fw_data_len;
      g_fw_remaining_size -= g_fw_data_len;
      g_bl_ctx.recv_size += g_fw_data_len;

      printf("Chunk write OK. recv_size=%lu, remaining=%lu\r\n",
             g_bl_ctx.recv_size, g_fw_remaining_size);
    } else {
      printf("Chunk write failed.\r\n");
      g_bl_ctx.state = BL_STATE_ERROR;
    }
  } else {
    printf("Recv data timeout.\r\n");
    g_bl_ctx.state = BL_STATE_ERROR;
  }
}

static void BL_HandleVerifyCrc(void) {
  g_calc_crc32 = BL_CRC32_Calculate((const uint8_t *)BL_GetTargetAppAddress(),
                                    g_fw_total_size);
  printf("Verify CRC stage. calc=0x%08lX, expected=0x%08lX\r\n", g_calc_crc32,
         g_bl_ctx.expected_crc32);

  if (g_calc_crc32 == g_bl_ctx.expected_crc32) {
    BL_Slot_t old_slot;
    BL_Slot_t new_slot;

    printf("CRC OK.\r\n");

    old_slot = (BL_Slot_t)g_meta_info.active_slot;
    new_slot = (BL_Slot_t)g_meta_info.target_slot;

    g_meta_info.active_slot = new_slot;
    g_meta_info.rollback_slot = old_slot;
    g_meta_info.target_slot = BL_GetInactiveSlot(new_slot);
    g_meta_info.boot_pending = 1U;
    g_meta_info.confirmed = 0U;

    BL_Meta_Set(BL_META_STATUS_TESTING, (BL_Slot_t)g_meta_info.active_slot,
                (BL_Slot_t)g_meta_info.target_slot,
                (BL_Slot_t)g_meta_info.rollback_slot, g_meta_info.boot_pending,
                g_meta_info.confirmed, g_fw_total_size, g_bl_ctx.expected_crc32,
                g_bl_ctx.version);

    g_bl_ctx.state = BL_STATE_PROGRAM_DONE;
  } else {
    printf("CRC mismatch.\r\n");

    BL_Meta_Set(BL_META_STATUS_ERROR, (BL_Slot_t)g_meta_info.active_slot,
                (BL_Slot_t)g_meta_info.target_slot,
                (BL_Slot_t)g_meta_info.rollback_slot, 0U, 0U, g_fw_total_size,
                g_bl_ctx.expected_crc32, g_bl_ctx.version);

    g_bl_ctx.state = BL_STATE_ERROR;
  }
}
