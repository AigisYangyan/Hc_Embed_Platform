#ifndef HC_HAL_DMA_H
#define HC_HAL_DMA_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"
#include "hc_common/hc_def.h"
#include "hc_hal_dma_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Neutral DMA channel identifiers. */
typedef enum {
    HC_HAL_DMA_CH_0 = 0,
    HC_HAL_DMA_CH_1 = 1,
    HC_HAL_DMA_CH_2 = 2,
    HC_HAL_DMA_CH_3 = 3,
    HC_HAL_DMA_CH_4 = 4,
    HC_HAL_DMA_CH_5 = 5,
    HC_HAL_DMA_CH_MAX
} HC_HAL_DMA_Ch_e;

HC_Error_e HC_HAL_DMA_Init(HC_VOID);
HC_Error_e HC_HAL_DMA_StartTransfer(HC_HAL_DMA_Ch_e ch, HC_U32 src_addr,
                                    HC_U32 dest_addr, HC_U16 size);
HC_Bool_e  HC_HAL_DMA_IsBusy(HC_HAL_DMA_Ch_e ch);
HC_Error_e HC_HAL_DMA_RegisterCallback(HC_HAL_DMA_Ch_e ch, HC_DMA_Callback_t cb);
HC_VOID    HC_HAL_DMA_IRQHandler(HC_VOID);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_DMA_H */
