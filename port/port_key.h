/**
 * @file    port_key.h
 * @brief   按键输入适配层接口
 *
 * 功能范围：
 * - 框架仅消费 4 个逻辑按键 K1~K4 (上 / 下 / 确认 / 返回)
 * - 用户根据目标 MCU 在 port_key.c 中实现 GPIO 读取与防抖
 *
 * 移植说明 (STM32F103C8T6 + HAL):
 * - 推荐 PA0~PA3 上拉输入，按下为低电平
 * - Key_Scan 在 5ms 按键采样任务中由框架自动调用，软件防抖即可
 *
 * 设计约定：
 * - Key_PollPressEvent 必须按 K1->K4 顺序取出"单次按下"事件，且取出后清除
 * - 框架不区分长按/连发，长按由用户自行扩展上层逻辑
 */

#ifndef PORT_KEY_H
#define PORT_KEY_H

#include "hc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 按键逻辑编号，顺序与板载 K1~K4 一致 */
typedef enum {
    KEY_ID_K1 = 0,  /**< 上 / 左 */
    KEY_ID_K2,      /**< 下 / 右 */
    KEY_ID_K3,      /**< 确认 / 进入 */
    KEY_ID_K4,      /**< 返回 / 退出 */
    KEY_ID_COUNT
} Key_Id_e;

/* 初始化按键 GPIO 与软件状态 */
void Key_Init(void);

/* 周期扫描按键 (建议 5ms 一次)，更新内部按下事件队列 */
void Key_Scan(void);

/* 按 K1->K4 顺序取出一个待处理的"单次按下"事件
 * 返回 HC_TRUE 时 *p_key 写入按键 ID 并清除该事件
 * 返回 HC_FALSE 表示当前无事件 */
HC_Bool_e Key_PollPressEvent(Key_Id_e *p_key);

#ifdef __cplusplus
}
#endif

#endif /* PORT_KEY_H */
