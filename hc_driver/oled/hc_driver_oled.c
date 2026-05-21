/**
 * @file    oled_hardware_i2c.c
 * @brief   OLED 硬件 I2C 驱动模块实现
 *
 * 本文件实现 128x64 OLED 的基础显示驱动，作为显示执行层（I2C + SSD1306 页寻址）。
 *
 * 功能范围：
 * - 发送 OLED 命令与显示数据
 * - 设置页地址、清屏、开关显示、旋转/反色控制
 * - 基于现有字库显示数字、英文、中文与位图
 *
 * 不负责的内容：
 * - 菜单逻辑、页面刷新策略与业务排版
 * - 显存缓存、脏矩形刷新等高级显示优化
 * - 字库生成与中文字模维护
 *
 * 实现说明：
 * 1. 使用“全局实例 + Init 填充硬件绑定”方式管理 OLED 的 I2C 资源
 * 2. 所有对外接口最终都落到底层 HAL 的 I2C 写与总线恢复调用
 * 3. 通信异常时先执行一次 I2C 总线恢复，再重试当前发送
 *
 * 硬件绑定项：
 * - OLED 所在 I2C 通道
 * - OLED 设备地址
 *
 * 若移植到其他 MCU，只需修改底层 I2C HAL 映射或本模块初始化绑定即可。
 */

#include "../hc_driver_oled.h"
#include "oled_hardware_i2c.h"
#include "hc_hal_systick.h"
#include "oledfont.h"

/* ---- 静态配置与全局实例 ------------------------------------------------- */

#define OLED_I2C_ADDR   0x3Cu
#define OLED_WIDTH      128u
#define OLED_PAGE_COUNT 8u
#define OLED_POWER_STABLE_DELAY_MS 200u

typedef enum {
    OLED_INIT_STATE_IDLE = 0,
    OLED_INIT_STATE_WAIT_POWER_STABLE,
    OLED_INIT_STATE_SEND_SEQUENCE,
    OLED_INIT_STATE_READY
} OLED_InitState_e;

static const OLED_Bus_T s_tOledDefaultBus = {
    HC_HAL_I2C_ID_OLED,
    OLED_I2C_ADDR
};

static const uint8_t s_oled_init_cmds[] = {
    0xAEu, 0x00u, 0x10u, 0x40u, 0x81u, 0xCFu, 0xA1u, 0xC8u,
    0xA6u, 0xA8u, 0x3Fu, 0xD3u, 0x00u, 0xD5u, 0x80u, 0xD9u,
    0xF1u, 0xDAu, 0x12u, 0xDBu, 0x40u, 0x20u, 0x02u, 0x8Du,
    0x14u, 0xA4u, 0xA6u
};

OLED_T g_tOLED = {
    { HC_HAL_I2C_ID_OLED, OLED_I2C_ADDR }
};

static OLED_InitState_e s_oled_init_state = OLED_INIT_STATE_IDLE;
static HC_U32 s_oled_init_start_tick_ms = 0u;

/* ---- 静态辅助函数 ------------------------------------------------------- */

static HC_Bool_e oled_is_ready_state(void)
{
    return (s_oled_init_state == OLED_INIT_STATE_READY) ? HC_TRUE : HC_FALSE;
}

static HC_Error_e oled_bus_recover(const OLED_T *p_oled)
{
    return HC_HAL_I2C_BusRecover(p_oled->bus.i2c_ch);
}

static HC_Error_e oled_write_packet(const OLED_T *p_oled, uint8_t dat,
                                    uint8_t mode)
{
    HC_U8 packet[2];
    HC_Error_e ret;

    packet[0] = (mode == OLED_DATA) ? 0x40u : 0x00u;
    packet[1] = dat;

    ret = HC_HAL_I2C_MasterWrite(p_oled->bus.i2c_ch, p_oled->bus.dev_addr,
                                 packet, (HC_U16)sizeof(packet));
    if (ret != HC_HAL_OK) {
        (void)oled_bus_recover(p_oled);
        ret = HC_HAL_I2C_MasterWrite(p_oled->bus.i2c_ch, p_oled->bus.dev_addr,
                                     packet, (HC_U16)sizeof(packet));
    }

    return ret;
}

static HC_Bool_e oled_delay_elapsed(HC_U32 start_tick_ms, HC_U32 delay_ms,
                                    HC_U32 current_tick_ms)
{
    return ((current_tick_ms - start_tick_ms) >= delay_ms) ? HC_TRUE : HC_FALSE;
}

static HC_Error_e oled_send_init_sequence(void)
{
    uint16_t i;
    HC_Error_e ret;

    for (i = 0u; i < (uint16_t)sizeof(s_oled_init_cmds); i++) {
        ret = OLED_WR_Byte(s_oled_init_cmds[i], OLED_CMD);
        if (ret != HC_HAL_OK) {
            return ret;
        }
    }

    ret = OLED_Clear();
    if (ret != HC_HAL_OK) {
        return ret;
    }

    return OLED_WR_Byte(0xAFu, OLED_CMD);
}

/* ---- 公开 API ----------------------------------------------------------- */

void oled_i2c_sda_unlock(void)
{
    (void)oled_bus_recover(&g_tOLED);
}

void OLED_ColorTurn(uint8_t enable_inverse)
{
    if (enable_inverse == 0u) {
        (void)OLED_WR_Byte(0xA6u, OLED_CMD);
    }
    else {
        (void)OLED_WR_Byte(0xA7u, OLED_CMD);
    }
}

void OLED_DisplayTurn(uint8_t enable_rotate)
{
    if (enable_rotate == 0u) {
        (void)OLED_WR_Byte(0xC8u, OLED_CMD);
        (void)OLED_WR_Byte(0xA1u, OLED_CMD);
    }
    else {
        (void)OLED_WR_Byte(0xC0u, OLED_CMD);
        (void)OLED_WR_Byte(0xA0u, OLED_CMD);
    }
}

HC_Error_e OLED_WR_Byte(uint8_t dat, uint8_t mode)
{
    return oled_write_packet(&g_tOLED, dat, mode);
}

void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    (void)OLED_WR_Byte((uint8_t)(0xB0u + y), OLED_CMD);
    (void)OLED_WR_Byte((uint8_t)(((x & 0xF0u) >> 4) | 0x10u), OLED_CMD);
    (void)OLED_WR_Byte((uint8_t)(x & 0x0Fu), OLED_CMD);
}

void OLED_Display_On(void)
{
    (void)OLED_WR_Byte(0x8Du, OLED_CMD);
    (void)OLED_WR_Byte(0x14u, OLED_CMD);
    (void)OLED_WR_Byte(0xAFu, OLED_CMD);
}

void OLED_Display_Off(void)
{
    (void)OLED_WR_Byte(0x8Du, OLED_CMD);
    (void)OLED_WR_Byte(0x10u, OLED_CMD);
    (void)OLED_WR_Byte(0xAEu, OLED_CMD);
}

HC_Error_e OLED_Clear(void)
{
    uint8_t page;
    uint8_t column;
    HC_Error_e ret;

    for (page = 0u; page < OLED_PAGE_COUNT; page++) {
        ret = OLED_WR_Byte((uint8_t)(0xB0u + page), OLED_CMD);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        ret = OLED_WR_Byte(0x00u, OLED_CMD);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        ret = OLED_WR_Byte(0x10u, OLED_CMD);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        for (column = 0u; column < OLED_WIDTH; column++) {
            ret = OLED_WR_Byte(0x00u, OLED_DATA);
            if (ret != HC_HAL_OK) {
                return ret;
            }
        }
    }

    return HC_HAL_OK;
}

HC_Error_e OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t sizey)
{
    uint8_t c = (uint8_t)(chr - ' ');
    uint8_t sizex = (uint8_t)(sizey / 2u);
    uint16_t i;
    uint16_t size1;
    HC_Error_e ret;

    if (sizey == 8u) {
        size1 = 6u;
    }
    else {
        size1 = (uint16_t)((sizey / 8u + ((sizey % 8u) ? 1u : 0u)) *
                           (sizey / 2u));
    }

    OLED_Set_Pos(x, y);
    for (i = 0u; i < size1; i++) {
        if (((i % sizex) == 0u) && (sizey != 8u)) {
            OLED_Set_Pos(x, y++);
        }

        if (sizey == 8u) {
            ret = OLED_WR_Byte(asc2_0806[c][i], OLED_DATA);
        }
        else if (sizey == 16u) {
            ret = OLED_WR_Byte(asc2_1608[c][i], OLED_DATA);
        }
        else {
            return HC_HAL_ERR_INVALID;
        }

        if (ret != HC_HAL_OK) {
            return ret;
        }
    }

    return HC_HAL_OK;
}

uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1u;

    while (n-- != 0u) {
        result *= m;
    }

    return result;
}

HC_Error_e OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len,
                        uint8_t sizey)
{
    uint8_t t;
    uint8_t temp;
    uint8_t m = 0u;
    uint8_t enshow = 0u;
    HC_Error_e ret;

    if (sizey == 8u) {
        m = 2u;
    }

    for (t = 0u; t < len; t++) {
        temp = (uint8_t)((num / oled_pow(10u, (uint8_t)(len - t - 1u))) % 10u);
        if ((enshow == 0u) && (t < (len - 1u))) {
            if (temp == 0u) {
                ret = OLED_ShowChar((uint8_t)(x + (sizey / 2u + m) * t), y,
                                    ' ', sizey);
                if (ret != HC_HAL_OK) {
                    return ret;
                }
                continue;
            }
            enshow = 1u;
        }

        ret = OLED_ShowChar((uint8_t)(x + (sizey / 2u + m) * t), y,
                            (uint8_t)(temp + '0'), sizey);
        if (ret != HC_HAL_OK) {
            return ret;
        }
    }

    return HC_HAL_OK;
}

HC_Error_e OLED_ShowString(uint8_t x, uint8_t y, const char *chr,
                           uint8_t sizey)
{
    uint8_t j = 0u;
    HC_Error_e ret;

    if (chr == HC_NULL_PTR) {
        return HC_HAL_ERR_NULL_PTR;
    }

    while (chr[j] != '\0') {
        ret = OLED_ShowChar(x, y, (uint8_t)chr[j++], sizey);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        if (sizey == 8u) {
            x = (uint8_t)(x + 6u);
        }
        else {
            x = (uint8_t)(x + sizey / 2u);
        }
    }

    return HC_HAL_OK;
}

HC_Error_e OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t no, uint8_t sizey)
{
    uint16_t i;
    HC_Error_e ret;
    const uint16_t size1 =
        (uint16_t)((sizey / 8u + ((sizey % 8u) ? 1u : 0u)) * sizey);

    for (i = 0u; i < size1; i++) {
        if ((i % sizey) == 0u) {
            OLED_Set_Pos(x, y++);
        }

        if (sizey == 16u) {
            ret = OLED_WR_Byte(Hzk[no][i], OLED_DATA);
        }
        else {
            return HC_HAL_ERR_INVALID;
        }

        if (ret != HC_HAL_OK) {
            return ret;
        }
    }

    return HC_HAL_OK;
}

void OLED_DrawBMP(uint8_t x, uint8_t y, uint8_t sizex, uint8_t sizey,
                  const uint8_t BMP[])
{
    uint16_t j = 0u;
    uint8_t page;
    uint8_t column;

    sizey = (uint8_t)(sizey / 8u + ((sizey % 8u) ? 1u : 0u));
    for (page = 0u; page < sizey; page++) {
        OLED_Set_Pos(x, (uint8_t)(page + y));
        for (column = 0u; column < sizex; column++) {
            (void)OLED_WR_Byte(BMP[j++], OLED_DATA);
        }
    }
}

void OLED_Init(void)
{
    HC_U32 tick_ms = 0u;

    g_tOLED.bus = s_tOledDefaultBus;
    oled_i2c_sda_unlock();

    if (HC_HAL_SYSTICK_GetTickMs(&tick_ms) == HC_HAL_OK) {
        s_oled_init_start_tick_ms = tick_ms;
        s_oled_init_state = OLED_INIT_STATE_WAIT_POWER_STABLE;
    }
    else {
        s_oled_init_start_tick_ms = 0u;
        s_oled_init_state = OLED_INIT_STATE_SEND_SEQUENCE;
    }
}

HC_Error_e OLED_Process(void)
{
    HC_U32 tick_ms = 0u;
    HC_Error_e ret;

    if (s_oled_init_state == OLED_INIT_STATE_IDLE) {
        return HC_HAL_OK;
    }

    if (s_oled_init_state == OLED_INIT_STATE_WAIT_POWER_STABLE) {
        ret = HC_HAL_SYSTICK_GetTickMs(&tick_ms);
        if (ret != HC_HAL_OK) {
            return ret;
        }

        if (oled_delay_elapsed(s_oled_init_start_tick_ms,
                               OLED_POWER_STABLE_DELAY_MS,
                               tick_ms) == HC_FALSE) {
            return HC_HAL_OK;
        }

        s_oled_init_state = OLED_INIT_STATE_SEND_SEQUENCE;
    }

    if (s_oled_init_state == OLED_INIT_STATE_SEND_SEQUENCE) {
        ret = oled_send_init_sequence();
        if (ret != HC_HAL_OK) {
            return ret;
        }

        s_oled_init_state = OLED_INIT_STATE_READY;
    }

    return HC_HAL_OK;
}

HC_Bool_e OLED_IsReady(void)
{
    return oled_is_ready_state();
}
