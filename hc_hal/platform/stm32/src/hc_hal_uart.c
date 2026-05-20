#include "hc_hal_uart.h"

#include "usart.h"

#include <stdio.h>
#include <string.h>

extern const HC_UART_Cfg_t g_uartCfgMap[UART_CH_MAX];

#define HC_HAL_UART_DMA_TX_BUFFER_SIZE 512u
#define HC_HAL_UART_PRINTF_BUFFER_SIZE 128u

typedef struct {
    UART_HandleTypeDef *handle;
    IRQn_Type irqn;
    HC_UART_RxCallback_t rx_callback;
    HC_UART_TxCallback_t tx_callback;
    HC_Bool_e dma_tx_enable;
    HC_Bool_e dma_rx_enable;
    HC_Bool_e tx_busy;
    HC_U8 rx_byte;
    HC_U8 tx_buffer[HC_HAL_UART_DMA_TX_BUFFER_SIZE];
} HC_HAL_UART_Port_t;

static HC_HAL_UART_Port_t s_uart_ports[UART_CH_MAX];
static HC_HAL_UART_Ch_e s_printf_channel = UART_CH_VOFA;

static HC_HAL_UART_Port_t *hc_hal_uart_get_port(HC_HAL_UART_Ch_e ch)
{
    if ((HC_U32)ch >= (HC_U32)UART_CH_MAX) {
        return HC_NULL_PTR;
    }

    return &s_uart_ports[ch];
}

static HC_HAL_UART_Ch_e hc_hal_uart_find_channel(const UART_HandleTypeDef *handle)
{
    HC_U32 i;

    for (i = 0u; i < (HC_U32)UART_CH_MAX; i++) {
        if (s_uart_ports[i].handle == handle) {
            return (HC_HAL_UART_Ch_e)i;
        }
    }

    return UART_CH_MAX;
}

static HC_Error_e hc_hal_uart_map_status(HAL_StatusTypeDef status)
{
    switch (status) {
    case HAL_OK:
        return HC_HAL_OK;
    case HAL_TIMEOUT:
        return HC_HAL_ERR_TIMEOUT;
    case HAL_BUSY:
        return HC_ERR_BUSY;
    default:
        return HC_ERR_UNKNOWN;
    }
}

static void hc_hal_uart_dispatch_rx_byte(HC_HAL_UART_Ch_e ch, HC_U8 data)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    if (port == HC_NULL_PTR) {
        return;
    }

    if (port->rx_callback != HC_NULL_FN) {
        port->rx_callback(data);
    } else {
        HC_HAL_UART_RxCpltCallback(ch, data);
    }
}

static void hc_hal_uart_restart_rx(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    if ((port == HC_NULL_PTR) || (port->handle == (UART_HandleTypeDef *)0)) {
        return;
    }

    if ((port->dma_rx_enable == HC_TRUE) && (port->handle->hdmarx != (DMA_HandleTypeDef *)0)) {
        (void)HAL_UART_Receive_DMA(port->handle, &port->rx_byte, 1u);
    } else {
        (void)HAL_UART_Receive_IT(port->handle, &port->rx_byte, 1u);
    }
}

HC_Error_e HC_HAL_UART_Init(HC_VOID)
{
    HC_U32 i;

    for (i = 0u; i < (HC_U32)UART_CH_MAX; i++) {
        s_uart_ports[i].handle = (UART_HandleTypeDef *)g_uartCfgMap[i].inst;
        s_uart_ports[i].irqn = (IRQn_Type)g_uartCfgMap[i].irqNum;
        s_uart_ports[i].rx_callback = HC_NULL_FN;
        s_uart_ports[i].tx_callback = HC_NULL_FN;
        s_uart_ports[i].dma_tx_enable = g_uartCfgMap[i].dmaTxEnable;
        s_uart_ports[i].dma_rx_enable = g_uartCfgMap[i].dmaRxEnable;
        s_uart_ports[i].tx_busy = HC_FALSE;
        s_uart_ports[i].rx_byte = 0u;
        memset(s_uart_ports[i].tx_buffer, 0, sizeof(s_uart_ports[i].tx_buffer));

        if ((s_uart_ports[i].handle != (UART_HandleTypeDef *)0) &&
            (g_uartCfgMap[i].rxIrqEnable == HC_TRUE)) {
            hc_hal_uart_restart_rx((HC_HAL_UART_Ch_e)i);
        }
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_SendByte(HC_HAL_UART_Ch_e ch, HC_U8 data)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    HC_HAL_ASSERT_PARAM((port != HC_NULL_PTR) && (port->handle != (UART_HandleTypeDef *)0), HC_ERR_NOT_ENABLE);
    return hc_hal_uart_map_status(HAL_UART_Transmit(port->handle, &data, 1u, HAL_MAX_DELAY));
}

HC_Error_e HC_HAL_UART_SendBuffer(HC_HAL_UART_Ch_e ch, const HC_U8 *p_buf, HC_U32 len)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HAL_StatusTypeDef status;

    HC_HAL_ASSERT_PARAM((port != HC_NULL_PTR) && (port->handle != (UART_HandleTypeDef *)0), HC_ERR_NOT_ENABLE);
    HC_HAL_ASSERT_PARAM((p_buf != HC_NULL_PTR) || (len == 0u), HC_HAL_ERR_NULL_PTR);

    if (len == 0u) {
        return HC_HAL_OK;
    }

    if ((port->dma_tx_enable == HC_TRUE) &&
        (port->handle->hdmatx != (DMA_HandleTypeDef *)0) &&
        (len <= HC_HAL_UART_DMA_TX_BUFFER_SIZE)) {
        if ((port->tx_busy == HC_TRUE) ||
            (port->handle->gState != HAL_UART_STATE_READY)) {
            return HC_ERR_BUSY;
        }

        memcpy(port->tx_buffer, p_buf, (size_t)len);
        port->tx_busy = HC_TRUE;
        status = HAL_UART_Transmit_DMA(port->handle, port->tx_buffer, (HC_U16)len);
        if (status != HAL_OK) {
            port->tx_busy = HC_FALSE;
        }
        return hc_hal_uart_map_status(status);
    }

    return hc_hal_uart_map_status(HAL_UART_Transmit(port->handle, (HC_U8 *)p_buf, (HC_U16)len, HAL_MAX_DELAY));
}

HC_Error_e HC_HAL_UART_SendString(HC_HAL_UART_Ch_e ch, const HC_CHAR *p_str)
{
    HC_HAL_ASSERT_PARAM(p_str != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);
    return HC_HAL_UART_SendBuffer(ch, (const HC_U8 *)p_str, (HC_U32)strlen(p_str));
}

HC_Error_e HC_HAL_UART_RegisterRxCallback(HC_HAL_UART_Ch_e ch, HC_UART_RxCallback_t cb)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    HC_HAL_ASSERT_PARAM(port != HC_NULL_PTR, HC_HAL_ERR_INVALID);
    port->rx_callback = cb;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_RegisterTxCallback(HC_HAL_UART_Ch_e ch, HC_UART_TxCallback_t cb)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    HC_HAL_ASSERT_PARAM(port != HC_NULL_PTR, HC_HAL_ERR_INVALID);
    port->tx_callback = cb;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_SetPrintfChannel(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)UART_CH_MAX, HC_HAL_ERR_INVALID);
    s_printf_channel = ch;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_Printf(HC_HAL_UART_Ch_e ch, const HC_CHAR *fmt, ...)
{
    HC_CHAR buffer[HC_HAL_UART_PRINTF_BUFFER_SIZE];
    va_list args;
    HC_S32 length;

    HC_HAL_ASSERT_PARAM(fmt != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);

    va_start(args, fmt);
    length = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (length < 0) {
        return HC_ERR_UNKNOWN;
    }

    if ((HC_U32)length >= (HC_U32)sizeof(buffer)) {
        length = (HC_S32)(sizeof(buffer) - 1u);
    }

    return HC_HAL_UART_SendBuffer(ch, (const HC_U8 *)buffer, (HC_U32)length);
}

HC_VOID HC_HAL_UART_IRQHandler(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    if ((port != HC_NULL_PTR) && (port->handle != (UART_HandleTypeDef *)0)) {
        HAL_UART_IRQHandler(port->handle);
    }
}

HC_WEAK HC_VOID HC_HAL_UART_ErrorCallback(HC_HAL_UART_Ch_e ch)
{
    HC_UNUSED(ch);
}

HC_WEAK HC_VOID HC_HAL_UART_RxCpltCallback(HC_HAL_UART_Ch_e ch, HC_U8 data)
{
    HC_UNUSED(ch);
    HC_UNUSED(data);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HC_HAL_UART_Ch_e ch = hc_hal_uart_find_channel(huart);

    if ((HC_U32)ch >= (HC_U32)UART_CH_MAX) {
        return;
    }

    hc_hal_uart_dispatch_rx_byte(ch, s_uart_ports[ch].rx_byte);
    hc_hal_uart_restart_rx(ch);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HC_HAL_UART_Ch_e ch = hc_hal_uart_find_channel(huart);
    HC_HAL_UART_Port_t *port;

    if ((HC_U32)ch >= (HC_U32)UART_CH_MAX) {
        return;
    }

    port = hc_hal_uart_get_port(ch);
    if (port == HC_NULL_PTR) {
        return;
    }

    port->tx_busy = HC_FALSE;
    if (port->tx_callback != HC_NULL_FN) {
        port->tx_callback(ch);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HC_HAL_UART_Ch_e ch = hc_hal_uart_find_channel(huart);

    if ((HC_U32)ch >= (HC_U32)UART_CH_MAX) {
        return;
    }

    HC_HAL_UART_ErrorCallback(ch);
    s_uart_ports[ch].tx_busy = HC_FALSE;
    hc_hal_uart_restart_rx(ch);
}
