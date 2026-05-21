/**
 * @file    menu_pages.c
 * @brief   OLED 菜单页面静态注册表实现
 *
 * 本文件负责集中定义页面描述符与菜单项静态表。
 *
 * 功能范围：
 * - 定义一级主菜单 (PAGE_HOME)、DEBUG 二级菜单 (PAGE_DEBUG_MENU)、统一运行页 (PAGE_RUNNING)
 * - 根据 App 注册表自动构建菜单项数组
 * - 提供 App 运行页统一渲染分发函数
 *
 * 设计约定：
 * - 页面注册采用编译期静态数组
 * - DEBUG 一级入口仍是普通子页面跳转，不属于 App 本体
 * - HOME 与 DEBUG 子项统一从 app_registry.c 自动生成
 * - 新增一个 App 时，优先只在 App 注册表补一行
 */

#include "ui/menu_pages.h"
#include "app/app_registry.h"
#include "runtime/runtime.h"
#include "hc_driver/hc_driver_oled.h"

/* ---- 布局常量 ----------------------------------------------------------- */

#define MENU_RUNNING_X          20u
#define MENU_RUNNING_Y          3u
#define MENU_HOME_FIXED_COUNT   1u  /* HOME 固定项：DEBUG 入口 */

/* ---- 菜单静态缓存 ------------------------------------------------------- */

static MenuItem_t s_home_items[MENU_HOME_FIXED_COUNT + HC_APP_MAX];
static MenuItem_t s_debug_items[HC_APP_MAX];

/* ---- 静态渲染函数 ------------------------------------------------------- */

/**
 * @brief 统一运行页渲染函数
 * @note  若当前 App 提供自定义 render，则优先调用；否则显示 APP
 */
static void menu_render_running_page(const MenuPage_t* page,
                                     const MenuState_t* state)
{
    const HcAppReg_t* app = HcRuntime_GetActiveApp();

    if ((app != (const HcAppReg_t*)0) &&
        (app->render != (HcAppRenderFn)0)) {
        app->render(page, state);
        return;
    }

    HC_UNUSED(page);
    HC_UNUSED(state);
    OLED_ShowString(MENU_RUNNING_X, MENU_RUNNING_Y, "APP", 16u);
}

/* ---- 页面静态注册 ------------------------------------------------------- */

MenuPage_t g_menu_pages[] = {
    { PAGE_HOME,       "Menu",  PAGE_MAX,  s_home_items,  0u, (MenuRenderFn)0           },
    { PAGE_DEBUG_MENU, "DEBUG", PAGE_HOME, s_debug_items, 0u, (MenuRenderFn)0           },
    { PAGE_RUNNING,    "APP",   PAGE_HOME, (const MenuItem_t*)0, 0u, menu_render_running_page }
};

const uint8_t g_menu_page_count =
    (uint8_t)(sizeof(g_menu_pages) / sizeof(g_menu_pages[0]));

/* ---- 页面初始化接口 ----------------------------------------------------- */

void MenuPages_Init(void)
{
    uint8_t home_dynamic_count;
    uint8_t debug_item_count;

    /* HOME 固定项：DEBUG 子菜单入口 */
    s_home_items[0].text = "DEBUG";
    s_home_items[0].type = MENU_ITEM_SUBPAGE;
    s_home_items[0].target_page = PAGE_DEBUG_MENU;
    s_home_items[0].action = (MenuActionFn)0;
    s_home_items[0].bind_value = MENU_BIND_NONE;

    /* HOME / DEBUG 动态项由 app_registry 自动生成 */
    home_dynamic_count = AppRegistry_BuildMenuItems(HC_FALSE,
        &s_home_items[MENU_HOME_FIXED_COUNT],
        HC_APP_MAX);
    debug_item_count = AppRegistry_BuildMenuItems(HC_TRUE,
        s_debug_items,
        HC_APP_MAX);

    g_menu_pages[PAGE_HOME].item_count =
        (uint8_t)(MENU_HOME_FIXED_COUNT + home_dynamic_count);
    g_menu_pages[PAGE_DEBUG_MENU].item_count = debug_item_count;
}
