/**
 * @file    hc_runtime_cfg.h
 * @brief   运行时后端配置
 */

#ifndef SYSTEM_HC_RUNTIME_CFG_H
#define SYSTEM_HC_RUNTIME_CFG_H

#define HC_RUNTIME_BAREMETAL 0
#define HC_RUNTIME_FREERTOS  1

#ifndef HC_RUNTIME_MODE
#define HC_RUNTIME_MODE HC_RUNTIME_BAREMETAL
#endif

#endif /* SYSTEM_HC_RUNTIME_CFG_H */
