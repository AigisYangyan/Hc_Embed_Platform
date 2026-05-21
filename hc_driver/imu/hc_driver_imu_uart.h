#ifndef HC_DRIVER_IMU_UART_H
#define HC_DRIVER_IMU_UART_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float accel[3];
    float gyro[3];
    float mag[3];
    float quat[4];
    float euler[3];
    float baro[4];
    char  version[8];
} imu_measurement_t;

HC_Error_e IMU_UART_Send(const uint8_t *data, uint16_t len);

void IMU_UART_RxByte(uint8_t data);
void IMU_UART_RxBytes(volatile uint8_t *data, uint16_t len);
void IMU_UART_Process(void);

HC_Error_e IMU_UART_SendCommand(uint8_t function, const uint8_t *params, uint8_t param_len);

void IMU_UART_ClearAutoReportData(void);

HC_Error_e IMU_UART_GetAccelerometer(float out[3]);
HC_Error_e IMU_UART_GetGyroscope(float out[3]);
HC_Error_e IMU_UART_GetMagnetometer(float out[3]);
HC_Error_e IMU_UART_GetQuaternion(float out[4]);
HC_Error_e IMU_UART_GetEuler(float out[3]);
HC_Error_e IMU_UART_GetBarometer(float out[4]);
HC_Error_e IMU_UART_GetVersion(char version[8]);
HC_Error_e IMU_UART_GetAll(imu_measurement_t *out);

void  IMU_Update_Yaw_Integration(float dt_sec);
float IMU_Get_User_Yaw(void);
void  IMU_Reset_User_Yaw(void);
void  IMU_Reset_Yaw(void);
float IMU_Get_Relative_Yaw(void);

HC_Error_e IMU_Calibrate_Gyro_Offset(void);
HC_Error_e IMU_Calibrate_Gyro_Offset_Custom(uint16_t sample_count, uint16_t interval_ms);

HC_Error_e IMU_UART_CalibrationImu(void);
HC_Error_e IMU_UART_CalibrationMag(void);
HC_Error_e IMU_UART_CalibrationTemp(float now_temperature);
HC_Error_e IMU_UART_ResetUserData(void);
HC_Error_e IMU_UART_RebootDevice(void);
HC_Error_e IMU_UART_WaitCalibration(uint8_t function, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_IMU_UART_H */
