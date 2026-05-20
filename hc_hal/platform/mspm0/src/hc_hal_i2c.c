/**
 * @file    hc_hal_i2c.c
 * @brief   I2C HAL 接口与实现，提供总线初始化与总线恢复能力。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_board_cfg.h"
#include "hc_hal_i2c.h"
#include "ti/driverlib/dl_i2c.h"
#include "ti/driverlib/dl_gpio.h"

/* Define generic I2C error if not present */
#ifndef HC_HAL_ERR_I2C
#define HC_HAL_ERR_I2C HC_ERR_UNKNOWN
#endif

/* Check if SysConfig generated macros exist. If not, fallback to manual definition */
#ifndef HC_HAL_I2C_OLED_INST
    #define HC_HAL_I2C_OLED_INST          I2C1
    #define HC_HAL_I2C_OLED_SDA_PORT      GPIOA
    #define HC_HAL_I2C_OLED_SDA_PIN       DL_GPIO_PIN_30
    #define HC_HAL_I2C_OLED_SDA_IOMUX     IOMUX_PINCM61
    #define HC_HAL_I2C_OLED_SDA_FUNC      IOMUX_PINCM61_PF_I2C1_SDA
    #define HC_HAL_I2C_OLED_SCL_PORT      GPIOA
    #define HC_HAL_I2C_OLED_SCL_PIN       DL_GPIO_PIN_29
    #define HC_HAL_I2C_OLED_SCL_IOMUX     IOMUX_PINCM60
    #define HC_HAL_I2C_OLED_SCL_FUNC      IOMUX_PINCM60_PF_I2C1_SCL
    /* Define a local init function if SysConfig one is missing */
    static void Manual_I2C_Init(void) {
        DL_GPIO_initPeripheralInputFunction(
            HC_HAL_I2C_OLED_SDA_PORT, HC_HAL_I2C_OLED_SDA_PIN, HC_HAL_I2C_OLED_SDA_FUNC);
        DL_GPIO_initPeripheralInputFunction(
            HC_HAL_I2C_OLED_SCL_PORT, HC_HAL_I2C_OLED_SCL_PIN, HC_HAL_I2C_OLED_SCL_FUNC);
        
        DL_I2C_reset(HC_HAL_I2C_OLED_INST);
        DL_I2C_enablePower(HC_HAL_I2C_OLED_INST);
        
        DL_I2C_ClockConfig gI2CClockConfig = {
            .clockSel = DL_I2C_CLOCK_BUSCLK,
            .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
        };
        DL_I2C_setClockConfig(HC_HAL_I2C_OLED_INST, (DL_I2C_ClockConfig *) &gI2CClockConfig);
        
        /* 400kHz approx setting for 32MHz defaults */
        DL_I2C_setTimerPeriod(HC_HAL_I2C_OLED_INST, 9);
        DL_I2C_enableController(HC_HAL_I2C_OLED_INST);
    }
#else
    /* SysConfig generated function prototype */
    extern void SYSCFG_DL_I2C_OLED_init(void);
#endif

/* ----- MPU6050 通道 (依赖 SysConfig 生成的宏与初始化函数) ----- */
#ifdef I2C_MPU6050_INST
    extern void SYSCFG_DL_I2C_MPU6050_init(void);
    #define HC_HAL_I2C_MPU6050_INST   I2C_MPU6050_INST
#endif

/* ============================================================================
 *  内部工具：逻辑通道 → I2C 寄存器基地址映射
 * ========================================================================== */
static I2C_Regs *GetI2CInst(HC_HAL_I2C_Ch_e ch)
{
    if ((ch == I2C_CH_OLED) || (ch == I2C_CH_AT24C02)) {
        return (I2C_Regs *)HC_HAL_I2C_OLED_INST;
    }
#ifdef HC_HAL_I2C_MPU6050_INST
    if (ch == I2C_CH_MPU6050) {
        return (I2C_Regs *)HC_HAL_I2C_MPU6050_INST;
    }
#endif
    return NULL;
}

/* ============================================================================
 *  公开 API 实现 (详细语义见 hc_hal_i2c.h)
 * ========================================================================== */

HC_Error_e HC_HAL_I2C_Init(HC_HAL_I2C_Ch_e ch)
{
    if ((ch == I2C_CH_OLED) || (ch == I2C_CH_AT24C02)) {
#ifdef HC_HAL_I2C_OLED_INST
        /* Use SysConfig init if available */
        SYSCFG_DL_I2C_OLED_init();
#else
        Manual_I2C_Init();
#endif
    }
#ifdef HC_HAL_I2C_MPU6050_INST
    else if (ch == I2C_CH_MPU6050) {
        SYSCFG_DL_I2C_MPU6050_init();
    }
#endif
    else {
        return HC_HAL_ERR_INVALID;
    }

    // Ensure controller is enabled
    I2C_Regs *i2c = GetI2CInst(ch);
    if (i2c && !(DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY_BUS)) {
       DL_I2C_enableController(i2c);
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_I2C_Write(HC_HAL_I2C_Ch_e ch, HC_U8 addr, const HC_U8 *p_data, HC_U16 len)
{
    I2C_Regs *i2c = GetI2CInst(ch);
    if (!i2c) return HC_HAL_ERR_INVALID;
    
    // Check if bus is busy
    int timeout = 10000;
    while (DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY) {
        if(timeout-- <= 0) return HC_HAL_ERR_I2C;
    }

    /* Send Start + Address + Write */
    int i = 0;
    int timeout_fifo;
    
    // Fill FIFO initially up to 8 bytes
    while (i < len && !DL_I2C_isControllerTXFIFOFull(i2c)) {
        DL_I2C_transmitControllerData(i2c, p_data[i++]);
    }
    
    DL_I2C_startControllerTransfer(i2c, addr, DL_I2C_CONTROLLER_DIRECTION_TX, len);
    
    // Send remaining data
    while (i < len) {
        timeout_fifo = 50000;
        /* Wait for space with timeout */
        while (DL_I2C_isControllerTXFIFOFull(i2c)) {
            if (timeout_fifo-- <= 0) return HC_HAL_ERR_TIMEOUT;
        }
        DL_I2C_transmitControllerData(i2c, p_data[i++]);
    }
    
    // Wait for idle
    timeout = 50000;
    while ((DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY)) {
        if (timeout-- <= 0) return HC_HAL_ERR_TIMEOUT;
    }
    
    if (DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_ERROR) {
        return HC_HAL_ERR_I2C;
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_I2C_BusRecover(HC_HAL_I2C_Ch_e ch)
{
    // Minimal recovery
    I2C_Regs *i2c = GetI2CInst(ch);
    if (!i2c) return HC_HAL_ERR_INVALID;
    
    DL_I2C_disableController(i2c);
    DL_I2C_enableController(i2c);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_I2C_MemWrite(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr, HC_U8 mem_addr, const HC_U8 *p_data, HC_U16 len)
{
    // Combined write: MemAddr + Data
    uint8_t buf[64];
    if (len + 1 > sizeof(buf)) return HC_HAL_ERR_INVALID;
    
    buf[0] = mem_addr;
    for(int i=0; i<len; i++) buf[i+1] = p_data[i];
    
    return HC_HAL_I2C_Write(ch, dev_addr, buf, len + 1);
}

HC_Error_e HC_HAL_I2C_MemRead(HC_HAL_I2C_Ch_e ch, HC_U8 dev_addr, HC_U8 mem_addr, HC_U8 *p_data, HC_U16 len)
{
    // Write MemAddr
    HC_Error_e res = HC_HAL_I2C_Write(ch, dev_addr, &mem_addr, 1);
    if (res != HC_HAL_OK) return res;
    
    // Read Data
    I2C_Regs *i2c = GetI2CInst(ch);
    
     DL_I2C_startControllerTransfer(i2c, dev_addr, DL_I2C_CONTROLLER_DIRECTION_RX, len);
     
     for (int i = 0; i < len; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(i2c));
        p_data[i] = DL_I2C_receiveControllerData(i2c);
    }
    
    while (DL_I2C_getControllerStatus(i2c) & DL_I2C_CONTROLLER_STATUS_BUSY);
    
    return HC_HAL_OK;
}
