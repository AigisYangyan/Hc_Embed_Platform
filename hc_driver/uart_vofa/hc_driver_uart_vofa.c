#include "hc_driver_uart_vofa.h"
#include "hc_hal_uart.h"

#define VOFA_UART_CH  HC_HAL_UART_CH_0

static HC_Driver_VOFA_RxCallback_t s_rx_callback = (HC_Driver_VOFA_RxCallback_t)0;

static void vofa_hal_rx_cb(uint8_t byte)
{
    if (s_rx_callback != (HC_Driver_VOFA_RxCallback_t)0) {
        s_rx_callback(byte);
    }
}

HC_Error_e HC_Driver_VOFA_UART_Init(void)
{
    HC_Error_e err = HC_HAL_UART_RegisterRxCallback(VOFA_UART_CH, vofa_hal_rx_cb);
    if (err != HC_HAL_OK) {
        return err;
    }
    return HC_HAL_UART_Init();
}

HC_Error_e HC_Driver_VOFA_UART_Send(const uint8_t *p_data, uint16_t len)
{
    if ((p_data == (void*)0) || (len == 0u)) {
        return HC_HAL_ERR_NULL_PTR;
    }
    return HC_HAL_UART_SendBuffer(VOFA_UART_CH, p_data, (uint32_t)len);
}

void HC_Driver_VOFA_UART_RegisterRxCallback(HC_Driver_VOFA_RxCallback_t callback)
{
    s_rx_callback = callback;
}
