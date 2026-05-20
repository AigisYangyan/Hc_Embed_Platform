/**
 * @file    ui_event.c
 * @brief   UI 事件缓冲实现
 */

#include "ui/ui_event.h"

/* ---- 模块常量 ----------------------------------------------------------- */

#define UI_EVENT_QUEUE_LEN 16u

/* ---- 模块状态 ----------------------------------------------------------- */

static Key_Id_e s_ui_event_queue[UI_EVENT_QUEUE_LEN];
static volatile uint8_t s_ui_event_head;
static volatile uint8_t s_ui_event_tail;

static HC_Bool_e ui_event_is_empty(void)
{
    return (s_ui_event_head == s_ui_event_tail) ? HC_TRUE : HC_FALSE;
}

static HC_Bool_e ui_event_is_full(void)
{
    uint8_t next_tail = (uint8_t)((s_ui_event_tail + 1u) % UI_EVENT_QUEUE_LEN);
    return (next_tail == s_ui_event_head) ? HC_TRUE : HC_FALSE;
}

/* ---- 公开 API ----------------------------------------------------------- */

void UiEvent_Init(void)
{
    s_ui_event_head = 0u;
    s_ui_event_tail = 0u;
}

HC_Bool_e UiEvent_Post(Key_Id_e key)
{
    uint8_t next_tail;

    if (ui_event_is_full() == HC_TRUE) {
        return HC_FALSE;
    }

    s_ui_event_queue[s_ui_event_tail] = key;
    next_tail = (uint8_t)((s_ui_event_tail + 1u) % UI_EVENT_QUEUE_LEN);
    s_ui_event_tail = next_tail;
    return HC_TRUE;
}

HC_Bool_e UiEvent_PostFromISR(Key_Id_e key)
{
    return UiEvent_Post(key);
}

HC_Bool_e UiEvent_Pend(Key_Id_e* out_key, uint32_t timeout_ms)
{
    uint32_t wait_count = timeout_ms;

    if (out_key == (Key_Id_e*)0) {
        return HC_FALSE;
    }

    while (ui_event_is_empty() == HC_TRUE) {
        if (wait_count == 0u) {
            return HC_FALSE;
        }

        wait_count--;
    }

    *out_key = s_ui_event_queue[s_ui_event_head];
    s_ui_event_head = (uint8_t)((s_ui_event_head + 1u) % UI_EVENT_QUEUE_LEN);
    return HC_TRUE;
}
