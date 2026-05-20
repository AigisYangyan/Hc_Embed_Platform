#ifndef HC_ERR_H
#define HC_ERR_H

#include "hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_HAL_OK                 =  0,
    HC_HAL_ERR_INVALID        = -1,
    HC_HAL_ERR_NULL_PTR       = -2,
    HC_HAL_ERR_TIMEOUT        = -3,
    HC_HAL_ERR_NOT_INIT       = -4,
    HC_HAL_ERR_ALREADY_INIT   = -5,
    HC_HAL_ERR_NOT_ENABLE     = -6,
    HC_HAL_ERR_BUSY           = -7,
    HC_HAL_ERR_NOT_READY      = -8,
    HC_HAL_ERR_I2C            = -9
} HC_Error_e;

/* Legacy aliases for existing code. Deprecated. */
#define HC_ERR_NOT_ENABLE    HC_HAL_ERR_NOT_ENABLE
#define HC_ERR_BUSY          HC_HAL_ERR_BUSY
#define HC_ERR_NOT_READY     HC_HAL_ERR_NOT_READY

#ifdef __cplusplus
}
#endif

#endif /* HC_ERR_H */
