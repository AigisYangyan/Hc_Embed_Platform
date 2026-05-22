#ifndef HC_BOARD_CFG_H
#define HC_BOARD_CFG_H

#include "hc_target_cfg.h"
#include "hc_hal_i2c.h"
#include "hc_hal_uart.h"

/*
 * hc_board_cfg.h — canonical board-level resource mapping.
 *
 * All pin, instance, and clock mappings that vary between boards or targets
 * live here. Drivers reference these macros instead of hardcoding HAL channel
 * values or platform-private names.
 */

/* ── System clocks ─────────────────────────────────────────────────── */
#if defined(HC_TARGET_STM32F1)
  #define HC_BOARD_SYSTICK_CPU_HZ           72000000u
  #define HC_BOARD_SYSTICK_TICK_HZ          1000u
  #define HC_BOARD_SYSTICK_DELAY_US_CYCLES  5u
#elif defined(HC_TARGET_MSPM0)
  #define HC_BOARD_SYSTICK_CPU_HZ           32000000u
  #define HC_BOARD_SYSTICK_TICK_HZ          1000u
  #define HC_BOARD_SYSTICK_DELAY_US_CYCLES  4u
#endif

/* ── PWM ───────────────────────────────────────────────────────────── */
#define HC_BOARD_PWM_DUTY_MAX  1000u

/* ── Encoder ─────────────────────────────────────────────────────────── */
#define HC_BOARD_ENCODER_LEFT_REVERSE             HC_FALSE
#define HC_BOARD_ENCODER_LEFT_PPR                 11u
#define HC_BOARD_ENCODER_LEFT_QUADRATURE_MULTIPLE 4u
#define HC_BOARD_ENCODER_RIGHT_REVERSE            HC_FALSE
#define HC_BOARD_ENCODER_RIGHT_PPR                11u
#define HC_BOARD_ENCODER_RIGHT_QUADRATURE_MULTIPLE 4u

/* ── I2C channel assignments (driver-facing) ──────────────────────── */
#if defined(HC_TARGET_STM32F1)
  #define HC_BOARD_I2C_CH_OLED         HC_HAL_I2C_CH_0
  #define HC_BOARD_I2C_CH_EEPROM       HC_HAL_I2C_CH_0
  #define HC_BOARD_I2C_CH_MPU6050      HC_HAL_I2C_CH_0
#elif defined(HC_TARGET_MSPM0)
  #define HC_BOARD_I2C_CH_OLED         HC_HAL_I2C_CH_0
  #define HC_BOARD_I2C_CH_EEPROM       HC_HAL_I2C_CH_0
  #define HC_BOARD_I2C_CH_MPU6050      HC_HAL_I2C_CH_0
#endif

/* ── UART channel assignments (driver-facing) ─────────────────────── */
#if defined(HC_TARGET_STM32F1)
  #define HC_BOARD_UART_CH_VOFA        HC_HAL_UART_CH_1
  #define HC_BOARD_UART_CH_IMU         HC_HAL_UART_CH_2
#elif defined(HC_TARGET_MSPM0)
  #define HC_BOARD_UART_CH_VOFA        HC_HAL_UART_CH_0
  #define HC_BOARD_UART_CH_IMU         HC_HAL_UART_CH_1
#endif

#endif /* HC_BOARD_CFG_H */
