/**
 * @file    key.c
 * @brief   板载按键基础驱动
 *
 * 本文件实现 K1 ~ K4 的最小按键输入逻辑，作为菜单与交互层的基础输入模块。
 *
 * 功能范围：
 * - 读取按键当前电平状态
 * - 在周期扫描中锁存“未按下 -> 按下”的单次事件
 * - 按固定顺序对外输出待处理的按键事件
 *
 * 不负责的内容：
 * - 长按、连发、双击等高级交互语义
 * - 复杂时间窗口消抖
 * - 菜单状态切换与业务动作分发
 *
 * 实现说明：
 * - 复用 HAL 中已经建立好的 VPIN_K1 ~ VPIN_K4 映射
 * - GPIO 下降沿中断只负责唤醒扫描；真正事件必须由周期采样确认
 * - 一次有效按下后，必须等待稳定释放，才允许产生下一次按下事件
 * - 所有对外接口都围绕“当前状态”和“单次事件”两类需求展开
 */

#include "../hc_driver_key.h"
#include "hc_hal_gpio.h"

 /* ---- 静态配置与运行时状态 ---------------------------------------------- */

/*
 * 更严格的软件消抖：
 * 1. 按下需要连续 4 次采样为低电平才确认，5ms 扫描下约 20ms。
 * 2. 释放同样要求连续 4 次采样为高电平，避免一次按压产生两个事件。
 */
#define KEY_PRESS_DEBOUNCE_TICKS    4u
#define KEY_RELEASE_DEBOUNCE_TICKS  4u

static const HC_HAL_GPIO_VPin_e s_key_vpins[KEY_ID_COUNT] = {
    VPIN_K1,
    VPIN_K2,
    VPIN_K3,
    VPIN_K4
};

static HC_Bool_e s_key_stable_pressed[KEY_ID_COUNT];
static HC_Bool_e s_key_press_event[KEY_ID_COUNT];
static HC_U8 s_key_press_debounce_count[KEY_ID_COUNT];
static HC_U8 s_key_release_debounce_count[KEY_ID_COUNT];
static volatile HC_Bool_e s_key_irq_pending[KEY_ID_COUNT];

/* ---- 静态辅助函数 ------------------------------------------------------- */

static HC_Bool_e key_is_valid(Key_Id_e key)
{
    return ((int)key >= 0) && ((int)key < (int)KEY_ID_COUNT);
}

/**
 * @brief  读取按键原始电平对应的按下状态
 * @param  key       按键编号
 * @param  p_pressed 输出：HC_TRUE 表示按下
 * @return HC_TRUE 表示读取成功，HC_FALSE 表示参数非法或底层读取失败
 * @note   由于按键为上拉输入，因此低电平表示按下
 */
static HC_Bool_e key_read_pressed(Key_Id_e key, HC_Bool_e* p_pressed)
{
    HC_HAL_GPIO_PinState_e pin_state = HC_PIN_SET;

    if ((p_pressed == HC_NULL_PTR) || (key_is_valid(key) == HC_FALSE)) {
        return HC_FALSE;
    }

    if (HC_HAL_GPIO_ReadPin(s_key_vpins[(int)key], &pin_state) != HC_HAL_OK) {
        *p_pressed = HC_FALSE;
        return HC_FALSE;
    }

    *p_pressed = (pin_state == HC_PIN_RESET) ? HC_TRUE : HC_FALSE;
    return HC_TRUE;
}

/* ---- 公开 API ----------------------------------------------------------- */

/**
 * @brief 初始化按键模块
 * @note  清空事件标志与中断待处理标志
 */
void Key_Init(void)
{
    for (int i = 0; i < (int)KEY_ID_COUNT; ++i) {
        HC_Bool_e pressed = HC_FALSE;

        (void)key_read_pressed((Key_Id_e)i, &pressed);
        s_key_stable_pressed[i] = pressed;
        s_key_press_event[i] = HC_FALSE;
        s_key_press_debounce_count[i] = 0u;
        s_key_release_debounce_count[i] = 0u;
        s_key_irq_pending[i] = HC_FALSE;
    }
}

/**
 * @brief 周期消抖确认
 * @note  下降沿中断只作为“开始观察”的提示。
 *        真正按下事件需满足：
 *        1. 当前稳定状态为释放
 *        2. 连续 KEY_PRESS_DEBOUNCE_TICKS 次读到真实按下
 *        3. 产生事件后，必须连续 KEY_RELEASE_DEBOUNCE_TICKS 次读到释放，才重新解锁
 */
void Key_Scan(void)
{
    for (int i = 0; i < (int)KEY_ID_COUNT; ++i) {
        HC_Bool_e raw_pressed = HC_FALSE;

        /*
         * 释放态下只在收到下降沿后开始采样，保留中断唤醒的低开销特性；
         * 已确认按下后则持续观察释放，防止同一次按压重复出事件。
         */
        if ((s_key_irq_pending[i] == HC_FALSE) &&
            (s_key_stable_pressed[i] == HC_FALSE)) {
            continue;
        }

        if (key_read_pressed((Key_Id_e)i, &raw_pressed) == HC_FALSE) {
            continue;
        }

        if (s_key_stable_pressed[i] == HC_FALSE) {
            if (raw_pressed == HC_TRUE) {
                if (s_key_press_debounce_count[i] < KEY_PRESS_DEBOUNCE_TICKS) {
                    s_key_press_debounce_count[i]++;
                }

                if (s_key_press_debounce_count[i] >= KEY_PRESS_DEBOUNCE_TICKS) {
                    s_key_stable_pressed[i] = HC_TRUE;
                    s_key_press_event[i] = HC_TRUE;
                    s_key_press_debounce_count[i] = 0u;
                    s_key_release_debounce_count[i] = 0u;
                    s_key_irq_pending[i] = HC_FALSE;
                }
            }
            else {
                /* 抖动或误触发：看到高电平就撤销本轮候选按下。 */
                s_key_press_debounce_count[i] = 0u;
                s_key_irq_pending[i] = HC_FALSE;
            }
        }
        else {
            if (raw_pressed == HC_FALSE) {
                if (s_key_release_debounce_count[i] < KEY_RELEASE_DEBOUNCE_TICKS) {
                    s_key_release_debounce_count[i]++;
                }

                if (s_key_release_debounce_count[i] >= KEY_RELEASE_DEBOUNCE_TICKS) {
                    s_key_stable_pressed[i] = HC_FALSE;
                    s_key_release_debounce_count[i] = 0u;
                    s_key_press_debounce_count[i] = 0u;
                    s_key_irq_pending[i] = HC_FALSE;
                }
            }
            else {
                /* 按住期间任何额外下降沿都不应再次触发事件。 */
                s_key_release_debounce_count[i] = 0u;
                s_key_irq_pending[i] = HC_FALSE;
            }
        }
    }
}

/**
 * @brief GPIO 中断回调（覆写 HAL 弱函数）
 * @note  K1~K4 下降沿触发时，仅置 pending 标志开始观察；
 *        已处于稳定按下时忽略额外中断，避免一次按压重复触发
 */
void HC_HAL_GPIO_Callback(HC_HAL_GPIO_VPin_e vpin)
{
    int i = -1;

    if      (vpin == VPIN_K1) { i = (int)KEY_ID_K1; }
    else if (vpin == VPIN_K2) { i = (int)KEY_ID_K2; }
    else if (vpin == VPIN_K3) { i = (int)KEY_ID_K3; }
    else if (vpin == VPIN_K4) { i = (int)KEY_ID_K4; }

    if ((i >= 0) && (s_key_stable_pressed[i] == HC_FALSE)) {
        s_key_irq_pending[i] = HC_TRUE;//标记该键有下降沿，等待 Key_Scan() 读电平确认
        s_key_press_debounce_count[i] = 0u;
    }
}

/**
 * @brief 读取按键当前稳定状态
 * @param key 按键编号
 * @return HC_TRUE 表示当前按下
 */
HC_Bool_e Key_IsPressed(Key_Id_e key)
{
    if (key_is_valid(key) == HC_FALSE) {
        return HC_FALSE;
    }

    return s_key_stable_pressed[(int)key];
}

/**
 * @brief 读取并清除指定按键的单次按下事件(事件读取安全检验)
 * @param key 按键编号
 * @return HC_TRUE 表示本次取到了待处理事件
 */
HC_Bool_e Key_GetPressEvent(Key_Id_e key)
{
    if (key_is_valid(key) == HC_FALSE) {
        return HC_FALSE;
    }

    if (s_key_press_event[(int)key] == HC_FALSE) {
        return HC_FALSE;
    }

    s_key_press_event[(int)key] = HC_FALSE;
    return HC_TRUE;
}

/**
 * @brief  按 K1 -> K4 顺序取出一个待处理事件
 * @param  p_key 输出：被取出的按键编号
 * @return HC_TRUE 表示成功取到一个事件
 */
HC_Bool_e Key_PollPressEvent(Key_Id_e* p_key)
{
    if (p_key == HC_NULL_PTR) {
        return HC_FALSE;
    }

    for (int i = 0; i < (int)KEY_ID_COUNT; ++i) {
        if (s_key_press_event[i] == HC_TRUE) {
            s_key_press_event[i] = HC_FALSE;
            *p_key = (Key_Id_e)i;
            return HC_TRUE;
        }
    }

    return HC_FALSE;
}
