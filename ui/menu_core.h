/**
 * @file    menu_core.h
 * @brief   轻量注册式 OLED 菜单核心对外接口
 *
 * 本模块定义菜单核心的数据结构与公开接口。
 *
 * 功能范围：
 * - 定义页面 ID、菜单项类型与菜单状态
 * - 声明菜单初始化、按键处理与渲染接口
 * - 为静态注册页面表提供统一的数据描述格式
 *
 * 设计约定：
 * - 菜单核心负责显示、导航与少量页面级状态切换链路
 * - 页面数据全部采用编译期静态注册，不使用动态内存
 * - OLED 重渲染采用脏标志驱动，仅在 dirty == HC_TRUE 时清屏 + 重绘
 */

#ifndef UI_MENU_CORE_H
#define UI_MENU_CORE_H

#include <stdint.h>
#include "hc_common/hc_types.h"
#include "hc_driver/hc_driver_key.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* ---- 类型定义 ----------------------------------------------------------- */

#define MENU_BIND_NONE 0xFFFFu

    typedef enum {
        PAGE_HOME = 0,      /* 一级主菜单 */
        PAGE_DEBUG_MENU,    /* DEBUG 二级菜单 */
        PAGE_RUNNING,       /* 统一运行页 */
        PAGE_MAX            /* 页面总数上限 */
    } MenuPageId_e;

    typedef enum {
        MENU_ITEM_SUBPAGE = 0,  /* 子页面跳转 */
        MENU_ITEM_ACTION,       /* 执行动作函数 */
        MENU_ITEM_BACK          /* 返回上一页 */
    } MenuItemType_e;

    typedef enum {
        MENU_KEY_NONE = 0,      /* 无按键 */
        MENU_KEY_UP,            /* 向上 */
        MENU_KEY_DOWN,          /* 向下 */
        MENU_KEY_ENTER,         /* 确认 */
        MENU_KEY_BACK           /* 返回 */
    } MenuKeyAction_e;

    typedef HC_Bool_e (*MenuActionFn)(uint16_t bind_value);

    struct MenuPage;    /* 前向声明 */
    struct MenuState;   /* 前向声明 */

    /*
     * ========================================================================
     * 设计意图：策略模式
     *   将不同页面的渲染逻辑抽象为统一接口，
     *   使菜单框架可以透明地调用任意页面的渲染函数，无需关心具体实现细节。
     *
     * 核心作用：
     *   1. 多态分发：同一接口，不同页面有不同渲染实现
     *   2. 解耦框架：框架只依赖函数指针，不依赖具体页面代码
     *   3. 动态切换：运行时更换渲染函数，实现页面切换效果
     * ========================================================================
     */
    typedef void (*MenuRenderFn)(const struct MenuPage* page,
                                 const struct MenuState* state);

    /* 菜单项描述符：文本 + 类型 + 目标页面 / 动作函数 */
    typedef struct {
        const char*    text;
        MenuItemType_e type;
        MenuPageId_e   target_page;
        MenuActionFn   action;
        uint16_t       bind_value;  /* 动作绑定值，用于统一注册入口参数传递 */
    } MenuItem_t;

    /* 页面描述符：ID + 标题 + 父页面 ID + 菜单项数组 + 数量 + 自定义渲染 */
    typedef struct MenuPage {
        MenuPageId_e      id;
        const char*       title;
        MenuPageId_e      parent_id;      /* PAGE_MAX 表示无父页面 */
        const MenuItem_t* items;
        uint8_t           item_count;
        MenuRenderFn      custom_render;  /* 为 NULL 则使用默认列表渲染 */
    } MenuPage_t;

    /* 菜单状态：当前页面、光标位置、窗口顶部索引与脏标志 */
    typedef struct MenuState {
        MenuPageId_e current_page;
        uint8_t      cursor;        /* 当前光标 (相对于页面项数组) */
        uint8_t      top_index;     /* 当前窗口顶部项索引 */
        HC_Bool_e    dirty;         /* 页面内容是否被标脏需要刷新 */
    } MenuState_t;

    /* ---- 公开 API ----------------------------------------------------------- */

    /* 菜单初始化 */
    void Menu_Init(void);

    /* 处理菜单按键输入 */
    void Menu_HandleKey(Key_Id_e key);

    /* 按需渲染当前页面 (脏标志为真时才执行清屏 + 重绘) */
    void Menu_RenderIfDirty(void);

    /* 切换当前页面 */
    void Menu_SetCurrentPage(MenuPageId_e page_id);

    /* 请求下一次刷新 */
    void Menu_RequestRedraw(void);

    /* 查询当前页面是否需要刷新 */
    HC_Bool_e Menu_IsDirty(void);

    /* 获取当前页面描述符 */
    const MenuPage_t* Menu_GetCurrentPage(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_MENU_CORE_H */
