/**
 * @file    hc_hal_uart.c
 * @brief   UART HAL 接口与实现，提供串口收发、回调注册与打印通道管理。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_uart.h"
#include "hc_hal_dma.h"
#include "ti_msp_dl_config.h"
#include <string.h>
#include <ti/driverlib/dl_uart.h>
#include <stdio.h>

/* ============================================================================
 *  1. 模块私有数据与缓冲大小
 * ========================================================================== */

/* cfg 层定义的 UART 外设配置表。 */
extern const HC_UART_Cfg_t g_uartCfgMap[UART_CH_MAX];

#define HC_HAL_UART_DMA_TX_BUFFER_SIZE  512u   /**< 单次 DMA TX 缓冲上限 */
#define HC_HAL_UART_DMA_RX_BUFFER_SIZE  16u    /**< DMA RX 环形缓冲大小 */
#define HC_HAL_UART_PRINTF_BUFFER_SIZE  128u   /**< printf 聚合发送缓冲大小 */

/** @brief 单个 UART 逻辑通道的运行时状态 (绑定实例/回调/DMA 缓冲)。 */
typedef struct {
    UART_Regs *base;
    IRQn_Type irqn;
    HC_UART_RxCallback_t rx_callback;
    HC_UART_TxCallback_t tx_callback;
    HC_Bool_e dma_tx_enable;
    HC_Bool_e dma_rx_enable;
    HC_HAL_DMA_Ch_e dma_tx_ch;
    HC_HAL_DMA_Ch_e dma_rx_ch;
    HC_U16 dma_rx_transfer_size;
    HC_U8 dma_rx_buffer[HC_HAL_UART_DMA_RX_BUFFER_SIZE];
    HC_U8 dma_tx_buffer[HC_HAL_UART_DMA_TX_BUFFER_SIZE];
} HC_HAL_UART_Port_t;

static HC_HAL_UART_Port_t s_uart_ports[UART_CH_MAX];

/* printf / fputc 重定向相关的静态状态。 */
static HC_HAL_UART_Ch_e s_printf_channel = UART_CH_VOFA;
static HC_U8 s_printf_tx_buffer[HC_HAL_UART_PRINTF_BUFFER_SIZE];
static HC_U16 s_printf_tx_length = 0u;

/* ============================================================================
 *  2. 内部辅助函数前向声明
 * ========================================================================== */
static void hc_hal_uart_dma_tx_callback_stepmotor(void);
static void hc_hal_uart_dma_tx_callback_vofa(void);
static void hc_hal_uart_dma_tx_callback_vision(void);
static void hc_hal_uart_dma_rx_callback_stepmotor(void);
static void hc_hal_uart_dma_rx_callback_vofa(void);
static void hc_hal_uart_dma_rx_callback_vision(void);
static void hc_hal_uart_drain_rx_fifo(HC_HAL_UART_Ch_e ch);

/** @brief 根据逻辑通道 ID 返回内部状态结构指针，越界返回 NULL。 */
static HC_HAL_UART_Port_t *hc_hal_uart_get_port(HC_HAL_UART_Ch_e ch)
{
    if ((HC_U32)ch >= (HC_U32)UART_CH_MAX) {
        return HC_NULL_PTR;
    }

    return &s_uart_ports[ch];
}

static const HC_DMA_Callback_t s_uart_dma_tx_callbacks[UART_CH_MAX] = {
    hc_hal_uart_dma_tx_callback_stepmotor,
    hc_hal_uart_dma_tx_callback_vofa,
    hc_hal_uart_dma_tx_callback_vision,
};

static const HC_DMA_Callback_t s_uart_dma_rx_callbacks[UART_CH_MAX] = {
    hc_hal_uart_dma_rx_callback_stepmotor,
    hc_hal_uart_dma_rx_callback_vofa,
    hc_hal_uart_dma_rx_callback_vision,
};

/* ============================================================================
 *  3. printf 聚合发送 / 接收分发 / DMA 接收管理
 * ========================================================================== */

/** @brief 将 printf 聚合缓冲写入 UART (满/换行时自动触发)。 */
static void hc_hal_uart_flush_printf_buffer(void)
{
    if (s_printf_tx_length == 0u) {
        return;
    }

    (void)HC_HAL_UART_SendBuffer(s_printf_channel,
                                 s_printf_tx_buffer,
                                 (HC_U32)s_printf_tx_length);
    s_printf_tx_length = 0u;
}

/** @brief 将接收到的单字节分发到业务回调或弱符号入口。 */
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

/** @brief 在中断中排空 UART RX FIFO 并逐字节分发给业务。 */
static void hc_hal_uart_dispatch_rx(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HC_U8 data = 0u;

    if ((port == HC_NULL_PTR) || (port->base == HC_NULL_PTR)) {
        return;
    }

    while (DL_UART_receiveDataCheck(port->base, &data)) {
        hc_hal_uart_dispatch_rx_byte(ch, data);
    }

    DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_RX);
}

static void hc_hal_uart_drain_rx_fifo(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HC_U8 data = 0u;

    if ((port == HC_NULL_PTR) || (port->base == HC_NULL_PTR)) {
        return;
    }

    while (DL_UART_receiveDataCheck(port->base, &data)) {
        hc_hal_uart_dispatch_rx_byte(ch, data);
    }
}

/** @brief (重新) 启动指定通道的 DMA 接收传输到 rx_buffer。 */
static HC_Error_e hc_hal_uart_start_dma_rx(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);

    HC_HAL_ASSERT_PARAM(port != HC_NULL_PTR, HC_HAL_ERR_INVALID);
    HC_HAL_ASSERT_PARAM(port->base != HC_NULL_PTR, HC_ERR_NOT_ENABLE);
    HC_HAL_ASSERT_PARAM(port->dma_rx_enable == HC_TRUE, HC_HAL_ERR_NOT_PERM);
    HC_HAL_ASSERT_PARAM((port->dma_rx_transfer_size > 0u) &&
                        (port->dma_rx_transfer_size <= HC_HAL_UART_DMA_RX_BUFFER_SIZE),
                        HC_HAL_ERR_INVALID);

    DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_RX);

    return HC_HAL_DMA_StartTransfer(port->dma_rx_ch,
                                    (HC_U32)(uintptr_t)&port->base->RXDATA,
                                    (HC_U32)(uintptr_t)&port->dma_rx_buffer[0],
                                    port->dma_rx_transfer_size);
}

/** @brief DMA TX 完成处理：清中断标志 + 调用业务 tx_callback。 */
static void hc_hal_uart_handle_dma_tx_complete(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HC_UART_TxCallback_t tx_callback = HC_NULL_FN;

    if ((port == HC_NULL_PTR) || (port->base == HC_NULL_PTR)) {
        return;
    }

    DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_TX);

    tx_callback = port->tx_callback;
    if (tx_callback != HC_NULL_FN) {
        tx_callback(ch);
    }
}

/**
 * @brief DMA RX 完成处理：遍历 rx_buffer 分发 + 重新启动 DMA RX。
 *
 * 做法：遍历 dma_rx_transfer_size 字节 → 再排空 FIFO 残留 → 重新 arm DMA。
 * 这样既支持字节流语义，又避免在 arm 窗口内丢失字节。
 */
static void hc_hal_uart_handle_dma_rx_complete(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HC_U16 index = 0u;

    if ((port == HC_NULL_PTR) || (port->base == HC_NULL_PTR)) {
        return;
    }

    DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_RX);

    for (index = 0u; index < port->dma_rx_transfer_size; index++) {
        hc_hal_uart_dispatch_rx_byte(ch, port->dma_rx_buffer[index]);
    }

    /* Drain any bytes that already entered the UART FIFO while DMA was rearming. */
    hc_hal_uart_drain_rx_fifo(ch);

    (void)hc_hal_uart_start_dma_rx(ch);
}

static void hc_hal_uart_dma_tx_callback_stepmotor(void)
{
    hc_hal_uart_handle_dma_tx_complete(UART_CH_STEPMOTOR);
}

static void hc_hal_uart_dma_tx_callback_vofa(void)
{
    hc_hal_uart_handle_dma_tx_complete(UART_CH_VOFA);
}

static void hc_hal_uart_dma_tx_callback_vision(void)
{
    hc_hal_uart_handle_dma_tx_complete(UART_CH_VISION);
}

static void hc_hal_uart_dma_rx_callback_stepmotor(void)
{
    hc_hal_uart_handle_dma_rx_complete(UART_CH_STEPMOTOR);
}

static void hc_hal_uart_dma_rx_callback_vofa(void)
{
    hc_hal_uart_handle_dma_rx_complete(UART_CH_VOFA);
}

static void hc_hal_uart_dma_rx_callback_vision(void)
{
    hc_hal_uart_handle_dma_rx_complete(UART_CH_VISION);
}

/* ============================================================================
 *  4. 公开 API 实现 (详细语义见 hc_hal_uart.h)
 * ========================================================================== */

HC_Error_e HC_HAL_UART_Init(HC_VOID)
{
    HC_U8 i;

    for (i = 0u; i < (HC_U8)UART_CH_MAX; i++) {
        s_uart_ports[i].base = (UART_Regs *)g_uartCfgMap[i].inst;
        s_uart_ports[i].irqn = (IRQn_Type)g_uartCfgMap[i].irqNum;
        s_uart_ports[i].rx_callback = HC_NULL_FN;
        s_uart_ports[i].tx_callback = HC_NULL_FN;
        s_uart_ports[i].dma_tx_enable = g_uartCfgMap[i].dmaTxEnable;
        s_uart_ports[i].dma_rx_enable = g_uartCfgMap[i].dmaRxEnable;
        s_uart_ports[i].dma_tx_ch = (HC_HAL_DMA_Ch_e)g_uartCfgMap[i].dmaTxChIdx;
        s_uart_ports[i].dma_rx_ch = (HC_HAL_DMA_Ch_e)g_uartCfgMap[i].dmaRxChIdx;
        s_uart_ports[i].dma_rx_transfer_size = g_uartCfgMap[i].dmaRxTransferSize;
        memset(s_uart_ports[i].dma_rx_buffer, 0, sizeof(s_uart_ports[i].dma_rx_buffer));
        memset(s_uart_ports[i].dma_tx_buffer, 0, sizeof(s_uart_ports[i].dma_tx_buffer));

        if (s_uart_ports[i].base == HC_NULL_PTR) {
            continue;
        }

        if (g_uartCfgMap[i].rxIrqEnable == HC_TRUE) {
            DL_UART_enableInterrupt(s_uart_ports[i].base, DL_UART_INTERRUPT_RX);
        }

        /*
         * The HAL uses single-byte DMA RX. If the FIFO trigger stays at 1/2 full,
         * short text commands can sit in the UART FIFO until more bytes arrive.
         * Force the RX trigger down to one entry so each received byte promptly
         * generates the DMA request expected by the HAL.
         */
        if ((g_uartCfgMap[i].fifoEnable == HC_TRUE) &&
            (s_uart_ports[i].dma_rx_enable == HC_TRUE) &&
            (s_uart_ports[i].dma_rx_transfer_size == 1u)) {
            DL_UART_Main_setRXFIFOThreshold(s_uart_ports[i].base,
                                            DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
        }

        if (s_uart_ports[i].dma_tx_enable == HC_TRUE) {
            (void)HC_HAL_DMA_RegisterCallback(s_uart_ports[i].dma_tx_ch,
                                              s_uart_dma_tx_callbacks[i]);
        }

        if (s_uart_ports[i].dma_rx_enable == HC_TRUE) {
            (void)HC_HAL_DMA_RegisterCallback(s_uart_ports[i].dma_rx_ch,
                                              s_uart_dma_rx_callbacks[i]);
            (void)hc_hal_uart_start_dma_rx((HC_HAL_UART_Ch_e)i);
        }

        if ((g_uartCfgMap[i].rxIrqEnable == HC_TRUE) ||
            (s_uart_ports[i].dma_tx_enable == HC_TRUE) ||
            (s_uart_ports[i].dma_rx_enable == HC_TRUE)) {
            NVIC_ClearPendingIRQ(s_uart_ports[i].irqn);
            NVIC_EnableIRQ(s_uart_ports[i].irqn);
        }
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_SendByte(HC_HAL_UART_Ch_e ch, HC_U8 data)
{
    HC_HAL_UART_Port_t *port;

    port = hc_hal_uart_get_port(ch);
    HC_HAL_ASSERT_PARAM((port != HC_NULL_PTR) && (port->base != HC_NULL_PTR),
                        HC_ERR_NOT_ENABLE);
    DL_UART_transmitDataBlocking(port->base, data);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_SendBuffer(HC_HAL_UART_Ch_e ch, const HC_U8 *p_buf, HC_U32 len)
{
    HC_HAL_UART_Port_t *port;
    HC_U32 i;

    HC_HAL_ASSERT_PARAM(p_buf != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);
    HC_HAL_ASSERT_PARAM(len > 0u, HC_HAL_ERR_INVALID);

    port = hc_hal_uart_get_port(ch);
    HC_HAL_ASSERT_PARAM((port != HC_NULL_PTR) && (port->base != HC_NULL_PTR),
                        HC_ERR_NOT_ENABLE);

    if ((port->dma_tx_enable == HC_TRUE) &&
        (len <= HC_HAL_UART_DMA_TX_BUFFER_SIZE)) {
        if (HC_HAL_DMA_IsBusy(port->dma_tx_ch) == HC_TRUE) {
            while (HC_HAL_DMA_IsBusy(port->dma_tx_ch) == HC_TRUE) {
            }

            for (i = 0u; i < len; i++) {
                HC_Error_e err = HC_HAL_UART_SendByte(ch, p_buf[i]);
                if (err != HC_HAL_OK) {
                    return err;
                }
            }
            return HC_HAL_OK;
        }

        memcpy(port->dma_tx_buffer, p_buf, len);
        DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_TX);

        if (HC_HAL_DMA_StartTransfer(port->dma_tx_ch,
                                     (HC_U32)(uintptr_t)&port->dma_tx_buffer[0],
                                     (HC_U32)(uintptr_t)&port->base->TXDATA,
                                     (HC_U16)len) == HC_HAL_OK) {
            return HC_HAL_OK;
        }
    }

    for (i = 0u; i < len; i++) {
        HC_Error_e err = HC_HAL_UART_SendByte(ch, p_buf[i]);
        if (err != HC_HAL_OK) {
            return err;
        }
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_SendString(HC_HAL_UART_Ch_e ch, const HC_CHAR *p_str)
{
    HC_HAL_ASSERT_PARAM(p_str != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);

    while (*p_str != '\0') {
        HC_Error_e err = HC_HAL_UART_SendByte(ch, (HC_U8)(*p_str));
        if (err != HC_HAL_OK) {
            return err;
        }
        p_str++;
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_RegisterRxCallback(HC_HAL_UART_Ch_e ch, HC_UART_RxCallback_t cb)
{
    HC_HAL_UART_Port_t *port;

    port = hc_hal_uart_get_port(ch);
    HC_HAL_ASSERT_PARAM(port != HC_NULL_PTR, HC_HAL_ERR_INVALID);

    port->rx_callback = cb;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_UART_RegisterTxCallback(HC_HAL_UART_Ch_e ch, HC_UART_TxCallback_t cb)
{
    HC_HAL_UART_Port_t *port;

    port = hc_hal_uart_get_port(ch);
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
    HC_CHAR buffer[256];
    va_list args;
    HC_S32 len;

    HC_HAL_ASSERT_PARAM(fmt != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);

    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len <= 0) {
        return HC_HAL_OK;
    }

    if ((HC_U32)len > (sizeof(buffer) - 1u)) {
        len = (HC_S32)(sizeof(buffer) - 1u);
    }

    return HC_HAL_UART_SendBuffer(ch, (const HC_U8 *)buffer, (HC_U32)len);
}

/* 默认弱符号：未注册 Rx 回调时收到的每个字节都会到达这里。 */
HC_WEAK HC_VOID HC_HAL_UART_RxCpltCallback(HC_HAL_UART_Ch_e ch, HC_U8 data)
{
    HC_UNUSED(ch);
    HC_UNUSED(data);
}

/* 默认弱符号：UART 错误回调，业务层可 Override。 */
HC_WEAK HC_VOID HC_HAL_UART_ErrorCallback(HC_HAL_UART_Ch_e ch)
{
    HC_UNUSED(ch);
}

/* ============================================================================
 *  5. 中断服务与 stdout 重定向
 * ========================================================================== */

HC_VOID HC_HAL_UART_IRQHandler(HC_HAL_UART_Ch_e ch)
{
    HC_HAL_UART_Port_t *port = hc_hal_uart_get_port(ch);
    HC_U32 irq_status = 0u;

    if ((port == HC_NULL_PTR) || (port->base == HC_NULL_PTR)) {
        return;
    }

    irq_status = DL_UART_getEnabledInterruptStatus(
        port->base,
        DL_UART_INTERRUPT_RX |
        DL_UART_INTERRUPT_DMA_DONE_RX |
        DL_UART_INTERRUPT_DMA_DONE_TX);

    if ((irq_status & DL_UART_INTERRUPT_RX) != 0u) {
        hc_hal_uart_dispatch_rx(ch);
    }

    if ((irq_status & DL_UART_INTERRUPT_DMA_DONE_RX) != 0u) {
        DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_RX);
    }

    if ((irq_status & DL_UART_INTERRUPT_DMA_DONE_TX) != 0u) {
        DL_UART_clearInterruptStatus(port->base, DL_UART_INTERRUPT_DMA_DONE_TX);
    }
}

#if defined(__clang__) || defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
int fputc(int ch, FILE *f)
{
    HC_UNUSED(f);

    if (s_printf_tx_length >= HC_HAL_UART_PRINTF_BUFFER_SIZE) {
        hc_hal_uart_flush_printf_buffer();
    }

    s_printf_tx_buffer[s_printf_tx_length++] = (HC_U8)ch;
    if (((HC_U8)ch == (HC_U8)'\n') ||
        (s_printf_tx_length >= HC_HAL_UART_PRINTF_BUFFER_SIZE)) {
        hc_hal_uart_flush_printf_buffer();
    }

    return ch;
}
#elif defined(__GNUC__)
int _write(int fd, const char *ptr, int len)
{
    HC_UNUSED(fd);
    (void)HC_HAL_UART_SendBuffer(s_printf_channel, (const HC_U8 *)ptr, (HC_U32)len);
    return len;
}
#endif

HC_IRQ_HANDLER(UART_STEPMOTOR_HORIZON_INST_IRQHandler)
{
    HC_HAL_UART_IRQHandler(UART_CH_STEPMOTOR);
}

HC_IRQ_HANDLER(UART_VOFA_INST_IRQHandler)
{
    HC_HAL_UART_IRQHandler(UART_CH_VOFA);
}

HC_IRQ_HANDLER(UART_Vision_INST_IRQHandler)
{
    HC_HAL_UART_IRQHandler(UART_CH_VISION);
}
