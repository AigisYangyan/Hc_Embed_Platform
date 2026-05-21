/**
 * @file    menu_core.c
 * @brief   轻量注册式 OLED 菜单核心实现
 *
 * 本文件实现菜单状态管理、按键映射、页面跳转与默认列表渲染。
 *
 * 功能范围：
 * - 维护菜单当前页面、光标与可视窗口状态
 * - 将物理按键映射为菜单动作
 * - 提供默认列表渲染与按需刷新接口
 *
 * 设计约定：
 * - 菜单核心负责显示、导航与页面级返回链路
 * - 页面注册表由 menu_pages.c 提供
 * - 页面内容未变化时不触发 OLED 重绘 (脏标志驱动)
 */

#include "ui/menu_core.h"
#include "ui/menu_pages.h"
#include "runtime/runtime.h"
#include "hc_driver/hc_driver_oled.h"

/* ---- 布局常量 ----------------------------------------------------------- */

#define MENU_TITLE_Y_PAGE        0u  /* 标题行 page 地址 */
#define MENU_FIRST_ITEM_Y_PAGE   2u  /* 第一行菜单项 page 地址 */
#define MENU_PAGE_ROW_STEP       2u  /* 菜单项行距 (page 增量) */
#define MENU_VISIBLE_ITEM_COUNT  3u  /* 可见菜单项数量 */
#define MENU_CURSOR_X            0u  /* 光标 X 像素列 */
#define MENU_TEXT_X              16u /* 菜单文本 X 像素列 */

/* ---- 模块状态 ----------------------------------------------------------- */

static MenuState_t s_tMenuState = {
    PAGE_HOME,
    0u,
    0u,
    HC_TRUE
};

/* ---- 静态辅助函数 ------------------------------------------------------- */

/* 物理按键 -> 菜单动作映射 */
static MenuKeyAction_e menu_map_key_to_action(Key_Id_e key)
{
    switch (key) {
    case KEY_ID_K1: return MENU_KEY_UP;
    case KEY_ID_K2: return MENU_KEY_DOWN;
    case KEY_ID_K3: return MENU_KEY_ENTER;
    case KEY_ID_K4: return MENU_KEY_BACK;
    default:        return MENU_KEY_NONE;
    }
}

/* 根据页面 ID 查找页面描述符 */
static const MenuPage_t* menu_find_page(MenuPageId_e id)
{
    for (uint8_t i = 0u; i < g_menu_page_count; i++) {
        if (g_menu_pages[i].id == id) {
            return &g_menu_pages[i];
        }
    }
    return (const MenuPage_t*)0;
}

static void menu_draw_string(uint8_t x, uint8_t y, const char* text)
{
    OLED_ShowString(x, y, text, 16u);
}

/* 切换页面时重置光标与窗口状态 */
static void menu_reset_page_view(void)
{
    s_tMenuState.cursor = 0u;
    s_tMenuState.top_index = 0u;
    s_tMenuState.dirty = HC_TRUE;
}

/* 同步光标与窗口状态，确保光标始终在可见范围内 */
static void menu_sync_scroll_window(const MenuPage_t* page)
{
    if ((page == (const MenuPage_t*)0) || (page->item_count == 0u)) {
        s_tMenuState.cursor = 0u;
        s_tMenuState.top_index = 0u;
        return;
    }

    if (s_tMenuState.cursor >= page->item_count) {
        s_tMenuState.cursor = (uint8_t)(page->item_count - 1u);
    }

    if (s_tMenuState.cursor < s_tMenuState.top_index) {
        s_tMenuState.top_index = s_tMenuState.cursor;
    }

    if (s_tMenuState.cursor >=
        (uint8_t)(s_tMenuState.top_index + MENU_VISIBLE_ITEM_COUNT)) {
        s_tMenuState.top_index =
            (uint8_t)(s_tMenuState.cursor - MENU_VISIBLE_ITEM_COUNT + 1u);
    }
}

/* 切换到指定页面 ID */
static void menu_switch_page(MenuPageId_e next_page)
{
    if (menu_find_page(next_page) == (const MenuPage_t*)0) {
        return;
    }

    s_tMenuState.current_page = next_page;
    menu_reset_page_view();
}

/* 从运行页 / DEBUG 页统一返回 HOME，并清掉当前 App */
static void menu_return_home_to_idle_page(void)
{
    (void)HcRuntime_RequestLeaveApp();
    menu_switch_page(PAGE_HOME);
}

/* 默认页面渲染：标题 + 菜单项列表 + 光标 */
static void menu_render_default_page(const MenuPage_t* page,
                                     const MenuState_t* state)
{
    uint8_t visible_count;

    if (page == (const MenuPage_t*)0) {
        return;
    }

    menu_draw_string(0u, MENU_TITLE_Y_PAGE, page->title);

    if (page->item_count == 0u) {
        menu_draw_string(0u, MENU_FIRST_ITEM_Y_PAGE, "No Items");
        return;
    }

    visible_count = (page->item_count > state->top_index)
        ? (uint8_t)(page->item_count - state->top_index)
        : 0u;

    if (visible_count > MENU_VISIBLE_ITEM_COUNT) {
        visible_count = MENU_VISIBLE_ITEM_COUNT;
    }

    for (uint8_t i = 0u; i < visible_count; i++) {
        uint8_t y_page = (uint8_t)(MENU_FIRST_ITEM_Y_PAGE + i * MENU_PAGE_ROW_STEP);
        uint8_t item_index = (uint8_t)(state->top_index + i);

        if (item_index == state->cursor) {
            OLED_ShowChar(MENU_CURSOR_X, y_page, '>', 16u);
        }

        menu_draw_string(MENU_TEXT_X, y_page, page->items[item_index].text);
    }
}

/* ---- 公开 API ----------------------------------------------------------- */

void Menu_Init(void)
{
    MenuPages_Init();
    s_tMenuState.current_page = PAGE_HOME;
    s_tMenuState.cursor = 0u;
    s_tMenuState.top_index = 0u;
    s_tMenuState.dirty = HC_TRUE;
}

void Menu_SetCurrentPage(MenuPageId_e page_id)
{
    menu_switch_page(page_id);
}

void Menu_HandleKey(Key_Id_e key)
{
    const MenuPage_t* page = Menu_GetCurrentPage();
    MenuKeyAction_e action = menu_map_key_to_action(key);

    if ((page == (const MenuPage_t*)0) || (action == MENU_KEY_NONE)) {
        return;
    }

    switch (action) {
    case MENU_KEY_UP:
        if ((page->item_count > 0u) && (s_tMenuState.cursor > 0u)) {
            s_tMenuState.cursor--;
            menu_sync_scroll_window(page);
            Menu_RequestRedraw();
        }
        break;

    case MENU_KEY_DOWN:
        if ((page->item_count > 0u) &&
            (s_tMenuState.cursor + 1u < page->item_count)) {
            s_tMenuState.cursor++;
            menu_sync_scroll_window(page);
            Menu_RequestRedraw();
        }
        break;

    case MENU_KEY_ENTER:
        if ((page->item_count > 0u) && (page->items != (const MenuItem_t*)0)) {
            const MenuItem_t* item = &page->items[s_tMenuState.cursor];

            if (item->type == MENU_ITEM_SUBPAGE) {
                menu_switch_page(item->target_page);
            }
            else if (item->type == MENU_ITEM_BACK) {
                if (page->parent_id < PAGE_MAX) {
                    menu_switch_page(page->parent_id);
                }
            }
            else if (item->type == MENU_ITEM_ACTION) {
                HC_Bool_e action_ok = HC_TRUE;

                if (item->action != (MenuActionFn)0) {
                    action_ok = item->action(item->bind_value);
                }

                if ((action_ok == HC_TRUE) && (item->target_page < PAGE_MAX)) {
                    menu_switch_page(item->target_page);
                }

                Menu_RequestRedraw();
            }
        }
        break;

    case MENU_KEY_BACK:
        /* 运行页 / DEBUG 页统一返回 HOME，并清掉当前 App */
        if ((page->id == PAGE_RUNNING) || (page->id == PAGE_DEBUG_MENU)) {
            menu_return_home_to_idle_page();
        }
        else if (page->parent_id < PAGE_MAX) {
            menu_switch_page(page->parent_id);
            Menu_RequestRedraw();
        }
        break;

    default:
        break;
    }
}

void Menu_RenderIfDirty(void)
{
    const MenuPage_t* page = Menu_GetCurrentPage();

    if ((page == (const MenuPage_t*)0) || (s_tMenuState.dirty == HC_FALSE)) {
        return;
    }

    /* 重渲染前清屏，避免残影 */
    OLED_Clear();

    if (page->custom_render != (MenuRenderFn)0) {
        page->custom_render(page, &s_tMenuState);
    }
    else {
        menu_render_default_page(page, &s_tMenuState);
    }

    /* 渲染完成后清除脏标志 */
    s_tMenuState.dirty = HC_FALSE;
}

void Menu_RequestRedraw(void)
{
    s_tMenuState.dirty = HC_TRUE;
}

HC_Bool_e Menu_IsDirty(void)
{
    return s_tMenuState.dirty;
}

const MenuPage_t* Menu_GetCurrentPage(void)
{
    return menu_find_page(s_tMenuState.current_page);
}
