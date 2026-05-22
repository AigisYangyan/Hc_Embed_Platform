/*
 * @file    at24cxx.c
 * @brief   AT24C02 EEPROM 读写实现（基于 HC_HAL）
 */

#include "at24cxx.h"
#include "hc_hal/hc_hal_i2c.h"
#include "hc_hal/hc_hal_systick.h"

/* Private constants ---------------------------------------------------------*/
#define AT24C_DEV_ADDR        ((HC_U8)0xA0)   /* I2C 器件地址，A2~A0 接地 */
#define AT24C_PAGE_SIZE       ((HC_U8)8)       /* AT24C02 页大小 8 字节 */
#define AT24C_TOTAL_SIZE      ((HC_U16)256)    /* AT24C02 总容量 256 字节 */
#define AT24C_I2C_CH          HC_HAL_I2C_CH_0  /* 使用的 I2C 通道 */
#define AT24C_WRITE_CYCLE_MS  ((HC_U32)5)      /* 写周期延时 ms */

/* Private helpers -----------------------------------------------------------*/

/**
 * @brief  按页边界拆包写入。
 * @return HC_HAL_OK 表示全部写入成功，否则返回首次失败的错误码。
 */
static HC_Error_e at24cxx_page_write(HC_U16 addr, const HC_U8 *p_data, HC_U16 len)
{
    HC_U16 remaining = len;
    HC_U16 current_addr = addr;
    const HC_U8 *buf_ptr = p_data;

    while (remaining > 0U) {
        HC_U16 page_offset = current_addr % AT24C_PAGE_SIZE;
        HC_U16 bytes_in_page = AT24C_PAGE_SIZE - page_offset;
        HC_U16 write_size = (remaining < bytes_in_page) ? remaining : bytes_in_page;
        HC_Error_e ret;

        ret = HC_HAL_I2C_MemWrite(AT24C_I2C_CH, AT24C_DEV_ADDR,
                                  (HC_U8)current_addr, buf_ptr, write_size);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        HC_HAL_SYSTICK_DelayMs(AT24C_WRITE_CYCLE_MS);

        remaining -= write_size;
        current_addr += write_size;
        buf_ptr += write_size;
    }

    return HC_HAL_OK;
}

/**
 * @brief  无副作用连通性探测：读一个字节检测器件是否应答。
 * @return HC_HAL_OK 表示器件存在且总线正常。
 */
static HC_Error_e at24cxx_probe(HC_VOID)
{
    HC_U8 dummy;
    return HC_HAL_I2C_MemRead(AT24C_I2C_CH, AT24C_DEV_ADDR, 0x00, &dummy, 1);
}

/* Public API ----------------------------------------------------------------*/

HC_Error_e at24cxx_init(HC_VOID)
{
    return at24cxx_probe();
}

HC_Error_e at24cxx_write(HC_U16 addr, const HC_U8 *p_buffer, HC_U16 len)
{
    if (p_buffer == NULL) {
        return HC_HAL_ERR_NULL_PTR;
    }
    if ((len == 0U) || (addr >= AT24C_TOTAL_SIZE)) {
        return HC_HAL_ERR_INVALID;
    }

    HC_U16 writable = AT24C_TOTAL_SIZE - addr;
    if (len > writable) {
        len = writable;
    }

    return at24cxx_page_write(addr, p_buffer, len);
}

HC_Error_e at24cxx_read(HC_U16 addr, HC_U8 *p_buffer, HC_U16 len)
{
    if (p_buffer == NULL) {
        return HC_HAL_ERR_NULL_PTR;
    }
    if ((len == 0U) || (addr >= AT24C_TOTAL_SIZE)) {
        return HC_HAL_ERR_INVALID;
    }

    HC_U16 readable = AT24C_TOTAL_SIZE - addr;
    if (len > readable) {
        len = readable;
    }

    return HC_HAL_I2C_MemRead(AT24C_I2C_CH, AT24C_DEV_ADDR,
                              (HC_U8)addr, p_buffer, len);
}
