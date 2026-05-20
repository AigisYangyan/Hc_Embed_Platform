/**
 * @file    demo_blink.c
 * @brief   BLINK App 示例实现
 */

#include "app/demo_blink/demo_blink.h"
#include "port_oled.h"
#include "service/telemetry_service.h"
#include "ui/menu_core.h"

static uint8_t  s_blink_state;
static uint32_t s_tick_count;

void DemoBlink_Init(void)
{
    s_blink_state = 0u;
    s_tick_count = 0u;
}

void DemoBlink_Enter(void)
{
    s_blink_state = 0u;
    s_tick_count = 0u;
    TelemetryService_EnterProfile(VOFA_PROFILE_DEMO);
    Menu_RequestRedraw();
}

void DemoBlink_Exit(void)
{
    TelemetryService_ExitProfile();
}

void DemoBlink_Task500ms(void)
{
    VofaDemoCtx_t* ctx = TelemetryService_GetDemoCtx();

    s_tick_count++;
    s_blink_state ^= 1u;

    if (ctx != (VofaDemoCtx_t*)0) {
        ctx->tx_state = (float)s_blink_state;
        ctx->tx_tick_count = (float)s_tick_count;
    }

    Menu_RequestRedraw();
}

void DemoBlink_Render(const MenuPage_t* page, const MenuState_t* state)
{
    HC_UNUSED(page);
    HC_UNUSED(state);

    OLED_ShowString(20u, 0u, "APP", 16u);
    OLED_ShowString(20u, 3u, (s_blink_state != 0u) ? "ON " : "OFF", 16u);
}
