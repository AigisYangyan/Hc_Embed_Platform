/**
 * @file    hc_hal_wdg.c
 * @brief   看门狗 HAL 接口与实现，提供初始化、喂狗与复位标志读取。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_board_cfg.h"
#include "hc_hal_wdg.h"

/* 看门狗初始化标记 */
/* 冷启动读取到的复位来源 */
static HC_Bool_e s_wdg_reset_flag = HC_FALSE;

/* 初始化并启动看门狗（硬件接入待补齐） */
HC_S32 HC_HAL_WDG_Init(HC_VOID)
{
    /* TODO: 读取硬件复位来源标志 */
    s_wdg_reset_flag = HC_FALSE;

    /* TODO: 接入 MSPM0 看门狗硬件初始化（配置超时窗口并启动计数器） */

    return HC_HAL_OK;
}

/* 喂狗 */
HC_S32 HC_HAL_WDG_Feed(HC_VOID)
{
    /* TODO: 喂狗操作 */
    return HC_ERR_NOT_READY;
}

/* 读取是否因看门狗复位 */
HC_S32 HC_HAL_WDG_GetResetFlag(HC_Bool_e *p_is_wdg_reset)
{
    if (p_is_wdg_reset == HC_NULL_PTR) return HC_HAL_ERR_NULL_PTR;
    *p_is_wdg_reset = s_wdg_reset_flag;
    return HC_HAL_OK;
}
