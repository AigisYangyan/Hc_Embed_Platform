/**
 * @file    port_systick.h
 * @brief   双后端 SysTick 适配说明
 *
 * 功能范围：
 * - 裸机模式：用户在 1ms SysTick 中调用 TaskTimeSliceManage()
 * - FreeRTOS 模式：SysTick/RTOS tick 由 FreeRTOS 移植层统一管理
 *
 * 移植说明 (STM32F103C8T6 + HAL):
 * - 裸机模式下保持 SysTick = 1kHz，并在中断中只做时间片推进
 * - FreeRTOS 模式下保持默认 SysTick 与 PendSV 配置
 *
 * 设计约定：
 * - 中断只做最小时间推进，不做耗时业务处理
 * - 裸机/RTOS 后端都通过 runtime.h 的统一接口运行
 */

#ifndef PORT_SYSTICK_H
#define PORT_SYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif

void TaskTimeSliceManage(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_SYSTICK_H */
