#ifndef PORT_UART_VOFA_H
#define PORT_UART_VOFA_H

/* Legacy compatibility header — forwards to canonical hc_hal for UART types. */
#include "hc_hal/hc_hal_uart.h"
#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void Vofa_SendBytes(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* PORT_UART_VOFA_H */
