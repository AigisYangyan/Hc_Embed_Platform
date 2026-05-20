#ifndef HC_HAL_UART_H
#define HC_HAL_UART_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"
#include "hc_hal_uart_cfg.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Neutral UART channel identifiers. Board mapping is in hc_cfg/hc_board_cfg.h. */
typedef enum {
    HC_HAL_UART_CH_0 = 0,
    HC_HAL_UART_CH_1 = 1,
    HC_HAL_UART_CH_2 = 2,
    HC_HAL_UART_CH_MAX
} HC_HAL_UART_Ch_e;

typedef HC_VOID (*HC_UART_RxCallback_t)(HC_U8 data);
typedef HC_VOID (*HC_UART_TxCallback_t)(HC_HAL_UART_Ch_e ch);

HC_Error_e HC_HAL_UART_Init(HC_VOID);
HC_Error_e HC_HAL_UART_SendByte(HC_HAL_UART_Ch_e ch, HC_U8 data);
HC_Error_e HC_HAL_UART_SendBuffer(HC_HAL_UART_Ch_e ch, const HC_U8 *p_buf, HC_U32 len);
HC_Error_e HC_HAL_UART_SendString(HC_HAL_UART_Ch_e ch, const HC_CHAR *p_str);
HC_Error_e HC_HAL_UART_RegisterRxCallback(HC_HAL_UART_Ch_e ch, HC_UART_RxCallback_t cb);
HC_Error_e HC_HAL_UART_RegisterTxCallback(HC_HAL_UART_Ch_e ch, HC_UART_TxCallback_t cb);
HC_Error_e HC_HAL_UART_SetPrintfChannel(HC_HAL_UART_Ch_e ch);
HC_Error_e HC_HAL_UART_Printf(HC_HAL_UART_Ch_e ch, const HC_CHAR *fmt, ...);
HC_VOID    HC_HAL_UART_IRQHandler(HC_HAL_UART_Ch_e ch);
HC_VOID    HC_HAL_UART_ErrorCallback(HC_HAL_UART_Ch_e ch);
HC_VOID    HC_HAL_UART_RxCpltCallback(HC_HAL_UART_Ch_e ch, HC_U8 data);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_UART_H */
