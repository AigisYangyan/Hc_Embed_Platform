#include "hc_hal_dma.h"

static HC_Bool_e s_dma_initialized = HC_FALSE;
static HC_DMA_Callback_t s_dma_callbacks[DMA_CH_MAX];

HC_Error_e HC_HAL_DMA_Init(HC_VOID)
{
    HC_U32 index;

    for (index = 0u; index < (HC_U32)DMA_CH_MAX; index++) {
        s_dma_callbacks[index] = HC_NULL_FN;
    }

    s_dma_initialized = HC_TRUE;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_DMA_StartTransfer(HC_HAL_DMA_Ch_e ch,
                                    HC_U32 src_addr,
                                    HC_U32 dest_addr,
                                    HC_U16 size)
{
    HC_UNUSED(ch);
    HC_UNUSED(src_addr);
    HC_UNUSED(dest_addr);
    HC_UNUSED(size);

    if (s_dma_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    return HC_ERR_NOT_ENABLE;
}

HC_Bool_e HC_HAL_DMA_IsBusy(HC_HAL_DMA_Ch_e ch)
{
    HC_UNUSED(ch);
    return HC_FALSE;
}

HC_Error_e HC_HAL_DMA_RegisterCallback(HC_HAL_DMA_Ch_e ch, HC_DMA_Callback_t cb)
{
    HC_HAL_ASSERT_PARAM((HC_U32)ch < (HC_U32)DMA_CH_MAX, HC_HAL_ERR_INVALID);
    s_dma_callbacks[ch] = cb;
    return HC_HAL_OK;
}

HC_VOID HC_HAL_DMA_IRQHandler(HC_VOID)
{
}
