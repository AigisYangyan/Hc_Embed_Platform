/**
 * @file    demo_blink.h
 * @brief   BLINK App 示例
 */

#ifndef APP_DEMO_BLINK_H
#define APP_DEMO_BLINK_H

#include "ui/menu_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void DemoBlink_Init(void);
void DemoBlink_Enter(void);
void DemoBlink_Exit(void);
void DemoBlink_Task500ms(void);
void DemoBlink_Render(const MenuPage_t* page, const MenuState_t* state);

#ifdef __cplusplus
}
#endif

#endif /* APP_DEMO_BLINK_H */
