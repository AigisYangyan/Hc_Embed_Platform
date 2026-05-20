/**
 * @file    mspm0g3507_uart_cfg.c
 * @brief   MSPM0G3507 UART 板级配置，定义串口实例与默认参数。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_uart.h"
#include "hc_hal_dma.h"
#include "ti_msp_dl_config.h"

const HC_UART_Cfg_t g_uartCfgMap[UART_CH_MAX] = {
    [UART_CH_STEPMOTOR] = {
        .inst = (HC_VOID *)UART_STEPMOTOR_HORIZON_INST,
        .iomuxTx = GPIO_UART_STEPMOTOR_HORIZON_IOMUX_TX,
        .iomuxTxFunc = GPIO_UART_STEPMOTOR_HORIZON_IOMUX_TX_FUNC,
        .iomuxRx = GPIO_UART_STEPMOTOR_HORIZON_IOMUX_RX,
        .iomuxRxFunc = GPIO_UART_STEPMOTOR_HORIZON_IOMUX_RX_FUNC,
        .clkFreqHz = UART_STEPMOTOR_HORIZON_INST_FREQUENCY,
        .baudRate = UART_STEPMOTOR_HORIZON_BAUD_RATE,
        .ibrd = UART_STEPMOTOR_HORIZON_IBRD_40_MHZ_921600_BAUD,
        .fbrd = UART_STEPMOTOR_HORIZON_FBRD_40_MHZ_921600_BAUD,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_FALSE,
        .irqNum = UART_STEPMOTOR_HORIZON_INST_INT_IRQN,
        .dmaTxEnable = HC_TRUE,
        .fifoEnable = HC_TRUE,
        .dmaTxChIdx = (HC_U8)DMA_CH_STEPMOTOR_TX,
        .dmaRxEnable = HC_TRUE,
        .dmaRxChIdx = (HC_U8)DMA_CH_STEPMOTOR_RX,
        .dmaRxTransferSize = 1u,
    },
    [UART_CH_VOFA] = {
        .inst = (HC_VOID *)UART_VOFA_INST,
        .iomuxTx = GPIO_UART_VOFA_IOMUX_TX,
        .iomuxTxFunc = GPIO_UART_VOFA_IOMUX_TX_FUNC,
        .iomuxRx = GPIO_UART_VOFA_IOMUX_RX,
        .iomuxRxFunc = GPIO_UART_VOFA_IOMUX_RX_FUNC,
        .clkFreqHz = UART_VOFA_INST_FREQUENCY,
        .baudRate = UART_VOFA_BAUD_RATE,
        .ibrd = UART_VOFA_IBRD_80_MHZ_115200_BAUD,
        .fbrd = UART_VOFA_FBRD_80_MHZ_115200_BAUD,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_FALSE,
        .irqNum = UART_VOFA_INST_INT_IRQN,
        .dmaTxEnable = HC_TRUE,
        .fifoEnable = HC_TRUE,
        .dmaTxChIdx = (HC_U8)DMA_CH_VOFA_TX,
        .dmaRxEnable = HC_TRUE,
        .dmaRxChIdx = (HC_U8)DMA_CH_VOFA_RX,
        .dmaRxTransferSize = 1u,
    },
    [UART_CH_VISION] = {
        .inst = (HC_VOID *)UART_Vision_INST,
        .iomuxTx = GPIO_UART_Vision_IOMUX_TX,
        .iomuxTxFunc = GPIO_UART_Vision_IOMUX_TX_FUNC,
        .iomuxRx = GPIO_UART_Vision_IOMUX_RX,
        .iomuxRxFunc = GPIO_UART_Vision_IOMUX_RX_FUNC,
        .clkFreqHz = UART_Vision_INST_FREQUENCY,
        .baudRate = UART_Vision_BAUD_RATE,
        .ibrd = UART_Vision_IBRD_40_MHZ_115200_BAUD,
        .fbrd = UART_Vision_FBRD_40_MHZ_115200_BAUD,
        .dataBits = HC_UART_DATABITS_8,
        .stopBits = HC_UART_STOPBITS_1,
        .parity = HC_UART_PARITY_NONE,
        .rxIrqEnable = HC_FALSE,
        .irqNum = UART_Vision_INST_INT_IRQN,
        .dmaTxEnable = HC_TRUE,
        .fifoEnable = HC_TRUE,
        .dmaTxChIdx = (HC_U8)DMA_CH_VISION_TX,
        .dmaRxEnable = HC_TRUE,
        .dmaRxChIdx = (HC_U8)DMA_CH_VISION_RX,
        .dmaRxTransferSize = 1u,
    },
};
