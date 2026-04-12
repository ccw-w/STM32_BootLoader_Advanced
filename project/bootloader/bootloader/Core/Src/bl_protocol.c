#include "bl_protocol.h"
#include "bl_config.h"

uint8_t BL_CheckFirmwareHeader(const BL_FirmwareHeader_t *header) {
  if (header == 0) {
    return 0;
  }

  /* 1. magic 必须正确 */
  if (header->magic != BL_FW_MAGIC) {
    return 0;
  }

  /* 2. size 不能为 0 */
  if (header->size == 0U) {
    return 0;
  }

  /* 3. 因为目前 A、B 两个槽大小一样，都是 48KB,所以这里先拿 APP_SLOT_A_SIZE
   * 当统一槽大小上限 */
  if (header->size > APP_SLOT_A_SIZE) {
    return 0;
  }

  /* 4. crc32 不能是明显空值 */
  if ((header->crc32 == 0x00000000U) || (header->crc32 == 0xFFFFFFFFU)) {
    return 0;
  }

  return 1;
}
