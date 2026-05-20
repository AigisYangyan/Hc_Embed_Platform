/**
 * @file    hc_hal_pwm.c
 * @brief   PWM HAL 接口与实现，提供占空比设置与输出启停控制。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_board_cfg.h"
#include "hc_hal_pwm.h"

/* ============================================================================
 *  1. 模块私有数据
 * ========================================================================== */

/** @brief PWM 单个通道的定时器绑定配置。 */
typedef struct {
    GPTIMER_Regs *inst;
    HC_U32 ccIndex;
    HC_U16 period;
} HC_HAL_PWM_ChannelCfg_t;

static const HC_HAL_PWM_ChannelCfg_t s_pwm_channel_cfg[HC_HAL_PWM_CH_MAX] = {
    [HC_HAL_PWM_CH_MOTOR_L] = {
        .inst = HC_HAL_PWM_MOTOR_L_INST,
        .ccIndex = HC_HAL_PWM_MOTOR_L_CC_INDEX,
        .period = HC_HAL_PWM_MOTOR_L_PERIOD,
    },
    [HC_HAL_PWM_CH_MOTOR_R] = {
        .inst = HC_HAL_PWM_MOTOR_R_INST,
        .ccIndex = HC_HAL_PWM_MOTOR_R_CC_INDEX,
        .period = HC_HAL_PWM_MOTOR_R_PERIOD,
    },
};

static HC_Bool_e s_pwm_initialized = HC_FALSE;

/* ============================================================================
 *  2. 内部工具函数
 * ========================================================================== */

/** @brief 根据通道 ID 获取 PWM 配置，越界返回 NULL。 */
static const HC_HAL_PWM_ChannelCfg_t *hc_hal_pwm_get_cfg(HC_HAL_PWM_Channel_e channel)
{
    if ((HC_U32)channel >= (HC_U32)HC_HAL_PWM_CH_MAX) {
        return HC_NULL_PTR;
    }

    return &s_pwm_channel_cfg[channel];
}

/**
 * @brief 占空比值 (0~1000) 转换为 Capture/Compare 寄存器值。
 *
 * 由于 cfg 中 period 已等于 HC_HAL_PWM_DUTY_MAX (1000)，duty 值可直接当作
 * 比较值写入，period 参数仅作预留语义。
 */
static HC_U32 hc_hal_pwm_duty_to_compare(HC_U16 duty, HC_U16 period)
{
    HC_UNUSED(period);
    return (HC_U32)duty;
}

/* ============================================================================
 *  3. 公开 API 实现 (详细语义见 hc_hal_pwm.h)
 * ========================================================================== */

HC_S32 HC_HAL_PWM_Init(HC_VOID)
{
    HC_U32 index;

    if (s_pwm_initialized == HC_TRUE) {
        return HC_HAL_ERR_ALREADY_INIT;
    }

    for (index = 0u; index < (HC_U32)HC_HAL_PWM_CH_MAX; index++) {
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channel_cfg[index];

        HC_HAL_ASSERT_PARAM(cfg->inst != HC_NULL_PTR, HC_ERR_NOT_ENABLE);
        DL_TimerA_setCaptureCompareValue(
            cfg->inst,
            hc_hal_pwm_duty_to_compare(0u, cfg->period),
            cfg->ccIndex);
    }

    s_pwm_initialized = HC_TRUE;
    return HC_HAL_OK;
}

HC_S32 HC_HAL_PWM_SetDuty(HC_HAL_PWM_Channel_e channel, HC_U16 duty)
{
    const HC_HAL_PWM_ChannelCfg_t *cfg;

    HC_HAL_ASSERT_PARAM(duty <= HC_HAL_PWM_DUTY_MAX, HC_HAL_ERR_INVALID);

    if (s_pwm_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    cfg = hc_hal_pwm_get_cfg(channel);
    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->inst != HC_NULL_PTR),
                        HC_HAL_ERR_INVALID);

    DL_TimerA_setCaptureCompareValue(
        cfg->inst,
        hc_hal_pwm_duty_to_compare(duty, cfg->period),
        cfg->ccIndex);

    return HC_HAL_OK;
}

HC_S32 HC_HAL_PWM_Start(HC_VOID)
{
    HC_U32 index;

    if (s_pwm_initialized == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    for (index = 0u; index < (HC_U32)HC_HAL_PWM_CH_MAX; index++) {
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channel_cfg[index];

        HC_HAL_ASSERT_PARAM(cfg->inst != HC_NULL_PTR, HC_ERR_NOT_ENABLE);
        DL_TimerA_startCounter(cfg->inst);
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
        const HC_HAL_PWM_ChannelCfg_t *cfg = &s_pwm_channel_cfg[index];

        HC_HAL_ASSERT_PARAM(cfg->inst != HC_NULL_PTR, HC_ERR_NOT_ENABLE);
        DL_TimerA_stopCounter(cfg->inst);
    }

    return HC_HAL_OK;
}
