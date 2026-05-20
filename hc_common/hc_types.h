/**
 * @file    hc_types.h
 * @brief   HC_EMBED_RULES 平台基础类型定义
 *
 * 提供框架内统一的布尔类型、标准整型别名和通用结构。
 * 不依赖任何 HAL/MCU 头文件，单文件自包含。
 */

#ifndef HC_TYPES_H
#define HC_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 布尔类型 ------------------------------------------------------------- */

typedef enum {
    HC_FALSE = 0,
    HC_TRUE  = 1
} HC_Bool_e;

/* ---- 通用回调类型 --------------------------------------------------------- */

typedef void (*HC_VoidFn_t)(void);

/* ---- 结果容器 ------------------------------------------------------------- */

typedef struct {
    int32_t  code;   /* 错误码，HC_OK 表示成功 */
    void    *data;   /* 返回数据指针，可为 NULL */
} HC_Result_t;

#ifdef __cplusplus
}
#endif

#endif /* HC_TYPES_H */
