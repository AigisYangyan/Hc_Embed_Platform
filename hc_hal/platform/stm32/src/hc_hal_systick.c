#include "hc_hal_systick.h"

#include "cmsis_os2.h"
#include "main.h"
#include "hc_hal_board_cfg.h"

static HC_Bool_e s_systick_is_init = HC_FALSE;

static void hc_hal_systick_enable_dwt(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

HC_S32 HC_HAL_SYSTICK_Init(HC_VOID)
{
    if (s_systick_is_init == HC_TRUE) {
        return HC_HAL_ERR_ALREADY_INIT;
    }

    hc_hal_systick_enable_dwt();
    s_systick_is_init = HC_TRUE;
    return HC_HAL_OK;
}

HC_S32 HC_HAL_SYSTICK_GetTickMs(HC_U32 *p_tick_ms)
{
    if (p_tick_ms == HC_NULL_PTR) {
        return HC_HAL_ERR_NULL_PTR;
    }

    if (s_systick_is_init == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    *p_tick_ms = HAL_GetTick();
    return HC_HAL_OK;
}

HC_S32 HC_HAL_SYSTICK_DelayMs(HC_U32 delay_ms)
{
    if (s_systick_is_init == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    if (delay_ms == 0u) {
        return HC_HAL_OK;
    }

    if (osKernelGetState() == osKernelRunning) {
        (void)osDelay(delay_ms);
    } else {
        HAL_Delay(delay_ms);
    }

    return HC_HAL_OK;
}

HC_S32 HC_HAL_SYSTICK_DelayUs(HC_U32 delay_us)
{
    HC_U32 start_cycles;
    HC_U32 wait_cycles;

    if (s_systick_is_init == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    if (delay_us == 0u) {
        return HC_HAL_OK;
    }

    start_cycles = DWT->CYCCNT;
    wait_cycles = delay_us * (HC_HAL_SYSTICK_CPU_HZ / 1000000u);
    while ((DWT->CYCCNT - start_cycles) < wait_cycles) {
    }

    return HC_HAL_OK;
}

HC_VOID HC_HAL_SYSTICK_IRQHandler(HC_VOID)
{
}
