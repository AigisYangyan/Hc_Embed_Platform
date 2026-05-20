/**
 * @file    hc_target_cfg.h
 * @brief   HC_EMBED_RULES 目标芯片与编译器配置
 *
 * 在此文件中配置目标 MCU 型号、编译器工具链和全局编译选项。
 * 本文件被所有层引用，修改后需全量重编译。
 */

#ifndef HC_TARGET_CFG_H
#define HC_TARGET_CFG_H

/* ---- 目标 MCU ------------------------------------------------------------- */

#define HC_TARGET_STM32F1    1
#define HC_TARGET_GD32F1     2
#define HC_TARGET_MM32F1     3

#ifndef HC_TARGET_MCU
#define HC_TARGET_MCU        HC_TARGET_STM32F1
#endif

/* ---- 编译器 --------------------------------------------------------------- */

#define HC_COMPILER_GCC      1
#define HC_COMPILER_ARMCC    2
#define HC_COMPILER_IAR      3

#if defined(__GNUC__) && !defined(__CC_ARM)
#define HC_COMPILER          HC_COMPILER_GCC
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define HC_COMPILER          HC_COMPILER_ARMCC
#elif defined(__ICCARM__)
#define HC_COMPILER          HC_COMPILER_IAR
#else
#error "Unsupported compiler. Define HC_COMPILER manually."
#endif

/* ---- 全局类型宽度（平台级约束） ------------------------------------------- */

/* 目标平台指针宽度 (字节) */
#define HC_PTR_SIZE           4

/* 目标平台最大对齐 (字节) */
#define HC_MAX_ALIGN          8

#endif /* HC_TARGET_CFG_H */
