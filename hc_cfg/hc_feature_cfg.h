/**
 * @file    hc_feature_cfg.h
 * @brief   HC_EMBED_RULES 功能模块开关与裁剪
 *
 * 通过宏开关控制各功能模块的编译裁剪。关闭的模块不参与编译，
 * 减小固件体积。
 */

#ifndef HC_FEATURE_CFG_H
#define HC_FEATURE_CFG_H

/* ---- 调度后端 ------------------------------------------------------------- */

#define HC_BACKEND_BAREMETAL   0
#define HC_BACKEND_FREERTOS    1

#ifndef HC_BACKEND_MODE
#define HC_BACKEND_MODE        HC_BACKEND_BAREMETAL
#endif

/* ---- 功能模块开关 (1 = 启用, 0 = 裁剪) ----------------------------------- */

#ifndef HC_MODULE_OLED_ENABLE
#define HC_MODULE_OLED_ENABLE          1
#endif

#ifndef HC_MODULE_KEY_ENABLE
#define HC_MODULE_KEY_ENABLE           1
#endif

#ifndef HC_MODULE_VOFA_ENABLE
#define HC_MODULE_VOFA_ENABLE          1
#endif

#ifndef HC_MODULE_MENU_ENABLE
#define HC_MODULE_MENU_ENABLE          1
#endif

#ifndef HC_MODULE_APP_LIFECYCLE_ENABLE
#define HC_MODULE_APP_LIFECYCLE_ENABLE 1
#endif

#endif /* HC_FEATURE_CFG_H */
