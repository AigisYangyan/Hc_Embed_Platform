#include "hc_hal_uart.h"
#include "hc_hal_dma.h"
#include "usart.h"

/* STM32 UART 逻辑通道绑定表。
 * 约定：
 * - UART2 继续给步进电机总线
 * - UART3 统一承接当前调试 / VOFA / printf，并复用 CubeMX 生成的 USART3 DMA
 * - UART1 当前切换为 drive_odom_proto（替代旧 Vision UART1 接入） */
const HC_UART_Cfg_t g_uartCfgMap[UART_CH_MAX] = {
    [UART_CH_STEPMOTOR] = {
        .inst = (HC_VOID *)&huart2,
        .baudRate = 9600u,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_TRUE,
        .irqNum = (HC_U32)USART2_IRQn,
        .dmaTxEnable = HC_FALSE,
        .fifoEnable = HC_FALSE,
        .dmaTxChIdx = 0u,
        .dmaRxEnable = HC_FALSE,
        .dmaRxChIdx = 0u,
        .dmaRxTransferSize = 1u,
    },
    [UART_CH_VOFA] = {
        .inst = (HC_VOID *)&huart3,
        .baudRate = 115200u,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_TRUE,
        .irqNum = (HC_U32)USART3_IRQn,
        .dmaTxEnable = HC_TRUE,
        .fifoEnable = HC_FALSE,
        .dmaTxChIdx = (HC_U8)DMA_CH_VOFA_TX,
        /* RX 侧临时降级为 IT 单字节接收，避开 DMA RX 链路耦合问题。 */
        .dmaRxEnable = HC_FALSE,
        .dmaRxChIdx = (HC_U8)DMA_CH_VOFA_RX,
        .dmaRxTransferSize = 1u,
    },
    [UART_CH_VISION] = {
        .inst = (HC_VOID *)&huart1,
        .baudRate = 115200u,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_TRUE,
        .irqNum = (HC_U32)USART1_IRQn,
        .dmaTxEnable = HC_FALSE,
        .fifoEnable = HC_FALSE,
        .dmaTxChIdx = 0u,
        .dmaRxEnable = HC_FALSE,
        .dmaRxChIdx = 0u,
        .dmaRxTransferSize = 1u,
    },
};
