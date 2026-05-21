#ifndef HC_DRIVER_STEPPER_EMM42_H
#define HC_DRIVER_STEPPER_EMM42_H

#include "hc_common/hc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Device-level constants ─────────────────────────────────────────── */

#define EMM42_SPEED_MIN_RPM             0u
#define EMM42_SPEED_MAX_RPM             100u
#define EMM42_ACCEL_MIN_GRADE           0u
#define EMM42_ACCEL_MAX_GRADE           255u

#define EMM42_MICROSTEP                 256u
#define EMM42_PULSES_PER_REVOLUTION     51200u

#define EMM42_ENABLE_ON                 0x01u
#define EMM42_ENABLE_OFF                0x00u

#define EMM42_DIR_CW                    0x01u
#define EMM42_DIR_CCW                   0x00u

#define EMM42_POSITION_MODE_RELATIVE    0u
#define EMM42_POSITION_MODE_ABSOLUTE    1u

/* ── Types ──────────────────────────────────────────────────────────── */

typedef enum {
    EMM42_AXIS_Y = 1u,
    EMM42_AXIS_X = 2u
} Emm42_Axis_e;

/* ── Public API ─────────────────────────────────────────────────────── */

void Emm42_SendEnableCommand(uint8_t axis_id, uint8_t enable_status);
void Emm42_SendSpeedCommand(uint8_t axis_id, uint8_t direction, uint16_t speed, uint8_t acceleration);
void Emm42_SendPositionCommand(uint8_t axis_id, uint8_t direction, uint16_t speed,
                               uint8_t acceleration, uint32_t pulses, uint8_t mod);
void Emm42_SendReadSpeedCommand(uint8_t axis_id);
void Emm42_SendPidConfigCommand(uint8_t axis_id, uint8_t save_to_flash,
                                uint32_t kp, uint32_t ki, uint32_t kd);

void Emm42_EnableAll(void);
void Emm42_DisableAll(void);
void Emm42_SetAllAxesZero(void);

void Emm42_MoveRelative(Emm42_Axis_e axis, int32_t pulses, uint16_t speed, uint8_t acceleration);
void Emm42_MoveAbsolute(Emm42_Axis_e axis, uint32_t position_pulses, uint16_t speed);

void Emm42_SetZeroPosition(uint8_t axis_id);
void Emm42_StartHoming(uint8_t axis_id);
void Emm42_ExitHoming(uint8_t axis_id);

void Emm42_RunCommandTask(void);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_STEPPER_EMM42_H */
