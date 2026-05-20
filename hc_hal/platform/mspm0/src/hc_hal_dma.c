/**
 * @file    hc_hal_dma.c
 * @brief   DMA HAL 接口与实现，提供通道初始化、状态查询与回调处理。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_dma.h"
#include "ti_msp_dl_config.h"
#include <string.h>
#include <ti/driverlib/dl_dma.h>

/* ============================================================================
 *  1. 模块私有数据
 * ========================================================================== */

/* cfg 层定义的 DMA 通道配置表。 */
extern const HC_DMA_ChCfg_t g_dmaCfgMap[DMA_CH_MAX];

/* 每个逻辑通道的完成回调与忙状态。 */
static HC_DMA_Callback_t s_dma_callbacks[DMA_CH_MAX] = { HC_NULL_FN };
static HC_Bool_e s_dma_busy[DMA_CH_MAX] = { HC_FALSE };

/* ============================================================================
 *  2. 内部工具函数
 * ========================================================================== */

/** @brief 根据逻辑通道 ID 返回 cfg 指针，越界返回 NULL。 */
static const HC_DMA_ChCfg_t *hc_hal_dma_get_cfg(HC_HAL_DMA_Ch_e ch)
{
    if ((HC_U32)ch >= (HC_U32)DMA_CH_MAX) {
        return HC_NULL_PTR;
    }

    return &g_dmaCfgMap[ch];
}

/** @brief 物理通道号 → DriverLib DMA 中断掩码映射 (0~6)。 */
static HC_U32 hc_hal_dma_get_irq_mask(HC_U8 dma_ch_num)
{
    switch (dma_ch_num) {
    case 0u:
        return DL_DMA_INTERRUPT_CHANNEL0;
    case 1u:
        return DL_DMA_INTERRUPT_CHANNEL1;
    case 2u:
        return DL_DMA_INTERRUPT_CHANNEL2;
    case 3u:
        return DL_DMA_INTERRUPT_CHANNEL3;
    case 4u:
        return DL_DMA_INTERRUPT_CHANNEL4;
    case 5u:
        return DL_DMA_INTERRUPT_CHANNEL5;
    case 6u:
        return DL_DMA_INTERRUPT_CHANNEL6;
    default:
        return 0u;
    }
}

/* ============================================================================
 *  3. 公开 API 实现 (详细语义见 hc_hal_dma.h)
 * ========================================================================== */

HC_Error_e HC_HAL_DMA_Init(HC_VOID)
{
    HC_U32 index;

    memset(s_dma_callbacks, 0, sizeof(s_dma_callbacks));
    memset(s_dma_busy, 0, sizeof(s_dma_busy));

    for (index = 0u; index < (HC_U32)DMA_CH_MAX; index++) {
        const HC_DMA_ChCfg_t *cfg = &g_dmaCfgMap[index];
        HC_U32 irq_mask = hc_hal_dma_get_irq_mask(cfg->dmaChNum);

        if ((cfg->irqEnable == HC_TRUE) && (irq_mask != 0u)) {
            DL_DMA_clearInterruptStatus(DMA, irq_mask);
            DL_DMA_enableInterrupt(DMA, irq_mask);
        }
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_DMA_StartTransfer(HC_HAL_DMA_Ch_e ch,
                                    HC_U32 src_addr,
                                    HC_U32 dest_addr,
                                    HC_U16 size)
{
    const HC_DMA_ChCfg_t *cfg;
    HC_U32 irq_mask;

    HC_HAL_ASSERT_PARAM(size > 0u, HC_HAL_ERR_INVALID);

    cfg = hc_hal_dma_get_cfg(ch);
    HC_HAL_ASSERT_PARAM(cfg != HC_NULL_PTR, HC_HAL_ERR_INVALID);
    HC_HAL_ASSERT_PARAM(src_addr != 0u, HC_HAL_ERR_NULL_PTR);
    HC_HAL_ASSERT_PARAM(dest_addr != 0u, HC_HAL_ERR_NULL_PTR);

    if (HC_HAL_DMA_IsBusy(ch) == HC_TRUE) {
        return HC_ERR_BUSY;
    }

    irq_mask = hc_hal_dma_get_irq_mask(cfg->dmaChNum);
    DL_DMA_disableChannel(DMA, cfg->dmaChNum);
    DL_DMA_setSrcAddr(DMA, cfg->dmaChNum, src_addr);
    DL_DMA_setDestAddr(DMA, cfg->dmaChNum, dest_addr);
    DL_DMA_setTransferSize(DMA, cfg->dmaChNum, size);

    if ((cfg->irqEnable == HC_TRUE) && (irq_mask != 0u)) {
        DL_DMA_clearInterruptStatus(DMA, irq_mask);
    }

    s_dma_busy[ch] = HC_TRUE;
    DL_DMA_enableChannel(DMA, cfg->dmaChNum);

    return HC_HAL_OK;
}

HC_Bool_e HC_HAL_DMA_IsBusy(HC_HAL_DMA_Ch_e ch)
{
    const HC_DMA_ChCfg_t *cfg;

    if ((HC_U32)ch >= (HC_U32)DMA_CH_MAX) {
        return HC_FALSE;
    }

    cfg = &g_dmaCfgMap[ch];
    if ((s_dma_busy[ch] == HC_TRUE) &&
        (DL_DMA_getTransferSize(DMA, cfg->dmaChNum) == 0u)) {
        s_dma_busy[ch] = HC_FALSE;
    }

    return s_dma_busy[ch];
}

HC_Error_e HC_HAL_DMA_RegisterCallback(HC_HAL_DMA_Ch_e ch, HC_DMA_Callback_t cb)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)DMA_CH_MAX, HC_HAL_ERR_INVALID);
    s_dma_callbacks[ch] = cb;
    return HC_HAL_OK;
}

/* ============================================================================
 *  4. 中断服务 (MSPM0 DMA 所有通道共享一条中断线)
 * ========================================================================== */

HC_VOID HC_HAL_DMA_IRQHandler(HC_VOID)
{
    HC_U32 pending_irq = DL_DMA_getEnabledInterruptStatus(DMA, 0xFFFFFFFFu);
    HC_U32 index;

    if (pending_irq == 0u) {
        return;
    }

    for (index = 0u; index < (HC_U32)DMA_CH_MAX; index++) {
        const HC_DMA_ChCfg_t *cfg = &g_dmaCfgMap[index];
        HC_U32 irq_mask = hc_hal_dma_get_irq_mask(cfg->dmaChNum);

        if ((irq_mask != 0u) && ((pending_irq & irq_mask) != 0u)) {
            DL_DMA_clearInterruptStatus(DMA, irq_mask);
            s_dma_busy[index] = HC_FALSE;

            if (s_dma_callbacks[index] != HC_NULL_FN) {
                s_dma_callbacks[index]();
            }
        }
    }
}

HC_IRQ_HANDLER(DMA_IRQHandler)
{
    HC_HAL_DMA_IRQHandler();
}
