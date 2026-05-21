/**
 * @file    motor.h
 * @brief   电机驱动模块对外接口定义
 *
 * @details
 * 模块职责：
 * - 控制双路直流电机方向与 PWM 输出，提供初始化、设置输出、主动刹车等基础接口。
 * - 为上层速度/位置控制模块提供执行层（方向与占空比）能力。
 *
 * 设计约定：
 * 1. 输出范围统一为 -1000 ~ 1000（对外归一化控制量，对应底层 PWM 占空比 0%~100%，参见 MOTOR_PWM_MAX）
 * 2. 正值表示正转，负值表示反转，0 表示停止/滑行
 * 3. 本模块只负责“方向 + PWM”执行，不负责速度闭环控制；速度闭环应由上层模块实现
 * 4. 上层速度控制通常以 mm/s 为口径，将由控制层负责把速度映射为本模块的 PWM 输出
 *
 * 单位与口径：
 * - PWM/输出量：-1000..1000（归一化量，对应 0..100% 占空比）
 * - 速度值：`Motor_GetSpeed()` 返回单位为 m/s
 *
 * 依赖：
 * - 底层 GPIO 与 PWM 定时器已正确初始化
 *
 * 使用方式：
 * 1. 先调用 Motor_Init()
 * 2. 再调用 Motor_SetPwm()/Motor_Brake()/其他控制接口
 *
 * 注意：
 * - 本模块默认 PWM 已经启动
 * - 本模块不直接处理编码器采样，编码器数据由上层采样/滤波/PID 模块提供
 * - 本模块不保证线程安全；若在中断与主循环中同时调用，需自行加保护
 */

#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "hc_hal_gpio.h"
#include "hc_hal_pwm.h"
#include "middleware/pid/pid.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /* HAL 接受的最大 PWM 占空比值 (0-1000 对应 0%-100%) */
#define MOTOR_PWM_MAX   1000

/* ---- 类型定义 ----------------------------------------------------------- */

/** H桥引脚对，用于方向控制 */
    typedef struct {
        HC_HAL_GPIO_VPin_e  pin_fwd;   /**< 正转时置高的 GPIO 引脚 */
        HC_HAL_GPIO_VPin_e  pin_rev;   /**< 反转时置高的 GPIO 引脚 */
    } Motor_HBridge_T;

    /** 单个电机实例：硬件配置 + 运行时状态 */
    typedef struct {
        /* 硬件配置 (初始化时设定，之后只读) */
        Motor_HBridge_T      hbridge; //H桥驱动配置
        HC_HAL_PWM_Channel_e pwm_ch;  //PWM 输出通道
        int8_t               encoder_sign;  /**< +1 或 -1，校正编码器极性 */

        /* 运行时状态 */
        int16_t              encoder_delta; /**< 上一采样周期的脉冲增量 */
        int32_t              encoder_total; /**< 编码器累计总数           */
        float                speed;         /**< 计算后的速度 (m/s)       */

        /* PID 引用 (由 pid 模块拥有，此处仅持有指针) */
        PID_T* p_pid;
    } Motor_T;

    /**
     * 设计思想：可扩展的电机管理架构
     *
     * 1. 枚举索引化
     *    - 用 MOTOR_LEFT/MOTOR_RIGHT 代替魔法数字 0/1
     *    - MOTOR_COUNT 自动统计数量，省去硬编码
     *
     * 2. 数组容器化
     *    - 单数组管理多实例，支持批量遍历操作
     *    - 扩展4轮/6轮时，只需追加枚举项，数组自动适配
     *
     * 3. 宏别名化
     *    - g_tMotorL/g_tMotorR 语义化包装，兼顾可读性与数组灵活性
     */
     /** 电机索引枚举 (2轮差速驱动) ,方便左右轮代码中直接区分给参数*/
    typedef enum {
        MOTOR_LEFT = 0,
        MOTOR_RIGHT = 1,
        MOTOR_COUNT //电机数量
    } Motor_Id_e;

    /* ---- 全局实例 ----------------------------------------------------------- */

    extern Motor_T g_tMotors[MOTOR_COUNT];

#define g_tMotorL  (g_tMotors[MOTOR_LEFT]) 
#define g_tMotorR  (g_tMotors[MOTOR_RIGHT])

    /* ---- 公开 API ----------------------------------------------------------- */

    /** 初始化电机子系统 (在 SysInit 中调用一次) */
    void Motor_Init(void);

    /** 设置带符号 PWM：正值=正转，负值=反转，0=滑行 */
    void Motor_SetPwm(Motor_T* m, float pwm);

    /** 主动刹车：H桥两引脚同时置高，PWM=0 */
    void Motor_Brake(Motor_T* m);

    /** 获取该电机最近一次计算的速度 (m/s) */
    float Motor_GetSpeed(const Motor_T* m);

    /** 全部电机刹车 */
    void Motor_BrakeAll(void);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H__ */
