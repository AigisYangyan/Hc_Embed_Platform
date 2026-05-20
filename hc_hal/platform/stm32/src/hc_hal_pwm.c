#include "hc_hal_pwm.h"

#include "hc_hal_board_cfg.h"
#include "tim.h"

typedef struct {
    TIM_HandleTypeDef *handle;
    HC_U32 channel;
} HC_HAL_PWM_ChannelCfg_t;

static const HC_HAL_PWM_ChannelCfg_t s_pwm_channels[HC_HAL_PWM_CH_MAX] = {
    [HC_HAL_PWM_CH_MOTOR_L] = { &htim2, TIM_CHANNEL_2 },
    [HC_HAL_PWM_CH_MOTOR_R] = { &htim2, TIM_CHANNEL_3 },
};

static HC_Bool_e s_pwm_initialized = HC_FALSE;

static const HC_HAL_PWM_ChannelCfg_t *hc_hal_pwm_get_cfg(HC_HAL_PWM_Channel_e channel)
{
    if ((HC_U32)channel >= (HC_U32)HC_HAL_PWM_CH_MAX) {
        return HC_NULL_PTR;
    }

    return &s_pwm_channels[channel];
}

static HC_U32 hc_hal_pwm_scale_duty(const TIM_HandleTypeDef *handle, HC_U16 duty)
{
    HC_U32 period = __HAL_TIM_GET_AUTORELOAD((TIM_HandleTypeDef *)handle);

    return (period * (HC_U32)duty) / HC_HAL_PWM_DUTY_MAX;
}

HC_S32 HC_HAL_PWM_Init(HC_VOID)
{
    HC_U32 index;

    if (s_pwm_initialized == HC_TRUE) {
        return HC_HAL_ERR_ALREADY_INIT;
    }

    for (index = 0u; index < (HC_U32)HC_HAL_PWM_CH_MAX; index++) {
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channels[index];

        HC_HAL_ASSERT_PARAM((cfg->handle != HC_NULL_PTR) && (cfg->channel != 0u), HC_ERR_NOT_ENABLE);
        __HAL_TIM_SET_COMPARE(cfg->handle, cfg->channel, 0u);
    }

    s_pwm_initialized = HC_TRUE;
    return HC_HAL_OK;
}

HC_S32 HC_HAL_PWM_SetDuty(HC_HAL_PWM_Channel_e channel, HC_U16 duty)
{
    const HC_HAL_PWM_ChannelCfg_t *cfg = hc_hal_pwm_get_cfg(channel);

    HC_HAL_ASSERT_PARAM(duty <= HC_HAL_PWM_DUTY_MAX, HC_HAL_ERR_INVALID);

    if (s_pwm_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->handle != HC_NULL_PTR), HC_HAL_ERR_INVALID);
    __HAL_TIM_SET_COMPARE(cfg->handle, cfg->channel, hc_hal_pwm_scale_duty(cfg->handle, duty));
    return HC_HAL_OK;
}

HC_S32 HC_HAL_PWM_Start(HC_VOID)
{
    HC_U32 index;

    if (s_pwm_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    for (index = 0u; index < (HC_U32)HC_HAL_PWM_CH_MAX; index++) {
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channels[index];

        HC_HAL_ASSERT_PARAM((cfg->handle != HC_NULL_PTR) && (cfg->channel != 0u), HC_ERR_NOT_ENABLE);
        if (HAL_TIM_PWM_Start(cfg->handle, cfg->channel) != HAL_OK) {
            return HC_ERR_UNKNOWN;
        }
    }

    return HC_HAL_OK;
}

HC_S32 HC_HAL_PWM_Stop(HC_VOID)
{
    HC_U32 index;

    if (s_pwm_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    for (index = 0u; index < (HC_U32)HC_HAL_PWM_CH_MAX; index++) {
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channels[index];

        HC_HAL_ASSERT_PARAM((cfg->handle != HC_NULL_PTR) && (cfg->channel != 0u), HC_ERR_NOT_ENABLE);
        if (HAL_TIM_PWM_Stop(cfg->handle, cfg->channel) != HAL_OK) {
            return HC_ERR_UNKNOWN;
        }
    }

    return HC_HAL_OK;
}
