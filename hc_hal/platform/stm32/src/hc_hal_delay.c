#include "hc_hal_delay.h"

#include "cmsis_os2.h"
#include "main.h"

extern const HC_Delay_Cfg_t g_delayCfg;

static HC_Bool_e s_delay_is_init = HC_FALSE;
static HC_U32 s_cycles_per_us = 0u;

static void hc_hal_delay_enable_dwt(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

HC_Error_e HC_HAL_Delay_Init(HC_VOID)
{
    if (s_delay_is_init == HC_TRUE) {
        return HC_HAL_ERR_ALREADY_INIT;
    }

    s_cycles_per_us = g_delayCfg.cpuFreqHz / 1000000u;
    if (s_cycles_per_us == 0u) {
        s_cycles_per_us = 1u;
    }

    hc_hal_delay_enable_dwt();
    s_delay_is_init = HC_TRUE;
    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Delay_Ms(HC_U32 ms)
{
    if (s_delay_is_init == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    if (ms == 0u) {
        return HC_HAL_OK;
    }

    if (osKernelGetState() == osKernelRunning) {
        (void)osDelay(ms);
    } else {
        HAL_Delay(ms);
    }

    return HC_HAL_OK;
}

HC_Error_e HC_HAL_Delay_Us(HC_U32 us)
{
    HC_U32 start_cycles;
    HC_U32 wait_cycles;

    if (s_delay_is_init == HC_FALSE) {
        return HC_HAL_ERR_NOT_INIT;
    }

    if (us == 0u) {
        return HC_HAL_OK;
    }

    start_cycles = DWT->CYCCNT;
    wait_cycles = us * s_cycles_per_us;
    while ((DWT->CYCCNT - start_cycles) < wait_cycles) {
    }

    return HC_HAL_OK;
}
