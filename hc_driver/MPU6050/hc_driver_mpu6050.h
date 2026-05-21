#ifndef HC_DRIVER_MPU6050_H
#define HC_DRIVER_MPU6050_H

#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double  Ax;
    double  Ay;
    double  Az;
    double  Gx;
    double  Gy;
    double  Gz;
    float   Temperature;
    double  KalmanAngleX;
    double  KalmanAngleY;
} MPU6050_T;

HC_Error_e MPU6050_Init(void);
HC_Error_e MPU6050_Read_Accel(MPU6050_T *p);
HC_Error_e MPU6050_Read_Gyro(MPU6050_T *p);
HC_Error_e MPU6050_Read_Temp(MPU6050_T *p);
HC_Error_e MPU6050_Read_All(MPU6050_T *p);

#ifdef __cplusplus
}
#endif

#endif /* HC_DRIVER_MPU6050_H */
