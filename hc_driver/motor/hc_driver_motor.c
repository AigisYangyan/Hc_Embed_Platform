#include "hc_driver_motor.h"
#include "hc_hal_gpio.h"
#include "hc_hal_pwm.h"

/* ── 私有类型定义 ──────────────────────────────────────────────────── */

typedef struct {
    HC_HAL_GPIO_VPin_e  pin_fwd;
    HC_HAL_GPIO_VPin_e  pin_rev;
} Motor_HBridge_T;

typedef struct {
    Motor_HBridge_T      hbridge;
    HC_HAL_PWM_Channel_e pwm_ch;
    int8_t               encoder_sign;
    int16_t              encoder_delta;
    int32_t              encoder_total;
    float                speed;
} Motor_T;

/* ── 私有电机实例 ──────────────────────────────────────────────────── */

static Motor_T s_motors[MOTOR_COUNT];

/* ── 公开 API ──────────────────────────────────────────────────────── */

void Motor_Init(void)
{
    s_motors[MOTOR_LEFT].hbridge.pin_fwd = VPIN_MOTOR_L2;
    s_motors[MOTOR_LEFT].hbridge.pin_rev = VPIN_MOTOR_L1;
    s_motors[MOTOR_LEFT].pwm_ch = HC_HAL_PWM_CH_MOTOR_L;
    s_motors[MOTOR_LEFT].encoder_sign = -1;

    s_motors[MOTOR_RIGHT].hbridge.pin_fwd = VPIN_MOTOR_R1;
    s_motors[MOTOR_RIGHT].hbridge.pin_rev = VPIN_MOTOR_R2;
    s_motors[MOTOR_RIGHT].pwm_ch = HC_HAL_PWM_CH_MOTOR_R;
    s_motors[MOTOR_RIGHT].encoder_sign = +1;

    for (int i = 0; i < MOTOR_COUNT; i++) {
        s_motors[i].encoder_delta = 0;
        s_motors[i].encoder_total = 0;
        s_motors[i].speed = 0.0f;
    }
}

void Motor_SetPwm(Motor_Id_e id, float pwm)
{
    if (id >= MOTOR_COUNT) {
        return;
    }
    Motor_T *m = &s_motors[id];

    int abs_pwm = (pwm >= 0.0f) ? (int)pwm : (int)(-pwm);
    if (abs_pwm > MOTOR_PWM_MAX) {
        abs_pwm = MOTOR_PWM_MAX;
    }

    if (pwm > 0.0f) {
        HC_HAL_GPIO_SetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_rev);
    } else if (pwm < 0.0f) {
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_SetPin(m->hbridge.pin_rev);
    } else {
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_rev);
        abs_pwm = 0;
    }

    HC_HAL_PWM_SetDuty(m->pwm_ch, (HC_U16)abs_pwm);
}

void Motor_Brake(Motor_Id_e id)
{
    if (id >= MOTOR_COUNT) {
        return;
    }
    Motor_T *m = &s_motors[id];

    HC_HAL_GPIO_SetPin(m->hbridge.pin_fwd);
    HC_HAL_GPIO_SetPin(m->hbridge.pin_rev);
    HC_HAL_PWM_SetDuty(m->pwm_ch, 0u);
}

HC_Error_e Motor_GetSpeed(Motor_Id_e id, float *p_speed)
{
    if (id >= MOTOR_COUNT || p_speed == (void*)0) {
        return HC_HAL_ERR_INVALID;
    }
    *p_speed = s_motors[id].speed;
    return HC_HAL_OK;
}

HC_Error_e Motor_SetSpeed(Motor_Id_e id, float speed)
{
    if (id >= MOTOR_COUNT) {
        return HC_HAL_ERR_INVALID;
    }
    s_motors[id].speed = speed;
    return HC_HAL_OK;
}

void Motor_BrakeAll(void)
{
    for (int i = 0; i < MOTOR_COUNT; i++) {
        Motor_Brake((Motor_Id_e)i);
    }
}

/* ── 最小运行态访问接口 ────────────────────────────────────────────── */

HC_Error_e Motor_GetEncoderTotal(Motor_Id_e id, int32_t *p_total)
{
    if (id >= MOTOR_COUNT || p_total == (void*)0) {
        return HC_HAL_ERR_INVALID;
    }
    *p_total = (int32_t)s_motors[id].encoder_total;
    return HC_HAL_OK;
}

HC_Error_e Motor_SetEncoderTotal(Motor_Id_e id, int32_t total)
{
    if (id >= MOTOR_COUNT) {
        return HC_HAL_ERR_INVALID;
    }
    s_motors[id].encoder_total = total;
    return HC_HAL_OK;
}

HC_Error_e Motor_GetEncoderDelta(Motor_Id_e id, int16_t *p_delta)
{
    if (id >= MOTOR_COUNT || p_delta == (void*)0) {
        return HC_HAL_ERR_INVALID;
    }
    *p_delta = s_motors[id].encoder_delta;
    return HC_HAL_OK;
}

HC_Error_e Motor_SetEncoderDelta(Motor_Id_e id, int16_t delta)
{
    if (id >= MOTOR_COUNT) {
        return HC_HAL_ERR_INVALID;
    }
    s_motors[id].encoder_delta = delta;
    return HC_HAL_OK;
}
