/**
 * @file    hc_err.h
 * @brief   HC_EMBED_RULES 统一错误码定义
 *
 * 所有模块返回的错误码集中定义于此，避免各层各自定义分散的错误枚举。
 * 错误码分为通用段（0x0000~0x0FFF）和各层保留段。
 */

#ifndef HC_ERR_H
#define HC_ERR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 通用错误码 ----------------------------------------------------------- */

typedef enum {
    HC_OK                    = 0x0000,  /* 成功 */

    /* 通用错误 0x0001 ~ 0x00FF */
    HC_ERR_UNKNOWN           = 0x0001,  /* 未知错误 */
    HC_ERR_INVALID_PARAM     = 0x0002,  /* 无效参数 */
    HC_ERR_NULL_PTR          = 0x0003,  /* 空指针 */
    HC_ERR_TIMEOUT           = 0x0004,  /* 超时 */
    HC_ERR_BUSY              = 0x0005,  /* 资源忙 */
    HC_ERR_NO_RESOURCE       = 0x0006,  /* 资源不足 */
    HC_ERR_NOT_INITIALIZED   = 0x0007,  /* 未初始化 */
    HC_ERR_ALREADY_EXISTS    = 0x0008,  /* 已存在 */
    HC_ERR_NOT_FOUND         = 0x0009,  /* 未找到 */
    HC_ERR_NOT_SUPPORTED     = 0x000A,  /* 不支持的操作 */
    HC_ERR_OUT_OF_RANGE      = 0x000B,  /* 数值越界 */
    HC_ERR_BUFFER_OVERFLOW   = 0x000C,  /* 缓冲区溢出 */
    HC_ERR_CHECKSUM          = 0x000D,  /* 校验和错误 */

    /* HAL 层错误 0x0100 ~ 0x01FF */
    HC_ERR_HAL_BASE          = 0x0100,
    HC_ERR_HAL_GPIO          = 0x0101,
    HC_ERR_HAL_UART          = 0x0102,
    HC_ERR_HAL_I2C           = 0x0103,
    HC_ERR_HAL_SPI           = 0x0104,
    HC_ERR_HAL_TIMER         = 0x0105,

    /* Driver 层错误 0x0200 ~ 0x02FF */
    HC_ERR_DRIVER_BASE       = 0x0200,
    HC_ERR_DRIVER_OLED       = 0x0201,
    HC_ERR_DRIVER_KEY        = 0x0202,

    /* Middleware 层错误 0x0300 ~ 0x03FF */
    HC_ERR_MIDDLEWARE_BASE   = 0x0300,

    /* Service 层错误 0x0400 ~ 0x04FF */
    HC_ERR_SERVICE_BASE      = 0x0400,

    /* App 层错误 0x0500 ~ 0x05FF */
    HC_ERR_APP_BASE          = 0x0500,

} HC_ErrCode_e;

#ifdef __cplusplus
}
#endif

#endif /* HC_ERR_H */
