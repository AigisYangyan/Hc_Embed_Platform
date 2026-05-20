/**
 * @file    mspm0g3507_dma_cfg.c
 * @brief   MSPM0G3507 DMA 板级配置，定义 DMA 通道与触发映射。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_dma.h"
#include "ti_msp_dl_config.h"

const HC_DMA_ChCfg_t g_dmaCfgMap[DMA_CH_MAX] = {
    [DMA_CH_STEPMOTOR_RX] = {
        .dmaChNum = DMA_CH0_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_UNCHANGED,
        .destIncrement = HC_DMA_ADDR_INCREMENT,
        .trigger = UART_STEPMOTOR_HORIZON_INST_DMA_TRIGGER_0,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
    [DMA_CH_VOFA_TX] = {
        .dmaChNum = DMA_CH1_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_INCREMENT,
        .destIncrement = HC_DMA_ADDR_UNCHANGED,
        .trigger = UART_VOFA_INST_DMA_TRIGGER_0,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
    [DMA_CH_STEPMOTOR_TX] = {
        .dmaChNum = DMA_CH2_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_INCREMENT,
        .destIncrement = HC_DMA_ADDR_UNCHANGED,
        .trigger = UART_STEPMOTOR_HORIZON_INST_DMA_TRIGGER_1,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
    [DMA_CH_VOFA_RX] = {
        .dmaChNum = DMA_CH5_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_UNCHANGED,
        .destIncrement = HC_DMA_ADDR_INCREMENT,
        .trigger = UART_VOFA_INST_DMA_TRIGGER_1,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
    [DMA_CH_VISION_RX] = {
        .dmaChNum = DMA_CH7_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_UNCHANGED,
        .destIncrement = HC_DMA_ADDR_INCREMENT,
        .trigger = UART_Vision_INST_DMA_TRIGGER_1,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
    [DMA_CH_VISION_TX] = {
        .dmaChNum = DMA_CH8_CHAN_ID,
        .transferMode = HC_DMA_TRANSFER_SINGLE,
        .srcWidth = HC_DMA_WIDTH_BYTE,
        .destWidth = HC_DMA_WIDTH_BYTE,
        .srcIncrement = HC_DMA_ADDR_INCREMENT,
        .destIncrement = HC_DMA_ADDR_UNCHANGED,
        .trigger = UART_Vision_INST_DMA_TRIGGER_0,
        .triggerExternal = HC_TRUE,
        .irqEnable = HC_TRUE,
    },
};
