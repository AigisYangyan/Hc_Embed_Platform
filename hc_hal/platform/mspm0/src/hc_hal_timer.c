/**
 * @file    hc_hal_timer.c
 * @brief   通用定时器 HAL 接口与实现，提供启停、回调注册与中断处理。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_timer.h"

/* ============================================================================
 *  模块私有数据：每个逻辑通道一个完成回调
 * ========================================================================== */
static HC_Timer_Callback_t s_timer_callbacks[TIMER_CH_MAX] = { HC_NULL_FN };

/* ============================================================================
 *  公开 API 实现 (详细语义见 hc_hal_timer.h)
 * ========================================================================== */

HC_Error_e HC_HAL_Timer_Init(HC_VOID)
{
    memset(s_timer_callbacks, 0, sizeof(s_timer_callbacks));
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Timer_Start(HC_HAL_Timer_Ch_e ch)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)TIMER_CH_MAX, HC_HAL_ERR_INVALID);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Timer_Stop(HC_HAL_Timer_Ch_e ch)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)TIMER_CH_MAX, HC_HAL_ERR_INVALID);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Timer_RegisterCallback(HC_HAL_Timer_Ch_e ch, HC_Timer_Callback_t cb)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)TIMER_CH_MAX, HC_HAL_ERR_INVALID);
    s_timer_callbacks[ch] = cb;
    return HC_HAL_OK;
}

/* ============================================================================
 *  中断服务
 * ========================================================================== */
HC_VOID HC_HAL_Timer_IRQHandler(HC_HAL_Timer_Ch_e ch)
{
    if (((HC_U32)ch < (HC_U32)TIMER_CH_MAX) && (s_timer_callbacks[ch] != HC_NULL_FN)) {
        s_timer_callbacks[ch](HC_TIMER_IRQ_ZERO);
    }
}
