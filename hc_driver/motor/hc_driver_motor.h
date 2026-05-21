#ifndef HC_DRIVER_MOTOR_H
#define HC_DRIVER_MOTOR_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_PWM_MAX   1000

typedef enum {
    MOTOR_LEFT = 0,
    MOTOR_RIGHT = 1,
    MOTOR_COUNT
} Motor_Id_e;

void  Motor_Init(void);
void  Motor_SetPwm(Motor_Id_e id, float pwm);
void  Motor_Brake(Motor_Id_e id);
void  Motor_BrakeAll(void);

HC_Error_e Motor_GetSpeed(Motor_Id_e id, float *p_speed);
HC_Error_e Motor_SetSpeed(Motor_Id_e id, float speed);

HC_Error_e Motor_GetEncoderTotal(Motor_Id_e id, int32_t *p_total);
HC_Error_e Motor_SetEncoderTotal(Motor_Id_e id, int32_t total);

HC_Error_e Motor_GetEncoderDelta(Motor_Id_e id, int16_t *p_delta);
HC_Error_e Motor_SetEncoderDelta(Motor_Id_e id, int16_t delta);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_MOTOR_H */
