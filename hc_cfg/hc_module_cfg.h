#ifndef HC_MODULE_CFG_H
#define HC_MODULE_CFG_H

/*
 * hc_module_cfg.h — canonical module feature switches.
 *
 * Set to 0 to disable a module at compile time, 1 to enable.
 * Board/target overrides belong in hc_board_cfg.h.
 */

#ifndef HC_MODULE_GPIO_ENABLE
  #define HC_MODULE_GPIO_ENABLE    1
#endif
#ifndef HC_MODULE_UART_ENABLE
  #define HC_MODULE_UART_ENABLE    1
#endif
#ifndef HC_MODULE_PWM_ENABLE
  #define HC_MODULE_PWM_ENABLE     1
#endif
#ifndef HC_MODULE_DMA_ENABLE
  #define HC_MODULE_DMA_ENABLE     1
#endif
#ifndef HC_MODULE_I2C_ENABLE
  #define HC_MODULE_I2C_ENABLE     1
#endif
#ifndef HC_MODULE_SYSTICK_ENABLE
  #define HC_MODULE_SYSTICK_ENABLE 1
#endif
#ifndef HC_MODULE_TIMER_ENABLE
  #define HC_MODULE_TIMER_ENABLE   1
#endif
#ifndef HC_MODULE_WDG_ENABLE
  #define HC_MODULE_WDG_ENABLE     1
#endif
#ifndef HC_MODULE_DWT_ENABLE
  #define HC_MODULE_DWT_ENABLE     1
#endif

#endif /* HC_MODULE_CFG_H */
