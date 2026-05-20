/**
 * @file    mspm0g3507_delay_cfg.c
 * @brief   MSPM0G3507 延时模块板级配置，实现时基参数落地。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_delay_cfg.h"

const HC_Delay_Cfg_t g_delayCfg = {
    .cpuFreqHz = 32000000u
};
