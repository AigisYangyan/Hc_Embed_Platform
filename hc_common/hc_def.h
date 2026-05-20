#ifndef HC_DEF_H
#define HC_DEF_H

#include "hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HC_UNUSED
#define HC_UNUSED(x)   ((void)(x))
#endif

#ifndef HC_NULL_FN
#define HC_NULL_FN     ((void*)0)
#endif

#ifndef HC_LOCAL
#define HC_LOCAL       static inline
#endif

#ifndef HC_WEAK
#if defined(__GNUC__) || defined(__clang__)
#define HC_WEAK        __attribute__((weak))
#else
#define HC_WEAK
#endif
#endif

#ifndef HC_ASSERT
#define HC_ASSERT(cond)   ((void)(cond))
#endif

#ifdef __cplusplus
}
#endif

#endif /* HC_DEF_H */
