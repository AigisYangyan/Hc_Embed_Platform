/**
 * @file    app_registry.c
 * @brief   App 注册表实现
 */

#include "app/app_registry.h"
#include "app/demo_blink/demo_blink.h"
#include "runtime/runtime.h"

static HC_Bool_e app_registry_enter_action(uint16_t bind_value)
{
    return HcRuntime_RequestEnterApp((HcAppId_e)bind_value);
}

const HcAppReg_t g_hc_apps[] = {
    { HC_APP_DEMO_BLINK, "BLINK", PAGE_RUNNING,
      HC_TRUE, HC_FALSE,
      DemoBlink_Init, DemoBlink_Enter, DemoBlink_Exit,
      (HcAppTickFn)0, DemoBlink_Task500ms,
      DemoBlink_Render }
};

const uint8_t g_hc_app_count =
    (uint8_t)(sizeof(g_hc_apps) / sizeof(g_hc_apps[0]));

const HcAppReg_t* AppRegistry_FindById(HcAppId_e id)
{
    uint8_t i;

    for (i = 0u; i < g_hc_app_count; i++) {
        if (g_hc_apps[i].id == id) {
            return &g_hc_apps[i];
        }
    }

    return (const HcAppReg_t*)0;
}

void AppRegistry_InitAll(void)
{
    uint8_t i;

    for (i = 0u; i < g_hc_app_count; i++) {
        if (g_hc_apps[i].init != (HcAppHookFn)0) {
            g_hc_apps[i].init();
        }
    }
}

uint8_t AppRegistry_BuildMenuItems(HC_Bool_e for_debug,
                                   MenuItem_t* out_items,
                                   uint8_t max_count)
{
    uint8_t i;
    uint8_t item_count = 0u;

    if (out_items == (MenuItem_t*)0) {
        return 0u;
    }

    for (i = 0u; i < g_hc_app_count; i++) {
        HC_Bool_e should_show = (for_debug != HC_FALSE)
            ? g_hc_apps[i].show_in_debug
            : g_hc_apps[i].show_in_home;

        if ((should_show == HC_FALSE) || (item_count >= max_count)) {
            continue;
        }

        out_items[item_count].text = g_hc_apps[i].name;
        out_items[item_count].type = MENU_ITEM_ACTION;
        out_items[item_count].target_page = g_hc_apps[i].target_page;
        out_items[item_count].action = app_registry_enter_action;
        out_items[item_count].bind_value = (uint16_t)g_hc_apps[i].id;
        item_count++;
    }

    return item_count;
}
