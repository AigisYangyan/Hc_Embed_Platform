#ifndef HC_HAL_UART_H
#define HC_HAL_UART_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_uart.h"
#include <stdio.h>

/* Project-semantic aliases (deprecated). New code should use HC_HAL_UART_Ch_e. */
typedef enum {
    UART_CH_STEPMOTOR = HC_HAL_UART_CH_0,
    UART_CH_VOFA      = HC_HAL_UART_CH_1,
    UART_CH_VISION    = HC_HAL_UART_CH_2,
    UART_CH_MAX       = HC_HAL_UART_CH_MAX
} HC_HAL_UART_Ch_e;

/* Legacy compatibility typedefs and wrappers. */
typedef HC_HAL_UART_Ch_e HC_HAL_UART_Id_e;
typedef HC_UART_RxCallback_t HC_HAL_UART_RxCallback_t;

#define HC_HAL_UART_ID_0   UART_CH_STEPMOTOR
#define HC_HAL_UART_ID_1   UART_CH_VOFA
#define HC_HAL_UART_ID_2   UART_CH_VISION
#define HC_HAL_UART_ID_MAX UART_CH_MAX

HC_LOCAL inline HC_Error_e HC_HAL_UART_ModuleInit(HC_VOID)
{
    return HC_HAL_UART_Init();
}

HC_LOCAL inline HC_Error_e HC_HAL_UART_SendById(HC_HAL_UART_Id_e id,
                                                const HC_U8 *p_buf,
                                                HC_U16 len)
{
    return HC_HAL_UART_SendBuffer((HC_HAL_UART_Ch_e)id, p_buf, (HC_U32)len);
}

HC_LOCAL inline HC_Error_e HC_HAL_UART_SendByteById(HC_HAL_UART_Id_e id,
                                                    HC_U8 data)
{
    return HC_HAL_UART_SendByte((HC_HAL_UART_Ch_e)id, data);
}

#endif /* HC_HAL_UART_H */
