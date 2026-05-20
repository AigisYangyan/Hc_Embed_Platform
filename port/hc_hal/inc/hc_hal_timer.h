#ifndef HC_HAL_TIMER_H
#define HC_HAL_TIMER_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_timer.h"

/* Legacy alias. */
typedef enum {
    TIMER_CH0   = HC_HAL_TIMER_CH_0,
    TIMER_CH_MAX = HC_HAL_TIMER_CH_MAX
} HC_HAL_Timer_Ch_e;

#endif /* HC_HAL_TIMER_H */
