/**
 * @file    hc_hal_gpio.c
 * @brief   GPIO HAL 接口与实现，负责虚拟引脚读写、中断与编码器计数。
 * @details 本文件属于 HAL 层公共代码，已补充快速上手导向注释。
 *          建议结合对应 cfg 文件与上层调用路径一起阅读。
 */
#include "hc_hal_gpio.h"
#include "ti_msp_dl_config.h"
#include <ti/driverlib/dl_gpio.h>

/* ============================================================================
 *  1. 模块私有数据
 * ========================================================================== */

/* cfg 层定义的引脚配置表，按 VPin 枚举顺序一一对应。 */
extern const HC_GPIO_PinCfg_t g_gpioPinMap[VPIN_MAX];

static HC_HAL_GPIO_IrqHandler_t s_gpio_irq_handlers[HC_HAL_GPIO_MAX_IRQ_HANDLERS];
static HC_U8 s_gpio_irq_handler_count = 0u;

HC_Error_e HC_HAL_GPIO_RegisterIrqHandler(HC_HAL_GPIO_IrqHandler_t handler)
{
    HC_U8 i;

    if (handler == HC_NULL_PTR) {
        return HC_HAL_ERR_INVALID;
    }
    for (i = 0u; i < s_gpio_irq_handler_count; i++) {
        if (s_gpio_irq_handlers[i] == handler) {
            return HC_HAL_OK;
        }
    }
    if (s_gpio_irq_handler_count >= HC_HAL_GPIO_MAX_IRQ_HANDLERS) {
        return HC_HAL_ERR_INVALID;
    }
    s_gpio_irq_handlers[s_gpio_irq_handler_count++] = handler;
    return HC_HAL_OK;
}

/** @brief 按端口聚合的中断极性临时配置，用于初始化时一次写入寄存器。 */
typedef struct {
    GPIO_Regs *port;
    HC_U32 irq_mask;
    HC_U32 lower_polarity;
    HC_U32 upper_polarity;
} HC_HAL_GPIO_PortIrqCfg_t;

/* ============================================================================
 *  2. 内部工具函数
 * ========================================================================== */

/** @brief 根据虚拟引脚 ID 返回配置项指针，越界返回 NULL。 */
static const HC_GPIO_PinCfg_t *hc_hal_gpio_get_cfg(HC_HAL_GPIO_VPin_e vpin)
{
    if ((HC_U32)vpin >= (HC_U32)VPIN_MAX) {
        return HC_NULL_PTR;
    }

    return &g_gpioPinMap[vpin];
}

/**
 * @brief 根据引脚掩码和中断模式计算 MSPM0 POLARITY 寄存器中对应的位域。
 *
 * POLARITY 寄存器为每个引脚分配 2 bit：00=禁用 / 01=上升 / 10=下降 / 11=两边。
 * 仅对单位掩码有效，输入非 2^n 返回 0。
 */
static HC_U32 hc_hal_gpio_get_irq_edge_mask(HC_U32 pin, HC_GPIO_IrqMode_e irq_mode)
{
    HC_U32 pin_index = 0u;

    if ((pin == 0u) || (irq_mode == HC_GPIO_IRQ_NONE)) {
        return 0u;
    }

    while (((pin >> pin_index) & 0x1u) == 0u) {
        pin_index++;
    }

    if ((pin_index >= 32u) || ((pin & (pin - 1u)) != 0u)) {
        return 0u;
    }

    return ((HC_U32)irq_mode) << ((pin_index & 0x0Fu) * 2u);
}

/** @brief 将单个引脚的中断规则合入所属端口的汇总配置。 */
static void hc_hal_gpio_append_irq_cfg(HC_HAL_GPIO_PortIrqCfg_t *p_port_cfg,
                                       HC_U32 pin,
                                       HC_GPIO_IrqMode_e irq_mode)
{
    HC_U32 edge_mask = hc_hal_gpio_get_irq_edge_mask(pin, irq_mode);

    if ((p_port_cfg == HC_NULL_PTR) || (edge_mask == 0u)) {
        return;
    }

    p_port_cfg->irq_mask |= pin;
    if ((pin & 0x0000FFFFu) != 0u) {
        p_port_cfg->lower_polarity |= edge_mask;
    } else {
        p_port_cfg->upper_polarity |= edge_mask;
    }
}

/** @brief 将汇总后的中断配置一次性写入硬件寄存器并使能中断。 */
static void hc_hal_gpio_apply_irq_cfg(const HC_HAL_GPIO_PortIrqCfg_t *p_port_cfg)
{
    if ((p_port_cfg == HC_NULL_PTR) || (p_port_cfg->port == HC_NULL_PTR)) {
        return;
    }

    if (p_port_cfg->lower_polarity != 0u) {
        DL_GPIO_setLowerPinsPolarity(p_port_cfg->port,
                                     p_port_cfg->lower_polarity);
    }
    if (p_port_cfg->upper_polarity != 0u) {
        DL_GPIO_setUpperPinsPolarity(p_port_cfg->port,
                                     p_port_cfg->upper_polarity);
    }
    if (p_port_cfg->irq_mask != 0u) {
        DL_GPIO_clearInterruptStatus(p_port_cfg->port, p_port_cfg->irq_mask);
        DL_GPIO_enableInterrupt(p_port_cfg->port, p_port_cfg->irq_mask);
    }
}

/**
 * @brief Dispatch interrupt to registered handlers, then legacy weak callback.
 */
static void hc_hal_gpio_handle_irq(HC_HAL_GPIO_VPin_e vpin)
{
    HC_U8 i;

    for (i = 0u; i < s_gpio_irq_handler_count; i++) {
        s_gpio_irq_handlers[i](vpin);
    }
    HC_HAL_GPIO_Callback(vpin);
}

/* ============================================================================
 *  3. 公开 API 实现 (详细语义见 hc_hal_gpio.h)
 * ========================================================================== */

HC_Error_e HC_HAL_GPIO_Init(HC_VOID)
{
    HC_U32 i;
    const HC_GPIO_PinCfg_t *cfg;
    HC_HAL_GPIO_PortIrqCfg_t gpioa_irq_cfg = { GPIOA, 0u, 0u, 0u };
    HC_HAL_GPIO_PortIrqCfg_t gpiob_irq_cfg = { GPIOB, 0u, 0u, 0u };

    for (i = 0u; i < (HC_U32)VPIN_MAX; i++) {
        cfg = &g_gpioPinMap[i];
        if ((cfg->port == HC_NULL_PTR) || (cfg->pin == 0u)) {
            continue;
        }

        if ((cfg->dir == HC_GPIO_DIR_INPUT) &&
            (cfg->irqMode != HC_GPIO_IRQ_NONE)) {
            if ((GPIO_Regs *)cfg->port == GPIOA) {
                hc_hal_gpio_append_irq_cfg(&gpioa_irq_cfg,
                                           cfg->pin,
                                           cfg->irqMode);
            } else if ((GPIO_Regs *)cfg->port == GPIOB) {
                hc_hal_gpio_append_irq_cfg(&gpiob_irq_cfg,
                                           cfg->pin,
                                           cfg->irqMode);
            } else {
                continue;
            }
        }
    }

    hc_hal_gpio_apply_irq_cfg(&gpioa_irq_cfg);
    hc_hal_gpio_apply_irq_cfg(&gpiob_irq_cfg);

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Write(HC_HAL_GPIO_VPin_e vpin, HC_Bool_e state)
{
    const HC_GPIO_PinCfg_t *cfg;
    GPIO_Regs *port;

    cfg = hc_hal_gpio_get_cfg(vpin);
    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);

    port = (GPIO_Regs *)cfg->port;
    if (state == HC_TRUE) {
        DL_GPIO_setPins(port, cfg->pin);
    } else {
        DL_GPIO_clearPins(port, cfg->pin);
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Read(HC_HAL_GPIO_VPin_e vpin, HC_Bool_e *p_state)
{
    const HC_GPIO_PinCfg_t *cfg;

    HC_HAL_ASSERT_PARAM(p_state != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);
    cfg = hc_hal_gpio_get_cfg(vpin);
    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);

    *p_state = (DL_GPIO_readPins((GPIO_Regs *)cfg->port, cfg->pin) != 0u)
                   ? HC_TRUE
                   : HC_FALSE;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Toggle(HC_HAL_GPIO_VPin_e vpin)
{
    const HC_GPIO_PinCfg_t *cfg;

    cfg = hc_hal_gpio_get_cfg(vpin);
    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);

    DL_GPIO_togglePins((GPIO_Regs *)cfg->port, cfg->pin);
    return HC_HAL_OK;
}

/* 弱符号默认空实现；业务层在自己的 .c 文件定义同名函数即可 Override。 */
HC_WEAK HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_VPin_e vpin)
{
    HC_UNUSED(vpin);
}

/* ============================================================================
 *  4. 中断服务
 * ========================================================================== */

/**
 * @brief 中断分发主逻辑。
 *
 * 1) 一次性读取 GPIOA / GPIOB 的 pending 标志
 * 2) 清除 pending，避免在遍历过程中重进中断
 * 3) 遍历 VPin 表，对每个匹配的引脚调用 `hc_hal_gpio_handle_irq`
 */
static void HC_HAL_GPIO_ProcessInterrupts(void)
{
    uint32_t pending_irq_a = 0u;
    uint32_t pending_irq_b = 0u;
    HC_U32 index;

    pending_irq_a = DL_GPIO_getEnabledInterruptStatus(GPIOA, 0xFFFFFFFFu);
    pending_irq_b = DL_GPIO_getEnabledInterruptStatus(GPIOB, 0xFFFFFFFFu);

    if ((pending_irq_a | pending_irq_b) == 0u) {
        return;
    }

    DL_GPIO_clearInterruptStatus(GPIOA, pending_irq_a);
    DL_GPIO_clearInterruptStatus(GPIOB, pending_irq_b);

    for (index = 0u; index < (HC_U32)VPIN_MAX; index++) {
        const HC_GPIO_PinCfg_t *cfg = &g_gpioPinMap[index];
        uint32_t current_port_pending = 0u;

        if ((cfg->port == HC_NULL_PTR) ||
            (cfg->pin == 0u) ||
            (cfg->irqMode == HC_GPIO_IRQ_NONE)) {
            continue;
        }

        if ((GPIO_Regs *)cfg->port == GPIOA) {
            current_port_pending = pending_irq_a;
        } else if ((GPIO_Regs *)cfg->port == GPIOB) {
            current_port_pending = pending_irq_b;
        } else {
            continue;
        }

        if ((current_port_pending & cfg->pin) != 0u) {
            hc_hal_gpio_handle_irq((HC_HAL_GPIO_VPin_e)index);
        }
    }
}

HC_IRQ_HANDLER(GROUP1_IRQHandler)
{
    /* On MSPM0G3507, GPIOA and GPIOB interrupts are mapped to GROUP1 */
    HC_HAL_GPIO_ProcessInterrupts();
}
