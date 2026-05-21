#include "hc_driver_mpu6050.h"
#include "hc_hal_i2c.h"
#include "hc_hal_systick.h"

#include <math.h>

/* ── Private hardware binding ───────────────────────────────────────── */
#define MPU6050_I2C_CH          I2C_CH_MPU6050
#define MPU6050_DEV_ADDR_7BIT   0x68u
#define MPU6050_WHO_AM_I_VAL    0x68u

#define REG_SMPLRT_DIV          0x19u
#define REG_GYRO_CONFIG         0x1Bu
#define REG_ACCEL_CONFIG        0x1Cu
#define REG_ACCEL_XOUT_H        0x3Bu
#define REG_TEMP_OUT_H          0x41u
#define REG_GYRO_XOUT_H         0x43u
#define REG_PWR_MGMT_1          0x6Bu
#define REG_WHO_AM_I            0x75u

#define ACCEL_LSB_PER_G         16384.0
#define GYRO_LSB_PER_DPS        131.0
#define ACCEL_Z_CORRECTOR       14418.0
#define RAD_TO_DEG              57.295779513082320876798154814105

/* ── Private device instance ────────────────────────────────────────── */

typedef struct {
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_T;

typedef struct {
    HC_HAL_I2C_Ch_e i2c_ch;
    HC_U8           dev_addr;
    MPU6050_T       data;
} MPU6050_Dev_T;

static MPU6050_Dev_T s_dev = {
    .i2c_ch   = MPU6050_I2C_CH,
    .dev_addr = MPU6050_DEV_ADDR_7BIT,
};

static Kalman_T s_kalman_x = {
    .Q_angle   = 0.001,
    .Q_bias    = 0.003,
    .R_measure = 0.03,
};

static Kalman_T s_kalman_y = {
    .Q_angle   = 0.001,
    .Q_bias    = 0.003,
    .R_measure = 0.03,
};

static uint32_t s_tick_last_ms = 0u;

/* ── Private helpers ────────────────────────────────────────────────── */

static uint32_t mpu_tick_ms(void)
{
    uint32_t ms = 0u;
    (void)HC_HAL_SYSTICK_GetTickMs(&ms);
    return ms;
}

static HC_Error_e mpu_write_reg(HC_U8 reg, HC_U8 val)
{
    return HC_HAL_I2C_MemWrite(s_dev.i2c_ch, s_dev.dev_addr, reg, &val, 1u);
}

static HC_Error_e mpu_read_regs(HC_U8 reg, HC_U8 *buf, HC_U16 len)
{
    return HC_HAL_I2C_MemRead(s_dev.i2c_ch, s_dev.dev_addr, reg, buf, len);
}

static double kalman_get_angle(Kalman_T *k, double new_angle, double new_rate, double dt)
{
    double rate = new_rate - k->bias;
    k->angle += dt * rate;

    k->P[0][0] += dt * (dt * k->P[1][1] - k->P[0][1] - k->P[1][0] + k->Q_angle);
    k->P[0][1] -= dt * k->P[1][1];
    k->P[1][0] -= dt * k->P[1][1];
    k->P[1][1] += k->Q_bias * dt;

    double S = k->P[0][0] + k->R_measure;
    double K[2];
    K[0] = k->P[0][0] / S;
    K[1] = k->P[1][0] / S;

    double y = new_angle - k->angle;
    k->angle += K[0] * y;
    k->bias  += K[1] * y;

    double P00 = k->P[0][0];
    double P01 = k->P[0][1];
    k->P[0][0] -= K[0] * P00;
    k->P[0][1] -= K[0] * P01;
    k->P[1][0] -= K[1] * P00;
    k->P[1][1] -= K[1] * P01;

    return k->angle;
}

/* ── Public API ─────────────────────────────────────────────────────── */

HC_Error_e MPU6050_Init(void)
{
    s_dev.i2c_ch   = MPU6050_I2C_CH;
    s_dev.dev_addr = MPU6050_DEV_ADDR_7BIT;

    HC_Error_e err = HC_HAL_I2C_Init(s_dev.i2c_ch);
    if (err != HC_HAL_OK) {
        return err;
    }

    HC_U8 who = 0u;
    err = mpu_read_regs(REG_WHO_AM_I, &who, 1u);
    if (err != HC_HAL_OK) {
        return err;
    }
    if (who != MPU6050_WHO_AM_I_VAL) {
        return HC_ERR_NOT_READY;
    }

    (void)mpu_write_reg(REG_PWR_MGMT_1, 0x00u);
    (void)mpu_write_reg(REG_SMPLRT_DIV, 0x07u);
    (void)mpu_write_reg(REG_ACCEL_CONFIG, 0x00u);
    (void)mpu_write_reg(REG_GYRO_CONFIG, 0x00u);

    s_tick_last_ms = mpu_tick_ms();
    return HC_HAL_OK;
}

HC_Error_e MPU6050_Read_Accel(MPU6050_T *p)
{
    if (p == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }
    HC_U8 buf[6];
    HC_Error_e err = mpu_read_regs(REG_ACCEL_XOUT_H, buf, 6u);
    if (err != HC_HAL_OK) {
        return err;
    }
    p->Accel_X_RAW = (int16_t)((buf[0] << 8) | buf[1]);
    p->Accel_Y_RAW = (int16_t)((buf[2] << 8) | buf[3]);
    p->Accel_Z_RAW = (int16_t)((buf[4] << 8) | buf[5]);
    p->Ax = p->Accel_X_RAW / ACCEL_LSB_PER_G;
    p->Ay = p->Accel_Y_RAW / ACCEL_LSB_PER_G;
    p->Az = p->Accel_Z_RAW / ACCEL_Z_CORRECTOR;
    return HC_HAL_OK;
}

HC_Error_e MPU6050_Read_Gyro(MPU6050_T *p)
{
    if (p == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }
    HC_U8 buf[6];
    HC_Error_e err = mpu_read_regs(REG_GYRO_XOUT_H, buf, 6u);
    if (err != HC_HAL_OK) {
        return err;
    }
    p->Gyro_X_RAW = (int16_t)((buf[0] << 8) | buf[1]);
    p->Gyro_Y_RAW = (int16_t)((buf[2] << 8) | buf[3]);
    p->Gyro_Z_RAW = (int16_t)((buf[4] << 8) | buf[5]);
    p->Gx = p->Gyro_X_RAW / GYRO_LSB_PER_DPS;
    p->Gy = p->Gyro_Y_RAW / GYRO_LSB_PER_DPS;
    p->Gz = p->Gyro_Z_RAW / GYRO_LSB_PER_DPS;
    return HC_HAL_OK;
}

HC_Error_e MPU6050_Read_Temp(MPU6050_T *p)
{
    if (p == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }
    HC_U8 buf[2];
    HC_Error_e err = mpu_read_regs(REG_TEMP_OUT_H, buf, 2u);
    if (err != HC_HAL_OK) {
        return err;
    }
    int16_t temp = (int16_t)((buf[0] << 8) | buf[1]);
    p->Temperature = (float)((float)temp / 340.0f + 36.53f);
    return HC_HAL_OK;
}

HC_Error_e MPU6050_Read_All(MPU6050_T *p)
{
    if (p == (void*)0) {
        return HC_HAL_ERR_NULL_PTR;
    }
    HC_U8 buf[14];
    HC_Error_e err = mpu_read_regs(REG_ACCEL_XOUT_H, buf, 14u);
    if (err != HC_HAL_OK) {
        return err;
    }

    int16_t temp = 0;
    p->Accel_X_RAW = (int16_t)((buf[0]  << 8) | buf[1]);
    p->Accel_Y_RAW = (int16_t)((buf[2]  << 8) | buf[3]);
    p->Accel_Z_RAW = (int16_t)((buf[4]  << 8) | buf[5]);
    temp           = (int16_t)((buf[6]  << 8) | buf[7]);
    p->Gyro_X_RAW  = (int16_t)((buf[8]  << 8) | buf[9]);
    p->Gyro_Y_RAW  = (int16_t)((buf[10] << 8) | buf[11]);
    p->Gyro_Z_RAW  = (int16_t)((buf[12] << 8) | buf[13]);

    p->Ax = p->Accel_X_RAW / ACCEL_LSB_PER_G;
    p->Ay = p->Accel_Y_RAW / ACCEL_LSB_PER_G;
    p->Az = p->Accel_Z_RAW / ACCEL_Z_CORRECTOR;
    p->Temperature = (float)((float)temp / 340.0f + 36.53f);
    p->Gx = p->Gyro_X_RAW / GYRO_LSB_PER_DPS;
    p->Gy = p->Gyro_Y_RAW / GYRO_LSB_PER_DPS;
    p->Gz = p->Gyro_Z_RAW / GYRO_LSB_PER_DPS;

    uint32_t now_ms = mpu_tick_ms();
    double dt = (double)(now_ms - s_tick_last_ms) / 1000.0;
    s_tick_last_ms = now_ms;

    double roll;
    double roll_sqrt = sqrt((double)p->Accel_X_RAW * p->Accel_X_RAW +
                            (double)p->Accel_Z_RAW * p->Accel_Z_RAW);
    if (roll_sqrt != 0.0) {
        roll = atan((double)p->Accel_Y_RAW / roll_sqrt) * RAD_TO_DEG;
    } else {
        roll = 0.0;
    }

    double pitch = atan2(-(double)p->Accel_X_RAW, (double)p->Accel_Z_RAW) * RAD_TO_DEG;
    if ((pitch < -90.0 && p->KalmanAngleY >  90.0) ||
        (pitch >  90.0 && p->KalmanAngleY < -90.0)) {
        s_kalman_y.angle = pitch;
        p->KalmanAngleY  = pitch;
    } else {
        p->KalmanAngleY = kalman_get_angle(&s_kalman_y, pitch, p->Gy, dt);
    }

    if (fabs(p->KalmanAngleY) > 90.0) {
        p->Gx = -p->Gx;
    }
    p->KalmanAngleX = kalman_get_angle(&s_kalman_x, roll, p->Gx, dt);

    return HC_HAL_OK;
}
