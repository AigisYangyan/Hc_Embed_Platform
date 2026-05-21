#ifndef HC_DRIVER_OLED_H
#define HC_DRIVER_OLED_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowString(uint8_t x, uint8_t y, const char *text, uint8_t size);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_OLED_H */
