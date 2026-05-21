/**
 * @file    hc_hal_board_cfg.h
 * @brief   板级 HAL 统一配置头，汇总各模块实例与特性开关。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#ifndef HC_HAL_BOARD_CFG_H
#define HC_HAL_BOARD_CFG_H

#include "hc_common/hc_types.h"
#include "ti_msp_dl_config.h"

#define HC_HAL_SYSTICK_CPU_HZ           32000000u
#define HC_HAL_SYSTICK_TICK_HZ          1000u
#define HC_HAL_SYSTICK_DELAY_US_CYCLES  4u

/* I2C_OLED (I2C1) */
#define HC_HAL_I2C_OLED_INST          I2C_OLED_INST
#define HC_HAL_I2C_OLED_SDA_PORT      GPIO_I2C_OLED_SDA_PORT
#define HC_HAL_I2C_OLED_SDA_PIN       GPIO_I2C_OLED_SDA_PIN
#define HC_HAL_I2C_OLED_SDA_IOMUX     GPIO_I2C_OLED_IOMUX_SDA
#define HC_HAL_I2C_OLED_SDA_FUNC      GPIO_I2C_OLED_IOMUX_SDA_FUNC
#define HC_HAL_I2C_OLED_SCL_PORT      GPIO_I2C_OLED_SCL_PORT
#define HC_HAL_I2C_OLED_SCL_PIN       GPIO_I2C_OLED_SCL_PIN
#define HC_HAL_I2C_OLED_SCL_IOMUX     GPIO_I2C_OLED_IOMUX_SCL
#define HC_HAL_I2C_OLED_SCL_FUNC      GPIO_I2C_OLED_IOMUX_SCL_FUNC

/* Motor PWM */
#define HC_HAL_PWM_DUTY_MAX             1000u

#define HC_HAL_PWM_MOTOR_L_INST         PWM_MOTOR_L_INST
#define HC_HAL_PWM_MOTOR_L_CC_INDEX     GPIO_PWM_MOTOR_L_C1_IDX
#define HC_HAL_PWM_MOTOR_L_PERIOD       HC_HAL_PWM_DUTY_MAX

#define HC_HAL_PWM_MOTOR_R_INST         PWM_MOTOR_R_INST
#define HC_HAL_PWM_MOTOR_R_CC_INDEX     GPIO_PWM_MOTOR_R_C0_IDX
#define HC_HAL_PWM_MOTOR_R_PERIOD       HC_HAL_PWM_DUTY_MAX

#endif /* HC_HAL_BOARD_CFG_H */
