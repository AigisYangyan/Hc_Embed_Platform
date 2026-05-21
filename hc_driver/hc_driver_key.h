#ifndef HC_DRIVER_KEY_H
#define HC_DRIVER_KEY_H

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

#endif /* HC_DRIVER_KEY_H */
