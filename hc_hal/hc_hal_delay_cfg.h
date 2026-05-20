#ifndef HC_HAL_DELAY_CFG_H
#define HC_HAL_DELAY_CFG_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    HC_U32 cpuFreqHz;
} HC_Delay_Cfg_t;

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_DELAY_CFG_H */
