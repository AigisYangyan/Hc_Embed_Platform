#ifndef HC_TARGET_CFG_H
#define HC_TARGET_CFG_H

/* Target MCU selection. Define exactly ONE before including this file.
 *   #define HC_TARGET_STM32F1  1
 *   #define HC_TARGET_MSPM0   1
 *
 * If neither is defined, the build will error out.
 */

#if !defined(HC_TARGET_STM32F1) && !defined(HC_TARGET_MSPM0)
#error "HC_TARGET_STM32F1 or HC_TARGET_MSPM0 must be defined"
#endif

#if defined(HC_TARGET_STM32F1) && defined(HC_TARGET_MSPM0)
#error "Only one target may be selected at a time"
#endif

#endif /* HC_TARGET_CFG_H */
