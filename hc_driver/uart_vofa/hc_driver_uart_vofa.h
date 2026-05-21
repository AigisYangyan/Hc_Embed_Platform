#ifndef HC_DRIVER_UART_VOFA_H
#define HC_DRIVER_UART_VOFA_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*HC_Driver_VOFA_RxCallback_t)(uint8_t byte);

HC_Error_e HC_Driver_VOFA_UART_Init(void);
HC_Error_e HC_Driver_VOFA_UART_Send(const uint8_t *p_data, uint16_t len);
void       HC_Driver_VOFA_UART_RegisterRxCallback(HC_Driver_VOFA_RxCallback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_UART_VOFA_H */
