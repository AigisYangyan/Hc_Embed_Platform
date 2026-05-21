#include "hc_driver_imu_uart.h"
#include "hc_hal_uart.h"
#include "hc_hal_systick.h"

#include <string.h>
#include <math.h>

/* ── Protocol constants (private) ──────────────────────────────────── */
#define IMU_UART_RX_BUF_SIZE    256u
#define IMU_UART_CH             HC_HAL_UART_CH_0

#define FRAME_HEAD1  0x7Eu
#define FRAME_HEAD2  0x23u

#define IMU_FUNC_VERSION        0x01u
#define IMU_FUNC_RAW_ACCEL      0x04u
#define IMU_FUNC_RAW_GYRO       0x0Au
#define IMU_FUNC_RAW_MAG        0x10u
#define IMU_FUNC_QUAT           0x16u
#define IMU_FUNC_EULER          0x26u
#define IMU_FUNC_BARO           0x32u
#define IMU_FUNC_CALIB_IMU      0x70u
#define IMU_FUNC_CALIB_MAG      0x71u
#define IMU_FUNC_CALIB_BARO     0x72u
#define IMU_FUNC_CALIB_TEMP     0x73u
#define IMU_FUNC_REQUEST_DATA   0x80u
#define IMU_FUNC_RETURN_STATE   0x81u
#define IMU_FUNC_RESET_FLASH    0xA0u

/* ── RX ring buffer ────────────────────────────────────────────────── */
static volatile uint8_t  s_rx_buffer[IMU_UART_RX_BUF_SIZE];
static volatile uint16_t s_rx_write_index = 0u;
static volatile uint16_t s_rx_read_index  = 0u;

/* ── Gyro integration state ────────────────────────────────────────── */
static float    s_integrated_yaw_deg    = 0.0f;
static float    s_yaw_reference_deg     = 0.0f;
static float    s_gyro_bias_rad         = 0.0f;
static uint8_t  s_gyro_bias_valid       = 0u;
static uint8_t  s_yaw_synced_from_euler = 0u;
static uint8_t  s_integrator_tick_init  = 0u;
static uint32_t s_last_integrate_tick   = 0u;

static const float GYRO_DEADBAND_RAD_S = 0.002f;
static const float RAD2DEG             = 57.29577951308232f;

/* ── Cached sensor state ───────────────────────────────────────────── */
static volatile float s_ax = 0.0f, s_ay = 0.0f, s_az = 0.0f;
static volatile float s_gx = 0.0f, s_gy = 0.0f, s_gz = 0.0f;
static volatile float s_mx = 0.0f, s_my = 0.0f, s_mz = 0.0f;
static volatile float s_roll = 0.0f, s_pitch = 0.0f, s_yaw = 0.0f;
static volatile float s_q0 = 0.0f, s_q1 = 0.0f, s_q2 = 0.0f, s_q3 = 0.0f;
static volatile float s_height = 0.0f, s_temperature = 0.0f, s_pressure = 0.0f, s_pressure_contrast = 0.0f;
static volatile int   s_version_high = -1, s_version_mid = 0, s_version_low = 0;
static volatile uint8_t  s_last_rx_function = 0u;
static volatile int16_t  s_last_rx_state = 0;

/* ── Private helpers ────────────────────────────────────────────────── */

static float wrap_deg180(float deg)
{
    if (deg >= 180.0f || deg < -180.0f) {
        deg = fmodf(deg, 360.0f);
        if (deg >= 180.0f) {
            deg -= 360.0f;
        } else if (deg < -180.0f) {
            deg += 360.0f;
        }
    }
    return deg;
}

static void gyro_integrator_step(float gz_rad, float dt_sec)
{
    if (dt_sec <= 0.0f) {
        return;
    }
    float gz = gz_rad - (s_gyro_bias_valid ? s_gyro_bias_rad : 0.0f);
    if (fabsf(gz) < GYRO_DEADBAND_RAD_S) {
        gz = 0.0f;
    }
    s_integrated_yaw_deg += gz * dt_sec * RAD2DEG;
}

static void gyro_integrator_update_auto(void)
{
    uint32_t now_tick;
    if (HC_HAL_SYSTICK_GetTickMs(&now_tick) != HC_HAL_OK) {
        return;
    }
    if (!s_integrator_tick_init) {
        s_last_integrate_tick  = now_tick;
        s_integrator_tick_init = 1u;
        return;
    }
    uint32_t delta_ms = now_tick - s_last_integrate_tick;
    s_last_integrate_tick = now_tick;
    if (delta_ms == 0u || delta_ms > 100u) {
        return;
    }
    float dt = (float)delta_ms * 0.001f;
    gyro_integrator_step(s_gz, dt);
}

static void imu_busy_delay_ms(uint32_t ms)
{
    uint32_t start;
    if (HC_HAL_SYSTICK_GetTickMs(&start) != HC_HAL_OK) {
        return;
    }
    for (;;) {
        uint32_t now;
        if (HC_HAL_SYSTICK_GetTickMs(&now) != HC_HAL_OK) {
            return;
        }
        if ((now - start) >= ms) {
            return;
        }
    }
}

static int gyro_calibrate_bias(uint16_t sample_count, uint16_t interval_ms)
{
    if (sample_count == 0u) {
        return -1;
    }
    float yaw_snapshot = s_integrated_yaw_deg;
    float sum = 0.0f;

    uint32_t start_tick = 0u;
    HC_HAL_SYSTICK_GetTickMs(&start_tick);
    const uint32_t timeout_ms = 3000u;

    for (uint16_t i = 0; i < sample_count; ++i) {
        uint32_t now_tick = 0u;
        HC_HAL_SYSTICK_GetTickMs(&now_tick);
        if ((now_tick - start_tick) > timeout_ms) {
            return -2;
        }
        IMU_UART_Process();
        sum += s_gz;
        if (interval_ms > 0u) {
            imu_busy_delay_ms(interval_ms);
        }
    }
    s_gyro_bias_rad   = sum / (float)sample_count;
    s_gyro_bias_valid = 1u;
    s_integrated_yaw_deg = yaw_snapshot;
    s_integrator_tick_init = 0u;
    return 0;
}

/* ── Ring buffer operations ─────────────────────────────────────────── */

static inline uint16_t rxbuf_next(uint16_t index)
{
    return (uint16_t)((index + 1u) % IMU_UART_RX_BUF_SIZE);
}

static inline int rxbuf_is_empty(void)
{
    return s_rx_write_index == s_rx_read_index;
}

static inline void rxbuf_push(uint8_t byte_value)
{
    uint16_t next_index = rxbuf_next(s_rx_write_index);
    if (next_index == s_rx_read_index) {
        s_rx_read_index = rxbuf_next(s_rx_read_index);
    }
    s_rx_buffer[s_rx_write_index] = byte_value;
    s_rx_write_index = next_index;
}

static inline int rxbuf_pop(uint8_t *out_byte)
{
    if (rxbuf_is_empty()) {
        return -1;
    }
    *out_byte = s_rx_buffer[s_rx_read_index];
    s_rx_read_index = rxbuf_next(s_rx_read_index);
    return 0;
}

/* ── Byte conversion ────────────────────────────────────────────────── */

static int16_t to_int16(const uint8_t *bytes)
{
    return (int16_t)((bytes[1] << 8) + bytes[0]);
}

static float to_float(const uint8_t *bytes)
{
    float v;
    memcpy(&v, bytes, sizeof(float));
    return v;
}

/* ── Frame parser ───────────────────────────────────────────────────── */

static void parse_frame_data(uint8_t frame_function, const uint8_t *frame_data)
{
    if (frame_function == IMU_FUNC_RAW_ACCEL) {
        float accel_ratio = 16.0f / 32767.0f;
        s_ax = to_int16(&frame_data[0])  * accel_ratio;
        s_ay = to_int16(&frame_data[2])  * accel_ratio;
        s_az = to_int16(&frame_data[4])  * accel_ratio;

        float deg_to_rad = 3.14159265358979323846f / 180.0f;
        float gyro_ratio  = (2000.0f / 32767.0f) * deg_to_rad;
        s_gx = to_int16(&frame_data[6])  * gyro_ratio;
        s_gy = to_int16(&frame_data[8])  * gyro_ratio;
        s_gz = to_int16(&frame_data[10]) * gyro_ratio;

        gyro_integrator_update_auto();

        float mag_ratio = 800.0f / 32767.0f;
        s_mx = to_int16(&frame_data[12]) * mag_ratio;
        s_my = to_int16(&frame_data[14]) * mag_ratio;
        s_mz = to_int16(&frame_data[16]) * mag_ratio;
    } else if (frame_function == IMU_FUNC_EULER) {
        s_roll  = to_float(&frame_data[0]);
        s_pitch = to_float(&frame_data[4]);
        s_yaw   = to_float(&frame_data[8]);

        if (!s_yaw_synced_from_euler) {
            s_integrated_yaw_deg = s_yaw * RAD2DEG;
            s_yaw_synced_from_euler = 1u;
        }
    } else if (frame_function == IMU_FUNC_QUAT) {
        s_q0 = to_float(&frame_data[0]);
        s_q1 = to_float(&frame_data[4]);
        s_q2 = to_float(&frame_data[8]);
        s_q3 = to_float(&frame_data[12]);
    } else if (frame_function == IMU_FUNC_BARO) {
        s_height            = to_float(&frame_data[0]);
        s_temperature       = to_float(&frame_data[4]);
        s_pressure          = to_float(&frame_data[8]);
        s_pressure_contrast = to_float(&frame_data[12]);
    } else if (frame_function == IMU_FUNC_VERSION) {
        s_version_high = frame_data[0];
        s_version_mid  = frame_data[1];
        s_version_low  = frame_data[2];
    } else if (frame_function == IMU_FUNC_RETURN_STATE) {
        s_last_rx_function = frame_data[0];
        s_last_rx_state    = (int16_t)frame_data[1];
    }
}

/* ── UART transport (private) ───────────────────────────────────────── */

static void imu_uart_send_byte(uint8_t data)
{
    (void)HC_HAL_UART_SendByte(IMU_UART_CH, data);
}

static void imu_uart_send_array(const uint8_t *p_data, uint8_t length)
{
    if ((p_data == NULL) || (length == 0u)) {
        return;
    }
    (void)HC_HAL_UART_SendBuffer(IMU_UART_CH, p_data, (uint32_t)length);
}

/* ── Public API ─────────────────────────────────────────────────────── */

HC_Error_e IMU_UART_SendCommand(uint8_t function, const uint8_t *params, uint8_t param_len)
{
    if (param_len > 3u || (param_len > 0u && params == NULL)) {
        return HC_HAL_ERR_INVALID;
    }
    uint8_t frame[8] = {FRAME_HEAD1, FRAME_HEAD2, 0, function, 0, 0, 0, 0};
    for (uint8_t i = 0; i < param_len; ++i) {
        frame[4 + i] = params[i];
    }
    uint8_t frame_len = (uint8_t)(4u + param_len + 1u);
    frame[2] = frame_len;
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < frame_len - 1u; ++i) {
        checksum = (uint8_t)(checksum + frame[i]);
    }
    frame[frame_len - 1u] = checksum;
    imu_uart_send_array(frame, frame_len);
    return HC_HAL_OK;
}

void IMU_UART_RxByte(uint8_t data)
{
    rxbuf_push(data);
}

void IMU_UART_RxBytes(volatile uint8_t *data, uint16_t len)
{
    if (!data || len == 0u) return;
    for (uint16_t i = 0; i < len; ++i) {
        rxbuf_push(data[i]);
    }
}

void IMU_UART_Process(void)
{
    enum {
        RX_STATE_EXPECT_HEAD1 = 0,
        RX_STATE_EXPECT_HEAD2,
        RX_STATE_EXPECT_LENGTH,
        RX_STATE_EXPECT_FUNCTION,
        RX_STATE_COLLECT_DATA
    };
    static uint8_t  rx_state = RX_STATE_EXPECT_HEAD1;
    static uint8_t  frame_length = 0;
    static uint8_t  frame_function = 0;
    static uint8_t  frame_buffer[64];
    static uint16_t frame_index = 0;

    uint8_t current_byte = 0;
    while (rxbuf_pop(&current_byte) == 0) {
        switch (rx_state) {
        case RX_STATE_EXPECT_HEAD1:
            rx_state = (current_byte == FRAME_HEAD1) ? RX_STATE_EXPECT_HEAD2 : RX_STATE_EXPECT_HEAD1;
            break;
        case RX_STATE_EXPECT_HEAD2:
            rx_state = (current_byte == FRAME_HEAD2) ? RX_STATE_EXPECT_LENGTH : RX_STATE_EXPECT_HEAD1;
            break;
        case RX_STATE_EXPECT_LENGTH:
            frame_length = current_byte;
            rx_state = RX_STATE_EXPECT_FUNCTION;
            break;
        case RX_STATE_EXPECT_FUNCTION:
            frame_function = current_byte;
            frame_index = 0;
            rx_state = RX_STATE_COLLECT_DATA;
            break;
        case RX_STATE_COLLECT_DATA: {
            uint16_t data_length = (frame_length >= 4u) ? (uint16_t)(frame_length - 4u) : 0u;
            if (data_length == 0u || data_length > sizeof(frame_buffer)) {
                rx_state = RX_STATE_EXPECT_HEAD1;
                break;
            }
            frame_buffer[frame_index++] = current_byte;
            if (frame_index >= data_length) {
                uint8_t calculated_checksum = (uint8_t)(FRAME_HEAD1 + FRAME_HEAD2 + frame_length + frame_function);
                for (uint16_t i = 0; i < data_length - 1u; ++i) {
                    calculated_checksum = (uint8_t)(calculated_checksum + frame_buffer[i]);
                }
                uint8_t received_checksum = frame_buffer[data_length - 1u];
                if (calculated_checksum == received_checksum) {
                    parse_frame_data(frame_function, frame_buffer);
                }
                rx_state = RX_STATE_EXPECT_HEAD1;
            }
        } break;
        default:
            rx_state = RX_STATE_EXPECT_HEAD1;
            break;
        }
    }
}

/* ── Data accessors ─────────────────────────────────────────────────── */

HC_Error_e IMU_UART_GetAccelerometer(float out[3])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_ax; out[1] = s_ay; out[2] = s_az;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetGyroscope(float out[3])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_gx; out[1] = s_gy; out[2] = s_gz;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetMagnetometer(float out[3])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_mx; out[1] = s_my; out[2] = s_mz;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetQuaternion(float out[4])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_q0; out[1] = s_q1; out[2] = s_q2; out[3] = s_q3;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetEuler(float out[3])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_roll  * RAD2DEG;
    out[1] = s_pitch * RAD2DEG;
    out[2] = s_yaw   * RAD2DEG;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetBarometer(float out[4])
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    out[0] = s_height; out[1] = s_temperature; out[2] = s_pressure; out[3] = s_pressure_contrast;
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetVersion(char version[8])
{
    if (!version) return HC_HAL_ERR_NULL_PTR;

    if (s_version_high < 0) {
        uint8_t payload[2] = {IMU_FUNC_VERSION, 0x00};
        IMU_UART_SendCommand(IMU_FUNC_REQUEST_DATA, payload, (uint8_t)sizeof(payload));
        for (int i = 0; i < 20; ++i) {
            IMU_UART_Process();
            if (s_version_high >= 0) {
                version[0] = (char)s_version_high;
                version[1] = (char)s_version_mid;
                version[2] = (char)s_version_low;
                version[3] = '\0';
                return HC_HAL_OK;
            }
            imu_busy_delay_ms(5u);
        }
        return HC_HAL_ERR_TIMEOUT;
    }
    version[0] = (char)s_version_high;
    version[1] = (char)s_version_mid;
    version[2] = (char)s_version_low;
    version[3] = '\0';
    return HC_HAL_OK;
}

HC_Error_e IMU_UART_GetAll(imu_measurement_t *out)
{
    if (!out) return HC_HAL_ERR_NULL_PTR;
    IMU_UART_GetAccelerometer(out->accel);
    IMU_UART_GetGyroscope(out->gyro);
    IMU_UART_GetMagnetometer(out->mag);
    IMU_UART_GetQuaternion(out->quat);
    IMU_UART_GetEuler(out->euler);
    IMU_UART_GetBarometer(out->baro);
    return HC_HAL_OK;
}

void IMU_UART_ClearAutoReportData(void)
{
    s_ax = s_ay = s_az = 0.0f;
    s_gx = s_gy = s_gz = 0.0f;
    s_mx = s_my = s_mz = 0.0f;
    s_roll = s_pitch = s_yaw = 0.0f;
    s_q0 = s_q1 = s_q2 = s_q3 = 0.0f;
    s_height = s_temperature = s_pressure = s_pressure_contrast = 0.0f;
    s_integrated_yaw_deg = 0.0f;
    s_yaw_reference_deg = 0.0f;
    s_gyro_bias_rad = 0.0f;
    s_gyro_bias_valid = 0u;
    s_yaw_synced_from_euler = 0u;
    s_integrator_tick_init = 0u;
}

/* ── Calibration helpers ────────────────────────────────────────────── */

static int calibration_with_wait(uint8_t function, const uint8_t *payload, uint8_t payload_len,
                                 uint32_t timeout_ms)
{
    s_last_rx_function = 0u;
    s_last_rx_state = -1;

    HC_Error_e rc = IMU_UART_SendCommand(function, payload, payload_len);
    if (rc != HC_HAL_OK) {
        return -1;
    }
    return IMU_UART_WaitCalibration(function, timeout_ms);
}

HC_Error_e IMU_UART_CalibrationImu(void)
{
    uint8_t payload[2] = {0x01u, 0x5Fu};
    int result = calibration_with_wait(IMU_FUNC_CALIB_IMU, payload, (uint8_t)sizeof(payload), 7000u);
    return (result == 1) ? HC_HAL_OK : HC_HAL_ERR_TIMEOUT;
}

HC_Error_e IMU_UART_CalibrationMag(void)
{
    uint8_t payload[2] = {0x01u, 0x5Fu};
    int result = calibration_with_wait(IMU_FUNC_CALIB_MAG, payload, (uint8_t)sizeof(payload), 0u);
    return (result == 1) ? HC_HAL_OK : HC_HAL_ERR_TIMEOUT;
}

HC_Error_e IMU_UART_CalibrationTemp(float now_temperature)
{
    if (now_temperature > 50.0f || now_temperature < -50.0f) {
        return HC_HAL_ERR_INVALID;
    }
    int16_t temperature_raw = (int16_t)(now_temperature * 100.0f);
    uint8_t param_low  = (uint8_t)(temperature_raw & 0xFFu);
    uint8_t param_high = (uint8_t)((temperature_raw >> 8) & 0xFFu);
    uint8_t payload[3] = {param_low, param_high, 0x5Fu};
    int result = calibration_with_wait(IMU_FUNC_CALIB_TEMP, payload, (uint8_t)sizeof(payload), 2000u);
    return (result == 1) ? HC_HAL_OK : HC_HAL_ERR_TIMEOUT;
}

HC_Error_e IMU_UART_ResetUserData(void)
{
    uint8_t payload[2] = {0x01u, 0x5Fu};
    return IMU_UART_SendCommand(IMU_FUNC_RESET_FLASH, payload, (uint8_t)sizeof(payload));
}

HC_Error_e IMU_UART_WaitCalibration(uint8_t function, uint32_t timeout_ms)
{
    uint32_t elapsed_ms = 0u;
    while (1) {
        IMU_UART_Process();
        if (s_last_rx_function == function) {
            return (s_last_rx_state == 0) ? HC_HAL_OK : HC_HAL_ERR_INVALID;
        }
        if (timeout_ms != 0u && elapsed_ms >= timeout_ms) {
            return HC_HAL_ERR_TIMEOUT;
        }
        imu_busy_delay_ms(1u);
        if (timeout_ms != 0u) {
            ++elapsed_ms;
        }
    }
}

/* ── Yaw integration ────────────────────────────────────────────────── */

void IMU_Update_Yaw_Integration(float dt_sec)
{
    gyro_integrator_step(s_gz, dt_sec);
    s_integrator_tick_init = 0u;
}

float IMU_Get_User_Yaw(void)
{
    return s_integrated_yaw_deg;
}

void IMU_Reset_User_Yaw(void)
{
    s_integrated_yaw_deg = 0.0f;
    s_yaw_reference_deg  = 0.0f;
    s_yaw_synced_from_euler = 0u;
    s_integrator_tick_init = 0u;
}

void IMU_Reset_Yaw(void)
{
    s_yaw_reference_deg = s_integrated_yaw_deg;
}

float IMU_Get_Relative_Yaw(void)
{
    float relative = s_integrated_yaw_deg - s_yaw_reference_deg;
    return wrap_deg180(relative);
}

HC_Error_e IMU_Calibrate_Gyro_Offset(void)
{
    int rc = gyro_calibrate_bias(500u, 2u);
    return (rc == 0) ? HC_HAL_OK : HC_HAL_ERR_TIMEOUT;
}

HC_Error_e IMU_Calibrate_Gyro_Offset_Custom(uint16_t sample_count, uint16_t interval_ms)
{
    int rc = gyro_calibrate_bias(sample_count, interval_ms);
    return (rc == 0) ? HC_HAL_OK : HC_HAL_ERR_TIMEOUT;
}
