/**
 * @file    hc_hal_delay.c
 * @brief   延时 HAL 接口与实现，提供毫秒/微秒级阻塞延时能力。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_delay.h"
#include <ti/driverlib/dl_common.h>

 /* ────────────────────────────────────────────
  *  外部配置表（定义在 mspm0g3507_delay_cfg.c）
  * ──────────────────────────────────────────── */
extern const HC_Delay_Cfg_t g_delayCfg;

/* ────────────────────────────────────────────
 *  模块私有变量
 * ──────────────────────────────────────────── */
HC_LOCAL HC_U32     s_cyclesPerMs = 0u;     /**< 每毫秒对应的 CPU 周期数 */
HC_LOCAL HC_U32     s_cyclesPerUs = 0u;     /**< 每微秒对应的 CPU 周期数 */

/* ============================================================
 *  公共 API 实现
 * ============================================================ */

HC_Error_e HC_HAL_Delay_Init(HC_VOID)
{
    /* 从配置表计算每 ms / us 的 cycle 数 */
    s_cyclesPerMs = g_delayCfg.cpuFreqHz / 1000u;      /* 32 MHz → 32000 */
    s_cyclesPerUs = g_delayCfg.cpuFreqHz / 1000000u;   /* 32 MHz → 32    */

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Delay_Ms(HC_U32 ms)
{
    /*
     * 逐毫秒调用 delay_cycles 避免 ms * cyclesPerMs 在大延时时
     * 超出 HC_U32 范围 (32 MHz 下 ~134 秒才溢出，但分段更安全)。
     */
    HC_U32 i;
    for (i = 0u; i < ms; i++)
    {
        DL_Common_delayCycles(s_cyclesPerMs);
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Delay_Us(HC_U32 us)
{
    /*
     * 微秒级延时：若 CPU 频率 < 1 MHz 则 s_cyclesPerUs 为 0，
     * 此时每次至少延时 1 cycle，保证不会空转。
     */
    if (s_cyclesPerUs > 0u)
    {
        HC_U32 i;
        for (i = 0u; i < us; i++)
        {
            DL_Common_delayCycles(s_cyclesPerUs);
        }
    }
    else
    {
        /* 主频低于 1 MHz 的保底处理 */
        DL_Common_delayCycles(us);
    }

    return HC_HAL_OK;
}
