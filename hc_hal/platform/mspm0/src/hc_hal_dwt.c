/**
 * @file    hc_hal_dwt.c
 * @brief   DWT HAL 接口与实现，提供周期计数读取与高精度延时能力。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_board_cfg.h"
#include "hc_hal_dwt.h"

/* DWT 不支持：返回未就绪 */
HC_S32 HC_HAL_DWT_Init(HC_VOID)
{
    return HC_ERR_NOT_READY;
}

/* 读取周期计数：占位未实现 */
HC_S32 HC_HAL_DWT_GetCycleCount(HC_U32 *p_cycles)
{
    HC_UNUSED(p_cycles);
    return HC_ERR_NOT_READY;
}

/* 基于 DWT 的延时：占位未实现 */
HC_S32 HC_HAL_DWT_DelayUs(HC_U32 delay_us)
{
    HC_UNUSED(delay_us);
    return HC_ERR_NOT_READY;
}
