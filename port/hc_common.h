/**
 * @file    hc_common.h
 * @brief   HC_Sys_Menu 框架最小公共类型定义
 *
 * 功能范围：
 * - 提供框架内统一布尔类型 HC_Bool_e (HC_TRUE / HC_FALSE)
 * - 提供 HC_UNUSED 宏抑制未用形参告警
 * - 不依赖任何 HAL/MCU 头，单文件自包含，跨平台可直接复用
 *
 * 设计约定：
 * - 框架代码只允许使用本文件提供的最小符号集
 * - 平台相关头文件在 port/ 目录下声明，禁止反向依赖
 */

#ifndef HC_COMMON_H
#define HC_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 通用布尔类型 ------------------------------------------------------- */

typedef enum {
    HC_FALSE = 0,
    HC_TRUE  = 1
} HC_Bool_e;

/* ---- 通用宏 ------------------------------------------------------------- */

/* 抑制未使用变量/形参编译告警 */
#ifndef HC_UNUSED
#define HC_UNUSED(x)   ((void)(x))
#endif

#ifdef __cplusplus
}
#endif

#endif /* HC_COMMON_H */
