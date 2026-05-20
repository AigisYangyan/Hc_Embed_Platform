#ifndef HC_HAL_TIMER_H
#define HC_HAL_TIMER_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_TIMER_MODE_PERIODIC = 0,
    HC_TIMER_MODE_ONE_SHOT = 1
} HC_Timer_Mode_e;

typedef enum {
    HC_TIMER_IRQ_NONE = 0x00u,
    HC_TIMER_IRQ_ZERO = 0x01u
} HC_Timer_IrqEvent_e;

typedef HC_VOID (*HC_Timer_Callback_t)(HC_Timer_IrqEvent_e irqEvent);

typedef enum {
    HC_HAL_TIMER_CH_0 = 0,
    HC_HAL_TIMER_CH_MAX
} HC_HAL_Timer_Ch_e;

HC_Error_e HC_HAL_Timer_Init(HC_VOID);
HC_Error_e HC_HAL_Timer_Start(HC_HAL_Timer_Ch_e ch);
HC_Error_e HC_HAL_Timer_Stop(HC_HAL_Timer_Ch_e ch);
HC_Error_e HC_HAL_Timer_RegisterCallback(HC_HAL_Timer_Ch_e ch, HC_Timer_Callback_t cb);
HC_VOID    HC_HAL_Timer_IRQHandler(HC_HAL_Timer_Ch_e ch);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_TIMER_H */
