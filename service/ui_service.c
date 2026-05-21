/**
 * @file    ui_service.c
 * @brief   UI 常驻服务实现
 */

#include "service/ui_service.h"
#include "hc_driver/hc_driver_key.h"
#include "ui/menu_core.h"

void UiService_Init(void)
{
}

void UiService_Run5ms(void)
{
    Key_Id_e key;

    Key_Scan();

    while (Key_PollPressEvent(&key) == HC_TRUE) {
        Menu_HandleKey(key);
    }

    Menu_RenderIfDirty();
}
