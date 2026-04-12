#include "bl_mode.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

uint8_t BL_ShouldEnterUpdateMode(void)
{
    uint8_t rx_buf[8] = {0};

    printf("Press update command in 2s...\r\n");

    if (HAL_UART_Receive(&huart1, rx_buf, 6, 2000) == HAL_OK)
    {
        if (memcmp(rx_buf, "update", 6) == 0)
        {
            printf("Update command detected.\r\n");
            return 1;
        }
    }

    return 0;
}
