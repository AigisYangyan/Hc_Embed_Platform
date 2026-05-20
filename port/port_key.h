#ifndef PORT_KEY_H
#define PORT_KEY_H

/* Legacy compatibility header — forwards to canonical hc_hal for GPIO types. */
#include "hc_hal/hc_hal_gpio.h"
#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KEY_ID_K1 = 0,
    KEY_ID_K2,
    KEY_ID_K3,
    KEY_ID_K4,
    KEY_ID_COUNT
} Key_Id_e;

void Key_Init(void);
void Key_Scan(void);
HC_Bool_e Key_PollPressEvent(Key_Id_e *p_key);

#ifdef __cplusplus
}
#endif

#endif /* PORT_KEY_H */
