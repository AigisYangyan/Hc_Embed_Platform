/**
 * @file    hc_module_cfg.h
 * @brief   HC_EMBED_RULES 模块依赖校验与拓扑约束
 *
 * 本文件通过编译期断言确保各层之间的依赖关系符合 HC_EMBED_RULES：
 *   hc_app → hc_service → hc_middleware → hc_driver → hc_hal
 *
 * 依赖校验示例（编译期生效）：
 *   - service 必须依赖 middleware（不得绕过）
 *   - middleware 必须依赖 driver（不得绕过）
 *   - driver 必须依赖 hal（不得绕过）
 *
 * 当前为骨架阶段：校验宏已定义，等待对应层实现文件创建后生效。
 * 旧菜单框架目录 (port/runtime/service/app/ui/system/scheduler) 不在校验范围内。
 */

#ifndef HC_MODULE_CFG_H
#define HC_MODULE_CFG_H

#include "hc_common/hc_def.h"
#include "hc_cfg/hc_feature_cfg.h"

/* ---- 模块存在性声明（各层实现后取消注释以激活校验） ----------------------- */

/* #define HC_HAL_PRESENT          1  */   /* hc_hal 实现就绪后启用 */
/* #define HC_DRIVER_PRESENT       1  */   /* hc_driver 实现就绪后启用 */
/* #define HC_MIDDLEWARE_PRESENT   1  */   /* hc_middleware 实现就绪后启用 */
/* #define HC_SERVICE_PRESENT      1  */   /* hc_service 实现就绪后启用 */
/* #define HC_TASK_PRESENT         1  */   /* hc_task 实现就绪后启用 */
/* #define HC_APP_PRESENT          1  */   /* hc_app 实现就绪后启用 */

/* ---- 依赖校验规则 --------------------------------------------------------- */

/*
 * 规则 1: service → middleware
 *   hc_service 层的模块必须通过 hc_middleware 访问 hc_driver。
 *   禁止 service 直接 include hc_driver/ 或 hc_hal/。
 *
 * 编译期校验示例（各层 present 宏启用后生效）：
 *
 *   #if HC_SERVICE_PRESENT
 *   HC_STATIC_ASSERT(HC_MIDDLEWARE_PRESENT,
 *       "hc_service requires hc_middleware: cannot depend on driver directly");
 *   #endif
 */

/*
 * 规则 2: middleware → driver
 *   hc_middleware 层的模块必须通过 hc_driver 访问 hc_hal。
 *   禁止 middleware 直接 include hc_hal/ 或厂商 SDK 头文件。
 *
 *   #if HC_MIDDLEWARE_PRESENT
 *   HC_STATIC_ASSERT(HC_DRIVER_PRESENT,
 *       "hc_middleware requires hc_driver: cannot depend on hal directly");
 *   #endif
 */

/*
 * 规则 3: driver → hal
 *   hc_driver 层的模块必须通过 hc_hal 访问硬件寄存器。
 *   禁止 driver 直接操作 MCU 寄存器或 include 厂商 HAL 内部头文件。
 *
 *   #if HC_DRIVER_PRESENT
 *   HC_STATIC_ASSERT(HC_HAL_PRESENT,
 *       "hc_driver requires hc_hal: cannot access MCU registers directly");
 *   #endif
 */

/*
 * 规则 4: 禁止反向依赖
 *   hc_hal 不得依赖 hc_driver / hc_middleware / hc_service / hc_app。
 *   hc_driver 不得依赖 hc_service / hc_app。
 *   hc_middleware 不得依赖 hc_service / hc_app。
 *
 * 由 check_hc_embed_arch.py 在代码扫描阶段静态检查。
 */

/* ---- 模块间接口契约（骨架） ----------------------------------------------- */

/*
 * HC_EMBED_RULES 模块间仅通过以下接口交互：
 *
 *   上层 → 下层:  直接调用下层头文件中声明的公开 API
 *   下层 → 上层:  通过回调函数指针 / 事件队列通知，禁止直接 include 上层头文件
 *   同层之间:     service 之间禁止互相依赖；middleware 之间通过接口解耦
 */

#endif /* HC_MODULE_CFG_H */
