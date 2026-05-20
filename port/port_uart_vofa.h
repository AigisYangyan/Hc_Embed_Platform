/**
 * @file    port_uart_vofa.h
 * @brief   VOFA 上位机串口字节流适配层接口
 *
 * 功能范围：
 * - 框架内 vofa_register 模块仅依赖 Vofa_SendBytes 一个字节流出口
 * - 用户根据目标 MCU 实现 UART 发送 (推荐 DMA + 双缓冲)
 *
 * 移植说明 (STM32F103C8T6 + HAL):
 *   void Vofa_SendBytes(const uint8_t *buf, uint16_t len)
 *   {
 *       HAL_UART_Transmit_DMA(&huart1, (uint8_t *)buf, len);
 *   }
 *
 * 设计约定：
 * - VOFA JustFloat 协议帧尾固定 4 字节 0x00 0x00 0x80 0x7F
 * - 发送阻塞或异步由用户实现自行决定，框架不假设发送语义
 */

#ifndef PORT_UART_VOFA_H
#define PORT_UART_VOFA_H

#include "hc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 向 VOFA 上位机串口发送一段字节流 */
void Vofa_SendBytes(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* PORT_UART_VOFA_H */
