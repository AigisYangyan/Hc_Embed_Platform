/*
 * @file    AT24CXX.h
 * @brief   AT24C02 EEPROM 驱动接口定义（HAL 版本）
 * @version 1.0.0
 * @date    2025-10-28
 *
 * 仅针对 AT24C02（2 Kbit）器件提供读写封装，默认使用 I2C1 (hi2c1)。
 * 所有注释均采用 UTF-8 编码，便于跨平台阅读。
 */

#ifndef AT24CXX_H__
#define AT24CXX_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdio.h"
#include "hc_hal_i2c.h"
#include "hc_hal_systick.h"

/* Exported defines ----------------------------------------------------------*/
/* AT24CXX 各容量器件的地址上限（用于兼容其他型号时参考）。 */
#define AT24C01     (127)       /* AT24C01  容量: 128字节   (1Kbit) */
#define AT24C02     (255)       /* AT24C02  容量: 256字节   (2Kbit) */
#define AT24C04     (511)       /* AT24C04  容量: 512字节   (4Kbit) */
#define AT24C08     (1023)      /* AT24C08  容量: 1024字节  (8Kbit) */
#define AT24C16     (2047)      /* AT24C16  容量: 2048字节  (16Kbit) */
#define AT24C32     (4095)      /* AT24C32  容量: 4096字节  (32Kbit) */
#define AT24C64     (8191)      /* AT24C64  容量: 8192字节  (64Kbit) */
#define AT24C128    (16383)     /* AT24C128 容量: 16384字节 (128Kbit) */
#define AT24C256    (32767)     /* AT24C256 容量: 32768字节 (256Kbit) */

/* 当前驱动仅针对 AT24C02，可按需修改为其他型号。 */
#define E2PROM_TYPE                             (AT24C02)

/* AT24C02 容量与页大小。 */
#define AT24_TOTAL_SIZE                         ((uint16_t)256)
#define AT24_PAGE_SIZE                          ((uint8_t)8)

/* AT24C02 使用 8 位内存地址 */
#define AT24_MEM_ADDR_SIZE_BITS                 (8)

/* I2C HAL API 封装宏 */
#define AT24_I2C_WRITE(DEV_ADDR, MEM_ADDR, BUFFER, BUFFER_SIZE) \
    do { \
        uint16_t remaining_bytes = (BUFFER_SIZE); \
        uint16_t current_addr = (MEM_ADDR); \
        const uint8_t *buffer_ptr = (BUFFER); \
        \
        while (remaining_bytes > 0U) { \
            uint16_t page_offset = current_addr % AT24_PAGE_SIZE; \
            uint16_t bytes_in_page = AT24_PAGE_SIZE - page_offset; \
            uint16_t write_size = (remaining_bytes < bytes_in_page) ? remaining_bytes : bytes_in_page; \
            HC_HAL_I2C_MemWrite(HC_HAL_I2C_ID_AT24C02, (DEV_ADDR), (uint8_t)current_addr, \
                                buffer_ptr, write_size); \
            remaining_bytes -= write_size; \
            current_addr += write_size; \
            buffer_ptr += write_size; \
            AT24_DELAY_MS(5); \
        } \
    } while (0)

#define AT24_I2C_READ(DEV_ADDR, MEM_ADDR, BUFFER, BUFFER_SIZE) \
        HC_HAL_I2C_MemRead(HC_HAL_I2C_ID_AT24C02, (DEV_ADDR), (MEM_ADDR), \
                           (BUFFER), (BUFFER_SIZE))

/* 延时宏（使用 HAL SysTick） */
#define AT24_DELAY_MS(x) HC_HAL_SYSTICK_DelayMs(x)

/* AT24CXX 器件 I2C 地址，A2~A0 接地时为 0xA0。 */
#define AT24C_DEV_ADDR (0XA0)

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief  ��ʼ��AT24CXX
 * @note   �ú������Զ����EEPROM�Ƿ���������
 * @retval None
 */
void at24cxx_init(void);

/**
 * @brief  ��AT24CXXָ����ַд������
 * @param  WriteAddr: д�����ݵ���ʼ��ַ (0 ~ EE_TYPE)
 * @param  pBuffer:   ָ��Ҫд�����ݵĻ�����ָ��
 * @param  NumToWrite: Ҫд����ֽ���
 * @retval None
 */
void at24cxx_write(uint16_t _usWriteAddr, uint8_t *_ucpBuffer, uint16_t _usNumToWrite);

/**
 * @brief  ��AT24CXXָ����ַ��ȡ����
 * @param  ReadAddr:  ��ȡ���ݵ���ʼ��ַ (0 ~ EE_TYPE)
 * @param  pBuffer:   ָ��洢��ȡ���ݵĻ�����ָ��
 * @param  NumToRead: Ҫ��ȡ���ֽ���
 * @retval None
 */
void at24cxx_read(uint16_t _usReadAddr, uint8_t *_ucpBuffer, uint16_t _usNumToRead);

#endif /* AT24CXX_H__ */
