#ifndef PORT_SYSTICK_H
#define PORT_SYSTICK_H

/* Legacy compatibility header — forwards to canonical hc_hal for systick. */
#include "hc_hal/hc_hal_systick.h"

#ifdef __cplusplus
extern "C" {
#endif

void TaskTimeSliceManage(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_SYSTICK_H */
