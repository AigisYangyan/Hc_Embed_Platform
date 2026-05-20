#ifndef HC_BOARD_CFG_H
#define HC_BOARD_CFG_H

#include "hc_target_cfg.h"

/*
 * hc_board_cfg.h — canonical board-level resource mapping.
 *
 * All pin, instance, and clock mappings that vary between boards or targets
 * live here.  Project-specific aliases (VOFA, VISION, MOTOR_L, etc.) do NOT
 * belong here; keep those in the legacy compat layer (port/hc_hal/inc/).
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

#endif /* HC_BOARD_CFG_H */
