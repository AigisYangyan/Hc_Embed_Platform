/**
 * @file    hc_def.h
 * @brief   HC_EMBED_RULES 平台基础宏与常量
 *
 * 提供框架公共宏定义：抑制告警、编译期断言、静态数组长度、
 * 最大/最小、对齐等。
 */

#ifndef HC_DEF_H
#define HC_DEF_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 编译器适配 ----------------------------------------------------------- */

#if defined(__GNUC__) || defined(__clang__)
#define HC_WEAK             __attribute__((weak))
#define HC_INLINE            static inline __attribute__((always_inline))
#define HC_PACKED            __attribute__((packed))
#define HC_ALIGNED(n)        __attribute__((aligned(n)))
#define HC_UNREACHABLE       __builtin_unreachable()
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define HC_WEAK             __weak
#define HC_INLINE            static __inline
#define HC_PACKED            __packed
#define HC_ALIGNED(n)        __align(n)
#define HC_UNREACHABLE
#else
#define HC_WEAK
#define HC_INLINE            static inline
#define HC_PACKED
#define HC_ALIGNED(n)
#define HC_UNREACHABLE
#endif

/* ---- 通用宏 --------------------------------------------------------------- */

/* 抑制未使用变量/形参编译告警 */
#ifndef HC_UNUSED
#define HC_UNUSED(x)   ((void)(x))
#endif

/* 静态数组元素个数 */
#ifndef HC_ARRAY_SIZE
#define HC_ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#endif

/* 取最大/最小值 */
#ifndef HC_MAX
#define HC_MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef HC_MIN
#define HC_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

/* 值域钳位 */
#define HC_CLAMP(val, lo, hi)  (HC_MIN(HC_MAX((val), (lo)), (hi)))

/* 编译期断言（C11 static_assert 的通用包装） */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define HC_STATIC_ASSERT(cond, msg)  _Static_assert((cond), msg)
#else
#define HC_STATIC_ASSERT(cond, msg)
#endif

/* 位操作 */
#define HC_BIT(n)            (1UL << (n))
#define HC_BITMASK(hi, lo)   (((1UL << ((hi) - (lo) + 1)) - 1) << (lo))

#ifdef __cplusplus
}
#endif

#endif /* HC_DEF_H */
