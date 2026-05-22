/**
 * @file    ui_event.h
 * @brief   UI 事件缓冲接口
 */

#ifndef UI_UI_EVENT_H
#define UI_UI_EVENT_H

#include "hc_common/hc_types.h"
#include "hc_driver/key/hc_driver_key.h"

#ifdef __cplusplus
extern "C" {
#endif

void UiEvent_Init(void);
HC_Bool_e UiEvent_Post(Key_Id_e key);
HC_Bool_e UiEvent_PostFromISR(Key_Id_e key);
HC_Bool_e UiEvent_Pend(Key_Id_e* out_key, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* UI_UI_EVENT_H */
