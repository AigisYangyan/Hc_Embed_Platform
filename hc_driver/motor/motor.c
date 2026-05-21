/**
 * @file    motor.c
 * @brief   电机驱动模块实现
 *
 * 本文件实现双路直流电机的基础驱动逻辑，作为执行层（Direction + PWM）。
 *
 * 功能范围：
 * - 控制左右电机正反转（基于 H 桥的方向引脚）
 * - 设置 PWM 占空比（对外归一化 -1000..1000，对应 0%..100% 占空比）
 * - 提供单电机/双电机输出接口与主动刹车
 *
 * 不负责的内容：
 * - 编码器采样与速度闭环（由上层模块负责）
 * - 电流保护、堵转检测等高级安全策略
 *
 * 实现说明：
 * 1. 使用“方向引脚 + PWM 占空比”方式驱动 H 桥
 * 2. pwm 的符号决定方向，绝对值决定 PWM 大小（会被限制在 MOTOR_PWM_MAX）
 * 3. 所有对外接口最终落到底层 HAL 的写引脚/写 PWM 调用
 *
 * 硬件绑定项：
 * - 左右电机方向引脚与 PWM 通道（通过 HAL 抽象映射）
 *
 * 若移植到其他 MCU，只需修改底层 GPIO/PWM HAL 的适配层。
 */

#include "motor.h"

 /* ---- 全局电机实例 ------------------------------------------------------- */

Motor_T g_tMotors[MOTOR_COUNT];

/* ---- 公开 API ----------------------------------------------------------- */

/**
 * @brief 电机子系统初始化
 * @note  填充左右电机的硬件配置并清零运行时状态
 *     电机引脚连接方式：(统一 HAL 后，此处不再依赖具体引脚定义，仅说明连接逻辑)
 *     初始化包含：H 桥引脚与 PWM 通道映射、编码器方向修正符号设置、PID 指针绑定。
 *     初始化后，上层应在周期任务中执行编码器采样以提供速度数据（`Motor_GetSpeed()` 返回单位为 m/s）。
 */
void Motor_Init(void)
{
    /* 左电机：正转 = L1低, L2高  =>  pin_fwd = L2, pin_rev = L1 */
    g_tMotors[MOTOR_LEFT].hbridge.pin_fwd = VPIN_MOTOR_L2;
    g_tMotors[MOTOR_LEFT].hbridge.pin_rev = VPIN_MOTOR_L1;
    g_tMotors[MOTOR_LEFT].pwm_ch = HC_HAL_PWM_CH_MOTOR_L;
    g_tMotors[MOTOR_LEFT].encoder_sign = -1;   /* 左轮编码器方向取反 */
    g_tMotors[MOTOR_LEFT].p_pid = &g_tLeftMotorPID;

    /* 右电机：正转 = R1高, R2低  =>  pin_fwd = R1, pin_rev = R2 */
    g_tMotors[MOTOR_RIGHT].hbridge.pin_fwd = VPIN_MOTOR_R1;
    g_tMotors[MOTOR_RIGHT].hbridge.pin_rev = VPIN_MOTOR_R2;
    g_tMotors[MOTOR_RIGHT].pwm_ch = HC_HAL_PWM_CH_MOTOR_R;
    g_tMotors[MOTOR_RIGHT].encoder_sign = +1;   /* 右轮编码器方向不变 */
    g_tMotors[MOTOR_RIGHT].p_pid = &g_tRightMotorPID;

    /* 清零运行时状态 */
    for (int i = 0; i < MOTOR_COUNT; i++) {
        g_tMotors[i].encoder_delta = 0;
        g_tMotors[i].encoder_total = 0;
        g_tMotors[i].speed = 0.0f;
    }
}

/**
 * @brief  设置电机 PWM 输出
 * @param  m   电机实例指针
 * @param  pwm 带符号占空比：正值=正转，负值=反转，0=滑行（范围 -1000..1000，会被限制在 MOTOR_PWM_MAX）
 */
void Motor_SetPwm(Motor_T* m, float pwm)
{
    int abs_pwm = (pwm >= 0.0f) ? (int)pwm : (int)(-pwm);//取绝对值,
    if (abs_pwm > MOTOR_PWM_MAX) {
        abs_pwm = MOTOR_PWM_MAX;
    }//限制占空比在最大值范围内

    if (pwm > 0.0f) {
        /* 正转：正转引脚高，反转引脚低 */
        HC_HAL_GPIO_SetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_rev);
    }
    else if (pwm < 0.0f) {
        /* 反转：正转引脚低，反转引脚高 */
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_SetPin(m->hbridge.pin_rev);
    }
    else {
        /* 滑行：两引脚均低 */
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_fwd);
        HC_HAL_GPIO_ResetPin(m->hbridge.pin_rev);
        abs_pwm = 0;
    }

    HC_HAL_PWM_SetDuty(m->pwm_ch, (HC_U16)abs_pwm);
}

/**
 * @brief  主动刹车
 * @param  m 电机实例指针
 * @note   H 桥两引脚同时置高，PWM 输出为 0；此为快速制动（短路制动），可能带来能耗或热量增加
 */
void Motor_Brake(Motor_T* m)
{
    HC_HAL_GPIO_SetPin(m->hbridge.pin_fwd);
    HC_HAL_GPIO_SetPin(m->hbridge.pin_rev);
    HC_HAL_PWM_SetDuty(m->pwm_ch, 0u);
}

/**
 * @brief  获取电机当前速度
 * @param  m 电机实例指针
 * @return 最近一次采样计算的速度 (m/s)
 */
float Motor_GetSpeed(const Motor_T* m)
{
    return m->speed;
}

/**
 * @brief 全部电机刹车
 */
void Motor_BrakeAll(void)
{
    for (int i = 0; i < MOTOR_COUNT; i++) {
        Motor_Brake(&g_tMotors[i]);
    }
}
