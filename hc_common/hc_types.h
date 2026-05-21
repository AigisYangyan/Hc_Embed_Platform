#ifndef HC_TYPES_H
#define HC_TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t   HC_U8;
typedef int8_t    HC_S8;
typedef uint16_t  HC_U16;
typedef int16_t   HC_S16;
typedef uint32_t  HC_U32;
typedef int32_t   HC_S32;
typedef char      HC_CHAR;
typedef void      HC_VOID;

typedef enum {
    HC_FALSE = 0,
    HC_TRUE  = 1
} HC_Bool_e;

#ifdef __cplusplus
}
#endif

#endif /* HC_TYPES_H */
