#include "hc_hal_gpio.h"

#include "main.h"

extern const HC_GPIO_PinCfg_t g_gpioPinMap[VPIN_MAX];

/* 平台后端负责把项目自定义抽象枚举翻译成 STM32 HAL 常量。 */

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

HC_LOCAL HC_U32 hc_hal_gpio_pull_to_stm32(HC_GPIO_Pull_e pull)
{
    switch (pull) {
    case HC_GPIO_PULL_UP:
        return GPIO_PULLUP;
    case HC_GPIO_PULL_DOWN:
        return GPIO_PULLDOWN;
    case HC_GPIO_PULL_NONE:
    default:
        return GPIO_NOPULL;
    }
}

HC_LOCAL HC_U32 hc_hal_gpio_irq_mode_to_stm32(HC_GPIO_IrqMode_e irq_mode)
{
    switch (irq_mode) {
    case HC_GPIO_IRQ_RISING:
        return GPIO_MODE_IT_RISING;
    case HC_GPIO_IRQ_FALLING:
        return GPIO_MODE_IT_FALLING;
    case HC_GPIO_IRQ_BOTH:
        return GPIO_MODE_IT_RISING_FALLING;
    case HC_GPIO_IRQ_NONE:
    default:
        return GPIO_MODE_INPUT;
    }
}

HC_LOCAL HC_Bool_e hc_hal_gpio_get_exti_irqn(HC_U16 pin, IRQn_Type *p_irqn)
{
    HC_HAL_ASSERT_PARAM(p_irqn != HC_NULL_PTR, HC_FALSE);

    switch (pin) {
    case GPIO_PIN_0:
        *p_irqn = EXTI0_IRQn;
        return HC_TRUE;
    case GPIO_PIN_1:
        *p_irqn = EXTI1_IRQn;
        return HC_TRUE;
    case GPIO_PIN_2:
        *p_irqn = EXTI2_IRQn;
        return HC_TRUE;
    case GPIO_PIN_3:
        *p_irqn = EXTI3_IRQn;
        return HC_TRUE;
    case GPIO_PIN_4:
        *p_irqn = EXTI4_IRQn;
        return HC_TRUE;
    case GPIO_PIN_5:
    case GPIO_PIN_6:
    case GPIO_PIN_7:
    case GPIO_PIN_8:
    case GPIO_PIN_9:
        *p_irqn = EXTI9_5_IRQn;
        return HC_TRUE;
    case GPIO_PIN_10:
    case GPIO_PIN_11:
    case GPIO_PIN_12:
    case GPIO_PIN_13:
    case GPIO_PIN_14:
    case GPIO_PIN_15:
        *p_irqn = EXTI15_10_IRQn;
        return HC_TRUE;
    default:
        return HC_FALSE;
    }
}

HC_LOCAL void hc_hal_gpio_enable_exti_irq(HC_U16 pin)
{
    IRQn_Type irqn;

    if (hc_hal_gpio_get_exti_irqn(pin, &irqn) == HC_TRUE) {
        /* GPIO IRQ 只做轻量分发，优先级保持低于关键控制中断。 */
        HAL_NVIC_SetPriority(irqn, 5u, 0u);
        HAL_NVIC_EnableIRQ(irqn);
    }
}

HC_LOCAL HC_HAL_GPIO_VPin_e hc_hal_gpio_find_irq_vpin(HC_U16 gpio_pin)
{
    /* 通过 cfg 反查虚拟引脚，避免把业务回调和 STM32 Pin 常量绑死。 */
    for (HC_U32 i = 0u; i < (HC_U32)VPIN_MAX; i++) {
        const HC_GPIO_PinCfg_t *cfg = &g_gpioPinMap[i];

        if ((cfg->port == HC_NULL_PTR) || (cfg->pin == 0u)) {
            continue;
        }

        if ((cfg->irqMode != HC_GPIO_IRQ_NONE) && ((HC_U16)cfg->pin == gpio_pin)) {
            return (HC_HAL_GPIO_VPin_e)i;
        }
    }

    return VPIN_MAX;
}

HC_Error_e HC_HAL_GPIO_Init(HC_VOID)
{
    HC_U32 i;

    /* EXTI 复用器属于 AFIO，按键边沿模式需要显式打开该时钟。 */
    __HAL_RCC_AFIO_CLK_ENABLE();

    for (i = 0u; i < (HC_U32)VPIN_MAX; i++) {
        const HC_GPIO_PinCfg_t *cfg = &g_gpioPinMap[i];
        GPIO_TypeDef *port = (GPIO_TypeDef *)cfg->port;

        if ((port == (GPIO_TypeDef *)0) || (cfg->pin == 0u)) {
            continue;
        }

        if (cfg->dir == HC_GPIO_DIR_OUTPUT) {
            HAL_GPIO_WritePin(port,
                              (uint16_t)cfg->pin,
                              (cfg->initState == HC_TRUE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        }

        if ((cfg->dir == HC_GPIO_DIR_INPUT) && (cfg->irqMode != HC_GPIO_IRQ_NONE)) {
            GPIO_InitTypeDef gpio_init = {0};

            /*
             * 这里不重做所有 GPIO 初始化，只补“需要边沿中断的输入脚”。
             * 这样保持 CubeMX 生成层继续负责大部分引脚基础初始化。
             */
            gpio_init.Pin = (uint16_t)cfg->pin;
            gpio_init.Mode = hc_hal_gpio_irq_mode_to_stm32(cfg->irqMode);
            gpio_init.Pull = hc_hal_gpio_pull_to_stm32(cfg->pull);
            HAL_GPIO_Init(port, &gpio_init);
            hc_hal_gpio_enable_exti_irq((HC_U16)cfg->pin);
        }
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Write(HC_HAL_GPIO_VPin_e vpin, HC_Bool_e state)
{
    const HC_GPIO_PinCfg_t *cfg = (HC_U32)vpin < (HC_U32)VPIN_MAX ? &g_gpioPinMap[vpin] : HC_NULL_PTR;
    GPIO_TypeDef *port;

    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);
    port = (GPIO_TypeDef *)cfg->port;
    HAL_GPIO_WritePin(port,
                      (uint16_t)cfg->pin,
                      (state == HC_TRUE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Read(HC_HAL_GPIO_VPin_e vpin, HC_Bool_e *p_state)
{
    const HC_GPIO_PinCfg_t *cfg = (HC_U32)vpin < (HC_U32)VPIN_MAX ? &g_gpioPinMap[vpin] : HC_NULL_PTR;
    GPIO_TypeDef *port;

    HC_HAL_ASSERT_PARAM(p_state != HC_NULL_PTR, HC_HAL_ERR_NULL_PTR);
    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);

    port = (GPIO_TypeDef *)cfg->port;
    *p_state = (HAL_GPIO_ReadPin(port, (uint16_t)cfg->pin) == GPIO_PIN_SET) ? HC_TRUE : HC_FALSE;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_GPIO_Toggle(HC_HAL_GPIO_VPin_e vpin)
{
    const HC_GPIO_PinCfg_t *cfg = (HC_U32)vpin < (HC_U32)VPIN_MAX ? &g_gpioPinMap[vpin] : HC_NULL_PTR;

    HC_HAL_ASSERT_PARAM((cfg != HC_NULL_PTR) && (cfg->port != HC_NULL_PTR), HC_ERR_NOT_ENABLE);
    HAL_GPIO_TogglePin((GPIO_TypeDef *)cfg->port, (uint16_t)cfg->pin);
    return HC_HAL_OK;
}

HC_WEAK HC_VOID HC_HAL_GPIO_Callback(HC_HAL_GPIO_VPin_e vpin)
{
    HC_UNUSED(vpin);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* STM32 的物理引脚中断先翻译成项目内的虚拟引脚，再交给上层。 */
    HC_HAL_GPIO_VPin_e vpin = hc_hal_gpio_find_irq_vpin((HC_U16)GPIO_Pin);
    HC_U8 i;

    if (vpin < VPIN_MAX) {
        for (i = 0u; i < s_gpio_irq_handler_count; i++) {
            s_gpio_irq_handlers[i](vpin);
        }
        HC_HAL_GPIO_Callback(vpin);
    }
}
