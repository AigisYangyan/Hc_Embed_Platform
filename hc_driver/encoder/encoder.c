/**
 * @file    encoder.c
 * @brief   两轮差速底盘编码器采样、速度换算与里程累计模块
 *
 * @details
 * 模块职责：
 * 1. 周期读取左右轮编码器总计数，计算单周期脉冲增量。
 * 2. 按轮径、采样周期、PPR 与修正系数将脉冲增量换算为线速度。
 * 3. 提供差速底盘左右轮目标速度分配与对称限幅。
 * 4. 基于左右轮平均速度做累计里程估算。
 *
 * 数据与单位约定：
 * - 采样周期：ms（内部换算为 s）。
 * - 轮径/周长：mm。
 * - 编码器换算速度：m/s。
 * - 控制目标速度接口：沿用控制层约定（当前工程默认为 mm/s）。
 * - 累计里程：m。
 *
 * 调用时序建议：
 * - 上电后先调用 Encoder_Init() 完成参数同步，并尝试进行基准计数对齐。
 * - 在固定周期任务中调用 Encoder_UpdateSample() 更新速度相关状态。
 * - 控制层按需调用 Encoder_CalcWheelTargets() 生成左右轮目标值。
 * - 里程统计任务周期调用 Encoder_UpdateTravelDistance()。
 *
 * 依赖：
 * - HAL 编码器读取接口：HC_HAL_GPIO_GetEncoderCounts()
 * - 电机运行态结构：g_tMotors
 *
 * 注意：
 * - 当采样周期或 PPR 非法（<= 0）时，速度换算系数会被置 0。
 * - 方向正负由通道配置表 g_tEncoderChannelCfg 统一管理。
 */

#include "driver/encoder/encoder.h"
#include "hc_hal_gpio.h"
#include <math.h>
#include <stddef.h>

#define ENCODER_PI 3.1416f

 /* 通道配置表：
  * 1. 明确左右轮对应到哪个电机实例
  * 2. 将方向修正从业务逻辑中抽离，统一由配置表管理
  */
const Encoder_ChannelCfg_t g_tEncoderChannelCfg[ENCODER_WHEEL_COUNT] = {
    { MOTOR_LEFT,  -1 },
    { MOTOR_RIGHT,  1 }
};

/* 浮点参数表：
 * 保留原 speed 模块的轮径、采样周期、限幅等参数，
 * 只是改成统一集中管理，便于后续调参与扩展。
 */
float g_fEncoderParam[ENCODER_PARAM_COUNT] = {
    [ENCODER_PARAM_MIU] = 1.0f,//默认摩擦修正系数为1，后续可以调节以补偿实际速度与理论速度的差异
    [ENCODER_PARAM_SAMPLE_PERIOD_MS] = 10.0f,//默认10ms采样周期
    [ENCODER_PARAM_WHEEL_DIAMETER_MM] = 68.6f,//初始值根据实际轮径设置
    [ENCODER_PARAM_WHEEL_CIRCUMFERENCE_MM] = (ENCODER_PI * 68.6f),//初始值根据轮径计算得到
    [ENCODER_PARAM_SPEED_FACTOR] = 0.0f,//初始值为0，后续根据其他参数计算得到
    [ENCODER_PARAM_MAX_TARGET_SPEED] = 1200.0f,//假设最大速度为1200mm/s, 约等于1.2m/s
    [ENCODER_PARAM_TRAVEL_DISTANCE_M] = 0.0f//初始累计距离为0
};

/* 整型参数表：当前只放每圈脉冲数 PPR。 */
int32_t g_iEncoderParam[ENCODER_INT_PARAM_COUNT] = {
    [ENCODER_INT_PARAM_PPR] = 1560//初始值根据实际编码器规格设置
};

/*
 * ┌─────────┬────────────────────────────────────────┐
 * │ 系数计算 │ encoder_update_speed_factor()          │
 * │ 对称限幅 │ encoder_clamp_symmetric()              │
 * │ 双轮读取 │ encoder_read_totals()                  │
 * │ 单轮更新 │ encoder_update_one_wheel()             │
 * │ 参数访问 │ encoder_get/set_param()                │
 * │         │ encoder_get_int_param()                  │
 * └─────────┴────────────────────────────────────────┘
 */
static void encoder_update_speed_factor(void);//根据当前参数表计算脉冲到速度的换算系数
static float encoder_clamp_symmetric(float value, float abs_limit);//对称限幅，保证左右轮目标值都在同一绝对范围内
static int encoder_read_totals(HC_S32 totals[ENCODER_WHEEL_COUNT]);//从 HAL 一次性读取左右轮编码器总数
static void encoder_update_one_wheel(Encoder_WheelId_e wheel, HC_S32 total_count);//更新单轮状态：计算增量、更新累计值、计算速度
static float encoder_get_param(Encoder_ParamId_e id);//参数访问函数，获取浮点参数值
static void encoder_set_param(Encoder_ParamId_e id, float value);//参数访问函数，设置浮点参数值
static int32_t encoder_get_int_param(Encoder_IntParamId_e id);//参数访问函数，获取整型参数值

/*-----------------------------分割线------------------------------*/


/* 参数访问函数保留为静态接口，避免外部直接散落地操作参数表。 */
static float encoder_get_param(Encoder_ParamId_e id)
{
    return g_fEncoderParam[id];
}

static void encoder_set_param(Encoder_ParamId_e id, float value)
{
    g_fEncoderParam[id] = value;
}

static int32_t encoder_get_int_param(Encoder_IntParamId_e id)
{
    return g_iEncoderParam[id];
}

/*----------------------------------------------------------------------*/


/* 根据当前参数表计算脉冲到速度的换算系数。
 * 说明：使用轮周长（mm）、摩擦修正系数、采样周期（s）与每转脉冲数（PPR）
 * 计算出每脉冲对应的速度，单位为 m/s。若采样周期或 PPR 非法（<=0），
 * 会将速度因子置为 0 以避免除零错误。
 */
static void encoder_update_speed_factor(void)
{
    float sample_period_sec = encoder_get_param(ENCODER_PARAM_SAMPLE_PERIOD_MS) / 1000.0f;//采样周期转换为秒
    float wheel_circ_mm = encoder_get_param(ENCODER_PARAM_WHEEL_CIRCUMFERENCE_MM);//轮子周长，单位毫米
    float miu = encoder_get_param(ENCODER_PARAM_MIU);//摩擦修正系数
    float ppr = (float)encoder_get_int_param(ENCODER_INT_PARAM_PPR);//每转脉冲数
    //如果采样周期或每转脉冲数为0，无法计算速度换算系数，设置为0并返回
    if (sample_period_sec <= 0.0f || ppr <= 0.0f) {
        encoder_set_param(ENCODER_PARAM_SPEED_FACTOR, 0.0f);
        return;
    }
    //速度换算系数计算公式：轮子周长 * 摩擦修正系数 / (采样周期 * 每转脉冲数 * 1000)，单位为米/秒
    encoder_set_param(
        ENCODER_PARAM_SPEED_FACTOR,//将编码器增量转换为物理速度的系数
        wheel_circ_mm * miu / (sample_period_sec * ppr * 1000.0f));
}

/* 对称限幅，保证左右轮目标值都在同一绝对范围内。
 * 输入输出保持在 [-abs_limit, +abs_limit] 区间内。
 */
static float encoder_clamp_symmetric(float value, float abs_limit)
{
    if (value > abs_limit) {
        return abs_limit;
    }
    if (value < -abs_limit) {
        return -abs_limit;
    }
    return value;
}

/* 从 HAL 一次性读取左右轮编码器总数。
 * 输出：将读取到的左右轮总计数按索引写入 totals 数组。
 * 返回：1 表示成功并填充 totals，0 表示读取失败。
 */
static int encoder_read_totals(HC_S32 totals[ENCODER_WHEEL_COUNT])
{
    HC_S32 left_total = 0;
    HC_S32 right_total = 0;

    if (HC_HAL_GPIO_GetEncoderCounts(&left_total, &right_total) != HC_HAL_OK) {
        return 0;
    }

    totals[ENCODER_WHEEL_LEFT] = left_total;
    totals[ENCODER_WHEEL_RIGHT] = right_total;
    return 1;
}

/* 更新单轮状态：
 * - 计算本周期脉冲增量：delta = total_count - previous_total
 * - 更新 motor->encoder_total 与 motor->encoder_delta
 * - 调用 Encoder_CalcSpeed() 将脉冲增量换算为速度（m/s），并乘以通道方向修正符号
 */
static void encoder_update_one_wheel(Encoder_WheelId_e wheel, HC_S32 total_count)
{
    const Encoder_ChannelCfg_t* cfg = &g_tEncoderChannelCfg[wheel];
    Motor_T* motor = &g_tMotors[cfg->motor_id];
    HC_S32 delta = total_count - (HC_S32)motor->encoder_total;

    motor->encoder_total = (int32_t)total_count;
    motor->encoder_delta = (int16_t)delta;
    motor->speed = Encoder_CalcSpeed((int32_t)delta) * (float)cfg->direction_sign;
}


/*
 * 初始化编码器模块
 *
 * - 同步轮径与周长参数并清零累计里程
 * - 刷新速度换算系数
 * - 尝试读取硬件总计数作为基准（读取失败时回退为 0）
 * - 清空运行时缓存，保证 PID/控制模块启动前状态干净
 */
void Encoder_Init(void)
{
    HC_S32 totals[ENCODER_WHEEL_COUNT] = { 0, 0 };

    /* 初始化时先同步依赖参数，避免外部直接修改轮径后没有刷新周长。 */
    encoder_set_param(
        ENCODER_PARAM_WHEEL_CIRCUMFERENCE_MM,
        encoder_get_param(ENCODER_PARAM_WHEEL_DIAMETER_MM) * ENCODER_PI);
    encoder_set_param(ENCODER_PARAM_TRAVEL_DISTANCE_M, 0.0f);
    encoder_update_speed_factor();

    /* 读取当前硬件总计数作为基准，防止上电后第一拍出现大跳变。 */
    if (encoder_read_totals(totals) != 0) {
        g_tMotors[MOTOR_LEFT].encoder_total = (int32_t)totals[ENCODER_WHEEL_LEFT];
        g_tMotors[MOTOR_RIGHT].encoder_total = (int32_t)totals[ENCODER_WHEEL_RIGHT];
    }
    else {
        g_tMotors[MOTOR_LEFT].encoder_total = 0;
        g_tMotors[MOTOR_RIGHT].encoder_total = 0;
    }

    /* 清空运行时缓存，保证 PID 启动前状态干净。 */
    for (int i = 0; i < MOTOR_COUNT; ++i) {
        g_tMotors[i].encoder_delta = 0;
        g_tMotors[i].speed = 0.0f;
    }
}

/*
 * 周期采样入口：读取总计数，计算增量，再更新各轮速度（速度单位为 m/s）。
 * 设计为按 ENCODER_PARAM_SAMPLE_PERIOD_MS 的节拍周期调用；一次性读取所有轮的总数
 * 以减少对 HAL 的多次访问带来的开销。
 */

void Encoder_UpdateSample(void)
{
    HC_S32 totals[ENCODER_WHEEL_COUNT] = { 0, 0 };

    if (encoder_read_totals(totals) == 0) {
        return;
    }

    /* 逐轮更新，流程统一走同一个参数化函数。 */
    for (int i = 0; i < ENCODER_WHEEL_COUNT; ++i) {
        encoder_update_one_wheel((Encoder_WheelId_e)i, totals[i]);
    }
}

/* 读取左右轮累计编码器总数。
 * 参数：p_left_total 与 p_right_total 可为 NULL，表示不需要该侧的值。
 */
void Encoder_GetTotals(int32_t* p_left_total, int32_t* p_right_total)
{
    if (p_left_total != NULL) {
        *p_left_total = g_tMotors[MOTOR_LEFT].encoder_total;
    }
    if (p_right_total != NULL) {
        *p_right_total = g_tMotors[MOTOR_RIGHT].encoder_total;
    }
}

/* 根据一个采样周期内的脉冲增量换算出物理速度（m/s）。
 * 若速度换算系数为 0，会尝试重新计算一次以确保参数同步。
 */
float Encoder_CalcSpeed(int32_t pulse_count)
{
    if (encoder_get_param(ENCODER_PARAM_SPEED_FACTOR) == 0.0f) {
        encoder_update_speed_factor();
    }
    return pulse_count * encoder_get_param(ENCODER_PARAM_SPEED_FACTOR);
}

/* 修改摩擦修正系数并同步刷新速度换算系数（会触发 encoder_update_speed_factor）。 */
void Encoder_SetMiu(float miu)
{
    encoder_set_param(ENCODER_PARAM_MIU, miu);
    encoder_update_speed_factor();
}

/* 修改采样周期（毫秒）并同步刷新速度换算系数（会触发 encoder_update_speed_factor）。 */
void Encoder_SetSamplePeriodMs(float sample_period_ms)
{
    encoder_set_param(ENCODER_PARAM_SAMPLE_PERIOD_MS, sample_period_ms);
    encoder_update_speed_factor();
}

/*
 * 统一计算左右轮目标值，适用于差速底盘。
 * - 参数说明：`base_speed` 与 `turn_output` 采用控制层约定的速度口径（当前工程默认 mm/s），
 *   `turn_sign` 用于选择符号约定（差速/角度控制）。
 * - 输出 `left_target` / `right_target` 使用相同单位并经过对称限幅，限制在 ENCODER_PARAM_MAX_TARGET_SPEED 范围内。
 */

void Encoder_CalcWheelTargets(float base_speed,
    float turn_output,
    Encoder_TurnSign_e turn_sign,
    float* left_target,
    float* right_target)
{
    /* 两轮差速底盘只需要左右轮线速度分配 */
    float turn = (float)turn_sign * turn_output;
    float max_speed = encoder_get_param(ENCODER_PARAM_MAX_TARGET_SPEED);

    if (left_target == NULL || right_target == NULL) {
        return;
    }

    *left_target = encoder_clamp_symmetric(base_speed + turn, max_speed);
    *right_target = encoder_clamp_symmetric(base_speed - turn, max_speed);
}

/* 更新累计行驶距离，单位：米（m）。
 * 使用左右轮平均速度近似整车前进速度：avg_speed = 0.5 * (left_speed + right_speed)
 * 按当前采样周期积分绝对距离并累加到 ENCODER_PARAM_TRAVEL_DISTANCE_M。
 */
void Encoder_UpdateTravelDistance(float left_speed, float right_speed)
{
    /* 用左右轮平均速度近似整车前进速度，适合普通轮轴底盘。 */
    float sample_period_sec = encoder_get_param(ENCODER_PARAM_SAMPLE_PERIOD_MS) / 1000.0f;
    float avg_speed = 0.5f * (left_speed + right_speed);
    encoder_set_param(
        ENCODER_PARAM_TRAVEL_DISTANCE_M,
        encoder_get_param(ENCODER_PARAM_TRAVEL_DISTANCE_M) + fabsf(avg_speed * sample_period_sec));
}

/* 获取累计行驶距离，单位：米（m）。 */
float Encoder_GetTravelDistance(void)
{
    return encoder_get_param(ENCODER_PARAM_TRAVEL_DISTANCE_M);
}

/* 重置累计行驶距离为 0（米）。 */
void Encoder_ResetTravelDistance(void)
{
    encoder_set_param(ENCODER_PARAM_TRAVEL_DISTANCE_M, 0.0f);
}
