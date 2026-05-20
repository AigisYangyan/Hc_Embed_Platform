#ifndef HC_HAL_GPIO_CFG_H
#define HC_HAL_GPIO_CFG_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_GPIO_DIR_INPUT = 0,
    HC_GPIO_DIR_OUTPUT = 1
} HC_GPIO_Dir_e;

typedef enum {
    HC_GPIO_PULL_NONE = 0,
    HC_GPIO_PULL_UP = 1,
    HC_GPIO_PULL_DOWN = 2
} HC_GPIO_Pull_e;

typedef enum {
    HC_GPIO_IRQ_NONE = 0,
    HC_GPIO_IRQ_RISING = 1,
    HC_GPIO_IRQ_FALLING = 2,
    HC_GPIO_IRQ_BOTH = 3
} HC_GPIO_IrqMode_e;

typedef enum {
    HC_GPIO_IO_PP = 0,
    HC_GPIO_IO_OD = 1
} HC_GPIO_IoStruct_e;

typedef struct {
    HC_VOID*            port;
    HC_U32              pin;
    HC_U32              iomux;
    HC_GPIO_Dir_e       dir;
    HC_GPIO_Pull_e      pull;
    HC_GPIO_IoStruct_e  ioStruct;
    HC_Bool_e           initState;
    HC_GPIO_IrqMode_e   irqMode;
} HC_GPIO_PinCfg_t;

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_GPIO_CFG_H */
