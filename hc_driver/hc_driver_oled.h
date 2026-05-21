#ifndef HC_DRIVER_OLED_H
#define HC_DRIVER_OLED_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void        OLED_Init(void);
HC_Error_e  OLED_Process(void);
HC_Bool_e   OLED_IsReady(void);

HC_Error_e  OLED_Clear(void);

HC_Error_e  OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t sizey);
HC_Error_e  OLED_ShowString(uint8_t x, uint8_t y, const char *chr, uint8_t sizey);
HC_Error_e  OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t sizey);
HC_Error_e  OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t no, uint8_t sizey);

void        OLED_DrawBMP(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey, const uint8_t BMP[]);

void        OLED_Display_On(void);
void        OLED_Display_Off(void);
void        OLED_ColorTurn(uint8_t enable_inverse);
void        OLED_DisplayTurn(uint8_t enable_rotate);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_OLED_H */
