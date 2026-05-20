/**
 * @file    hc_hal_systick.c
 * @brief   SYSTICK HAL 接口与实现，提供系统 Tick 获取与延时能力。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_board_cfg.h"
#include "hc_hal_systick.h"
#include "app/scheduler/task_scheduler.h"

/* SysTick 初始化标记 */
static volatile HC_Bool_e s_systick_is_init = HC_FALSE;
/* 累计的毫秒节拍 */
static volatile HC_U32    s_systick_tick_ms  = 0u;

/* 初始化 1ms SysTick 基准 */
HC_S32 HC_HAL_SYSTICK_Init(HC_VOID)
{
    if (s_systick_is_init == HC_TRUE) return HC_HAL_ERR_ALREADY_INIT;

    s_systick_tick_ms = 0u;
    DL_SYSTICK_config(32000u);  /* 32MHz / 32000 = 1kHz (1ms) */
    s_systick_is_init = HC_TRUE;
    return HC_HAL_OK;
}

/* 读取当前毫秒节拍 */
HC_S32 HC_HAL_SYSTICK_GetTickMs(HC_U32 *p_tick_ms)
{
    if (p_tick_ms == HC_NULL_PTR)      return HC_HAL_ERR_NULL_PTR;
    if (s_systick_is_init == HC_FALSE) return HC_HAL_ERR_NOT_INIT;
    *p_tick_ms = s_systick_tick_ms;
    return HC_HAL_OK;
}

/* 忙等毫秒级延时（裸机） */
HC_S32 HC_HAL_SYSTICK_DelayMs(HC_U32 delay_ms)
{
    HC_U32 start_ms, now_ms;
    if (delay_ms == 0u)                return HC_HAL_OK;
    if (s_systick_is_init == HC_FALSE) return HC_HAL_ERR_NOT_INIT;

    start_ms = s_systick_tick_ms;
    do {
        now_ms = s_systick_tick_ms;
    } while ((now_ms - start_ms) < delay_ms);

    return HC_HAL_OK;
}

/* 忙等微秒级延时（近似） */
HC_S32 HC_HAL_SYSTICK_DelayUs(HC_U32 delay_us)
{
    HC_U32 loop_per_us;
    if (delay_us == 0u)                return HC_HAL_OK;
    if (s_systick_is_init == HC_FALSE) return HC_HAL_ERR_NOT_INIT;

    loop_per_us = HC_HAL_SYSTICK_CPU_HZ / (HC_HAL_SYSTICK_DELAY_US_CYCLES * 1000000u);
    if (loop_per_us == 0u) loop_per_us = 1u;

    while (delay_us-- > 0u) {
        volatile HC_U32 cnt = loop_per_us;
        while (cnt-- > 0u);
    }
    return HC_HAL_OK;
}

/* SysTick 中断：递增节拍并驱动任务时间片 */
void SysTick_Handler(void)
{
    if (s_systick_is_init == HC_FALSE) return;
    s_systick_tick_ms++;
    TaskTimeSliceManage();
}
