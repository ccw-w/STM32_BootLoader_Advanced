#include "bl_state.h"

/* 定义升级状态机和状态记录结构 */

void BL_State_Init(BL_Context_t *ctx)
{
    if (ctx == 0)
    {
        return;
    }

    ctx->state = BL_STATE_IDLE;
    ctx->recv_size = 0;
    ctx->expected_size = 0;
    ctx->expected_crc32 = 0;
    ctx->version = 0;
}
