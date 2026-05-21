/**
 * @file    encoder.h
 * @brief   两轮差速底盘编码器模块对外接口
 *
 * @details
 * 模块职责：
 * - 初始化编码器采样基准并同步依赖参数
 * - 周期读取左右轮编码器总数、计算脉冲增量并换算线速度
 * - 提供差速底盘左右轮目标速度分配接口与对称限幅
 * - 维护并查询累计行驶里程
 *
 * 单位与口径约定：
 * - 采样周期：毫秒（ms），内部换算为秒用于速度计算
 * - 轮径/周长：毫米（mm）
 * - 编码器计算出的速度：米/秒（m/s）
 * - 控制目标速度接口：与控制层约定（当前工程默认使用 mm/s）
 *
 * 使用建议：
 * - 上电后调用 `Encoder_Init()` 进行参数同步与基准计数对齐（尽量在 HAL 可用后调用）
 * - 在固定周期任务中调用 `Encoder_UpdateSample()` 更新速度与增量
 * - 使用 `Encoder_CalcWheelTargets()` 生成左右轮目标速度（注意单位口径）
 *
 * 依赖：
 * - HAL 编码器读取接口：`HC_HAL_GPIO_GetEncoderCounts()`
 * - motor driver 最小状态访问接口（Motor_Get*/Motor_Set*）
 */
#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "hc_driver/motor/hc_driver_motor.h"

 /* 左右轮索引。当前底盘是两轮差速。 */
typedef enum {
    ENCODER_WHEEL_LEFT = 0,
    ENCODER_WHEEL_RIGHT,
    ENCODER_WHEEL_COUNT
} Encoder_WheelId_e;

/* 浮点参数统一放入参数表，后续调参时只需要维护索引和值。 */
typedef enum {
    ENCODER_PARAM_MIU = 0,//摩擦修正系数, 用于补偿实际速度与理论速度的差异, 通过调节该系数使得编码器计算的速度更接近实际速度
    ENCODER_PARAM_SAMPLE_PERIOD_MS,//采样周期, 单位毫秒, 影响速度计算的时间基准, 通过调节该参数可以改变速度计算的响应速度和稳定性
    ENCODER_PARAM_WHEEL_DIAMETER_MM,//轮子直径, 单位毫米, 用于计算轮子周长和速度换算系数, 通过调节该参数可以修正编码器计算的速度与实际速度的比例关系
    ENCODER_PARAM_WHEEL_CIRCUMFERENCE_MM,//轮子周长, 单位毫米, 直接用于速度换算, 通过调节该参数可以修正编码器计算的速度与实际速度的比例关系
    ENCODER_PARAM_SPEED_FACTOR,//速度换算系数, 用于将编码器增量转换为物理速度, 通过调节该参数可以修正编码器计算的速度与实际速度的比例关系
    ENCODER_PARAM_MAX_TARGET_SPEED,//最大目标速度：按控制层口径（当前工程默认 mm/s），用于限制计算出的目标速度不超过该值
    ENCODER_PARAM_TRAVEL_DISTANCE_M,//累计行驶距离, 单位米, 通过调用Encoder_UpdateTravelDistance函数更新, 可以通过Encoder_GetTravelDistance函数获取当前累计距离, 通过Encoder_ResetTravelDistance函数重置累计距离
    ENCODER_PARAM_COUNT//参数数量
} Encoder_ParamId_e;

/* 整型参数单独管理，避免脉冲数等参数被误当成浮点参数修改。 */
typedef enum {
    ENCODER_INT_PARAM_PPR = 0,//每转脉冲数, 用于计算速度换算系数, 通过调节该参数可以修正编码器计算的速度与实际速度的比例关系
    ENCODER_INT_PARAM_COUNT//整型参数数量
} Encoder_IntParamId_e;

/* 转向符号约定，用统一接口兼容不同控制环的左右轮组合方式。 */
typedef enum {
    ENCODER_TURN_SIGN_TRACK = -1,//差速底盘转向符号约定: 负值表示左转, 正值表示右转
    ENCODER_TURN_SIGN_ANGLE = 1//角度控制转向符号约定: 负值表示逆时针转, 正值表示顺时针转
} Encoder_TurnSign_e;

/* 单个编码器通道和电机实例的绑定关系。 */
typedef struct {
    Motor_Id_e motor_id;//对应的电机索引, 用于从全局电机实例数组中获取相关状态
    int8_t direction_sign;//方向符号, +1表示编码器计数与电机正转方向一致, -1表示相反, 通过调节该值可以修正编码器计数的正负关系
} Encoder_ChannelCfg_t;

extern const Encoder_ChannelCfg_t g_tEncoderChannelCfg[ENCODER_WHEEL_COUNT];//编码器通道配置表, 定义了每个编码器通道对应的电机和方向关系
extern float g_fEncoderParam[ENCODER_PARAM_COUNT];//浮点参数表, 包含了所有可调节的浮点参数, 通过索引访问和修改
extern int32_t g_iEncoderParam[ENCODER_INT_PARAM_COUNT];//整型参数表, 包含了所有可调节的整型参数, 通过索引访问和修改

/* 初始化编码器模块，建立初始计数基准并刷新速度换算系数。 */
void Encoder_Init(void);

/* 周期采样入口：读取总计数，计算增量，再更新各轮速度。 */
void Encoder_UpdateSample(void);

/* 读取左右轮累计编码器总数。 */
void Encoder_GetTotals(int32_t* p_left_total, int32_t* p_right_total);

/* 根据一个采样周期内的脉冲增量换算出物理速度。 */
float Encoder_CalcSpeed(int32_t pulse_count);

/* 修改摩擦修正系数并同步刷新速度换算系数。 */
void Encoder_SetMiu(float miu);

/* 修改采样周期并同步刷新速度换算系数。 */
void Encoder_SetSamplePeriodMs(float sample_period_ms);

/* 统一计算左右轮目标值，适用于差速底盘。 */
void Encoder_CalcWheelTargets(float base_speed,
    float turn_output,
    Encoder_TurnSign_e turn_sign,
    float* left_target,
    float* right_target);

/* 按平均轮速累计行驶距离。 */
void Encoder_UpdateTravelDistance(float left_speed, float right_speed);

/* 获取累计行驶距离。 */
float Encoder_GetTravelDistance(void);

/* 清零累计行驶距离。 */
void Encoder_ResetTravelDistance(void);

#endif /* ENCODER_H */
