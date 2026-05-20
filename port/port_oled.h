/**
 * @file    port_oled.h
 * @brief   OLED 显示适配层接口
 *
 * 功能范围：
 * - 框架仅依赖本文件中的 OLED 抽象接口，不关心底层是 SSD1306/SH1106/IIC/SPI
 * - 用户在移植时需根据目标 MCU 实现下列符号
 *
 * 移植说明 (STM32F103C8T6 + HAL):
 * - OLED_Init: HAL_I2C_Init + 写 SSD1306 初始化序列
 * - OLED_ShowString: 8x16 字模 + I2C 数据流
 * - 详细代码片段见 docs/HC_Sys_Menu/PORTING_GUIDE_STM32F103.md
 *
 * 设计约定：
 * - 行/列采用页地址 (page) 模式：8 像素为 1 page，128x64 OLED 共 8 page
 * - x 单位为像素列 (0~127)，y 单位为 page (0~7)
 * - size 字号建议固定 16 (8x16)，如需 6x8 由用户自行扩展
 */

#ifndef PORT_OLED_H
#define PORT_OLED_H

#include "hc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 初始化 OLED 控制器 (上电序列 + 清屏) */
void OLED_Init(void);

/* 整屏清空，0 像素 */
void OLED_Clear(void);

/* 在 (x 像素列, y page) 处显示一段以 '\0' 结尾的字符串
 * size = 16 表示 8x16 字模，其它字号由用户自行实现或忽略 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *text, uint8_t size);

/* 在 (x 像素列, y page) 处显示单个 ASCII 字符 */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif /* PORT_OLED_H */
