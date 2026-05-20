/**
 * @file    hc_board_cfg.h
 * @brief   HC_EMBED_RULES 板级引脚与资源映射
 *
 * 所有与具体 PCB 相关的引脚映射、外设实例编号、总线地址
 * 集中定义于此。更换硬件时只需修改本文件。
 */

#ifndef HC_BOARD_CFG_H
#define HC_BOARD_CFG_H

#include "hc_cfg/hc_target_cfg.h"

/* ---- 按键引脚 ------------------------------------------------------------- */

#define HC_KEY1_GPIO_PORT     GPIOA
#define HC_KEY1_GPIO_PIN      GPIO_PIN_0
#define HC_KEY2_GPIO_PORT     GPIOA
#define HC_KEY2_GPIO_PIN      GPIO_PIN_1
#define HC_KEY3_GPIO_PORT     GPIOA
#define HC_KEY3_GPIO_PIN      GPIO_PIN_2
#define HC_KEY4_GPIO_PORT     GPIOA
#define HC_KEY4_GPIO_PIN      GPIO_PIN_3

/* ---- OLED 接口 ------------------------------------------------------------ */

#define HC_OLED_I2C_INSTANCE  I2C1
#define HC_OLED_I2C_ADDR      0x3C
#define HC_OLED_WIDTH         128
#define HC_OLED_HEIGHT        64

/* ---- UART (VOFA 遥测) ----------------------------------------------------- */

#define HC_VOFA_UART_INSTANCE USART1
#define HC_VOFA_UART_BAUDRATE 115200

/* ---- SysTick -------------------------------------------------------------- */

#define HC_SYSTICK_PERIOD_MS  1

#endif /* HC_BOARD_CFG_H */
