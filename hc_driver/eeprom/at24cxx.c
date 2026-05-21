/*
 * @file    at24cxx.c
 * @brief   AT24C02 EEPROM 读写实现（基于 HAL）
 */

#include "at24cxx.h"
#include "hc_hal_i2c.h"
#include "hc_hal_systick.h"

static uint8_t at24cxx_check(void);

void at24cxx_init(void)
{
    uint8_t res;
    res = at24cxx_check();
    if (!res) {
        printf("AT24CXX OK!\r\n");
    }
    else {
        printf("AT24CXX ERROR!\r\n");
    }
}

/**
 * @brief  将数据写入 AT24C02。
 * @param  _usWriteAddr  起始 EEPROM 地址，范围 0~255。
 * @param  _ucpBuffer    指向待写入数据的缓冲区。
 * @param  _usNumToWrite 计划写入的字节数，0 表示不写。
 * @note   函数会自动限制写入区域不越界，并按页写入。
 */
void at24cxx_write(uint16_t _usWriteAddr, uint8_t* _ucpBuffer, uint16_t _usNumToWrite)
{
    if ((_ucpBuffer == NULL) || (_usNumToWrite == 0U) || (_usWriteAddr >= AT24_TOTAL_SIZE)) {
        return;
    }

    uint16_t writable = AT24_TOTAL_SIZE - _usWriteAddr; /* 剩余可写字节数 */
    if (_usNumToWrite > writable) {
        _usNumToWrite = writable;
    }

    AT24_I2C_WRITE(AT24C_DEV_ADDR, _usWriteAddr, _ucpBuffer, _usNumToWrite);
    AT24_DELAY_MS(5);
}

/**
 * @brief  从 AT24C02 读取数据。
 * @param  _usReadAddr  起始 EEPROM 地址，范围 0~255。
 * @param  _ucpBuffer   指向接收数据的缓冲区。
 * @param  _usNumToRead 计划读取的字节数，0 表示不读。
 * @note   函数会自动裁剪读取长度，避免越界读取。
 */
void at24cxx_read(uint16_t _usReadAddr, uint8_t* _ucpBuffer, uint16_t _usNumToRead)
{
    if ((_ucpBuffer == NULL) || (_usNumToRead == 0U) || (_usReadAddr >= AT24_TOTAL_SIZE)) {
        return;
    }

    uint16_t readable = AT24_TOTAL_SIZE - _usReadAddr; /* 剩余可读字节数 */
    if (_usNumToRead > readable) {
        _usNumToRead = readable;
    }

    AT24_I2C_READ(AT24C_DEV_ADDR, _usReadAddr, _ucpBuffer, _usNumToRead);
}

static uint8_t at24cxx_check(void)
{
    uint8_t _ucTemp;
    uint8_t _ucData = 0XAB;

    at24cxx_read(E2PROM_TYPE, &_ucTemp, 1);

    if (_ucTemp != 0XAB) {

        at24cxx_write(E2PROM_TYPE, &_ucData, 1);
        at24cxx_read(E2PROM_TYPE, &_ucTemp, 1);
        if (_ucTemp != 0XAB)
            return 1;
    }
    else {
        return 0;
    }

    return 0;
}
