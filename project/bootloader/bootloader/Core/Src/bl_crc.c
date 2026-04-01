#include "bl_crc.h"

/* 软件CRC32计算接口 */

uint32_t BL_CRC32_Calculate(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    uint32_t j;
    uint32_t crc = 0xFFFFFFFFU;

    for (i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 1U)
            {
                crc = (crc >> 1) ^ 0xEDB88320U;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}
