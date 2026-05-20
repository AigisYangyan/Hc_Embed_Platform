#ifndef HC_HAL_UART_CFG_H
#define HC_HAL_UART_CFG_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_UART_PARITY_NONE = 0,
    HC_UART_PARITY_EVEN = 1,
    HC_UART_PARITY_ODD  = 2
} HC_UART_Parity_e;

typedef enum {
    HC_UART_STOPBITS_1 = 0,
    HC_UART_STOPBITS_2 = 1
} HC_UART_StopBits_e;

typedef enum {
    HC_UART_DATABITS_7 = 0,
    HC_UART_DATABITS_8 = 1
} HC_UART_DataBits_e;

typedef struct {
    HC_VOID*            inst;
    HC_U32              iomuxTx;
    HC_U32              iomuxTxFunc;
    HC_U32              iomuxRx;
    HC_U32              iomuxRxFunc;
    HC_U32              clkFreqHz;
    HC_U32              baudRate;
    HC_U32              ibrd;
    HC_U32              fbrd;
    HC_UART_DataBits_e  dataBits;
    HC_UART_StopBits_e  stopBits;
    HC_UART_Parity_e    parity;
    HC_Bool_e           rxIrqEnable;
    HC_U32              irqNum;
    HC_Bool_e           dmaTxEnable;
    HC_Bool_e           fifoEnable;
    HC_U8               dmaTxChIdx;
    HC_Bool_e           dmaRxEnable;
    HC_U8               dmaRxChIdx;
    HC_U16              dmaRxTransferSize;
} HC_UART_Cfg_t;

typedef HC_VOID (*HC_UART_RxCallback_t)(HC_U8 data);

#ifdef __cplusplus
}
#endif

#endif /* HC_HAL_UART_CFG_H */
