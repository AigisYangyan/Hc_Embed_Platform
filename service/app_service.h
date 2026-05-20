/**
 * @file    app_service.h
 * @brief   App 生命周期服务
 */

#ifndef SERVICE_APP_SERVICE_H
#define SERVICE_APP_SERVICE_H

#include "app/app_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

void AppService_Init(void);
HC_Bool_e AppService_EnterApp(HcAppId_e id);
void AppService_LeaveApp(void);
const HcAppReg_t* AppService_GetActiveApp(void);
void AppService_RunActive5ms(void);
void AppService_RunActive500ms(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_APP_SERVICE_H */
