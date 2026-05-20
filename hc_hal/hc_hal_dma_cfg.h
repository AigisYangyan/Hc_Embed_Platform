#ifndef HC_HAL_DMA_CFG_H
#define HC_HAL_DMA_CFG_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_DMA_TRANSFER_SINGLE = 0,
    HC_DMA_TRANSFER_BLOCK  = 1
} HC_DMA_TransferMode_e;

typedef enum {
    HC_DMA_WIDTH_BYTE      = 0,
    HC_DMA_WIDTH_HALF_WORD = 1,
    HC_DMA_WIDTH_WORD      = 2,
    HC_DMA_WIDTH_LONG      = 3
} HC_DMA_Width_e;

typedef enum {
    HC_DMA_ADDR_UNCHANGED  = 0,
    HC_DMA_ADDR_INCREMENT  = 1,
    HC_DMA_ADDR_DECREMENT  = 2
} HC_DMA_AddrMode_e;

typedef struct {
    HC_U8                   dmaChNum;
    HC_DMA_TransferMode_e   transferMode;
    HC_DMA_Width_e          srcWidth;
    HC_DMA_Width_e          destWidth;
    HC_DMA_AddrMode_e       srcIncrement;
    HC_DMA_AddrMode_e       destIncrement;
    HC_U8                   trigger;
    HC_Bool_e               triggerExternal;
    HC_Bool_e               irqEnable;
} HC_DMA_ChCfg_t;

typedef HC_VOID (*HC_DMA_Callback_t)(HC_VOID);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_DMA_CFG_H */
