#ifndef HC_HAL_DMA_H
#define HC_HAL_DMA_H

/* Legacy compatibility header — forward to canonical hc_hal. */
#include "hc_hal/hc_hal_dma.h"

/* Project-semantic aliases (deprecated). New code should use HC_HAL_DMA_Ch_e. */
typedef enum {
    DMA_CH_STEPMOTOR_RX = HC_HAL_DMA_CH_0,
    DMA_CH_VOFA_TX      = HC_HAL_DMA_CH_1,
    DMA_CH_STEPMOTOR_TX = HC_HAL_DMA_CH_2,
    DMA_CH_VOFA_RX      = HC_HAL_DMA_CH_3,
    DMA_CH_VISION_RX    = HC_HAL_DMA_CH_4,
    DMA_CH_VISION_TX    = HC_HAL_DMA_CH_5,
    DMA_CH_MAX          = HC_HAL_DMA_CH_MAX
} HC_HAL_DMA_Ch_e;

#endif /* HC_HAL_DMA_H */
