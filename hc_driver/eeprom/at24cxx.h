/*
 * @file    AT24CXX.h
 * @brief   AT24CXX EEPROM 驱动公开接口
 * @version 1.0.0
 * @date    2025-10-28
 *
 * 设备级 EEPROM 驱动接口，不暴露 HAL 或芯片私有细节。
 */

#ifndef AT24CXX_H__
#define AT24CXX_H__

/* Includes ------------------------------------------------------------------*/
#include "hc_common/hc_types.h"
#include "hc_common/hc_err.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化 EEPROM，执行无副作用连通性探测。
 * @note   不向 EEPROM 写入任何数据，仅通过 I2C 读操作检测器件是否应答。
 * @retval HC_HAL_OK         探测成功，器件正常应答。
 * @retval HC_HAL_ERR_TIMEOUT I2C 无应答，器件不存在或总线故障。
 * @retval HC_HAL_ERR_I2C     I2C 总线错误。
 */
HC_Error_e at24cxx_init(HC_VOID);

/**
 * @brief  向 EEPROM 指定地址写入数据。
 * @param  addr     起始地址，范围 0 ~ (总容量 - 1)。
 * @param  p_buffer 指向待写入数据的缓冲区，不可为 NULL。
 * @param  len      要写入的字节数，不可为 0。
 * @note   超出尾部容量时自动裁剪到剩余空间。
 * @retval HC_HAL_OK           写入成功。
 * @retval HC_HAL_ERR_NULL_PTR p_buffer 为 NULL。
 * @retval HC_HAL_ERR_INVALID  len 为 0 或 addr 越界。
 * @retval HC_HAL_ERR_TIMEOUT  I2C 通信超时。
 * @retval HC_HAL_ERR_I2C      I2C 总线错误。
 */
HC_Error_e at24cxx_write(HC_U16 addr, const HC_U8 *p_buffer, HC_U16 len);

/**
 * @brief  从 EEPROM 指定地址读取数据。
 * @param  addr     起始地址，范围 0 ~ (总容量 - 1)。
 * @param  p_buffer 指向接收数据的缓冲区，不可为 NULL。
 * @param  len      要读取的字节数，不可为 0。
 * @note   超出尾部容量时自动裁剪到剩余空间。
 * @retval HC_HAL_OK           读取成功。
 * @retval HC_HAL_ERR_NULL_PTR p_buffer 为 NULL。
 * @retval HC_HAL_ERR_INVALID  len 为 0 或 addr 越界。
 * @retval HC_HAL_ERR_TIMEOUT  I2C 通信超时。
 * @retval HC_HAL_ERR_I2C      I2C 总线错误。
 */
HC_Error_e at24cxx_read(HC_U16 addr, HC_U8 *p_buffer, HC_U16 len);

#endif /* AT24CXX_H__ */
