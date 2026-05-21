/**
 * @file    app_registry.h
 * @brief   App 注册表对外接口
 */

#ifndef APP_APP_REGISTRY_H
#define APP_APP_REGISTRY_H

#include "hc_common/hc_types.h"
#include "ui/menu_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HC_APP_NONE = 0,
    HC_APP_DEMO_BLINK,
    HC_APP_MAX
} HcAppId_e;

typedef void (*HcAppHookFn)(void);
typedef void (*HcAppTickFn)(void);
typedef void (*HcAppRenderFn)(const MenuPage_t* page, const MenuState_t* state);

typedef struct {
    HcAppId_e      id;
    const char*    name;
    MenuPageId_e   target_page;
    HC_Bool_e      show_in_home;
    HC_Bool_e      show_in_debug;
    HcAppHookFn    init;
    HcAppHookFn    enter;
    HcAppHookFn    exit;
    HcAppTickFn    tick_5ms;
    HcAppTickFn    tick_500ms;
    HcAppRenderFn  render;
} HcAppReg_t;

extern const HcAppReg_t g_hc_apps[];
extern const uint8_t    g_hc_app_count;

const HcAppReg_t* AppRegistry_FindById(HcAppId_e id);
void AppRegistry_InitAll(void);
uint8_t AppRegistry_BuildMenuItems(HC_Bool_e for_debug,
                                   MenuItem_t* out_items,
                                   uint8_t max_count);

#ifdef __cplusplus
}
#endif

#endif /* APP_APP_REGISTRY_H */
