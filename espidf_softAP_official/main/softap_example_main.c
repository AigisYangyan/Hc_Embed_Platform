// softap_example_main.c
// 示例：将 ESP32-S3 作为 WiFi SoftAP（热点）并通过简单的 HTTP 页面触发 GPIO 输出
// 功能：
// - 初始化 NVS、网络接口和 WiFi SoftAP
// - 启动一个内置的 HTTP 服务器，提供“UNLOCK”按钮
// - 按下按钮时通过 GPIO 控制一个“电子锁”开闸动作（短时激活）

#include <string.h>

// FreeRTOS 头文件（任务与延时）
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 硬件与 SDK 功能头文件
#include "driver/gpio.h"        // GPIO 控制
#include "esp_err.h"           // 错误码与检查宏
#include "esp_event.h"         // 事件循环
#include "esp_http_server.h"   // 简单 HTTP 服务器
#include "esp_log.h"           // 日志打印
#include "esp_mac.h"           // MAC 地址相关宏
#include "esp_netif.h"         // 网络接口
#include "esp_wifi.h"          // WiFi 配置与控制
#include "nvs_flash.h"         // 非易失性存储（NVS）

#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

// 用到的 GPIO 与电子锁行为定义
// 本示例使用 GPIO4 控制锁，高电平表示打开（根据电路可能反向）
#define LOCK_GPIO                  GPIO_NUM_4
#define LOCK_ACTIVE_LEVEL          1
#define LOCK_INACTIVE_LEVEL        0
// 开锁脉冲时长（毫秒）
#define UNLOCK_TIME_MS             800

// 日志 TAG，便于过滤输出
static const char *TAG = "wifi softAP";

static void lock_gpio_init(void)
{
    // 复位并配置锁控制 GPIO 为输出，初始为未激活状态
    gpio_reset_pin(LOCK_GPIO);
    gpio_set_direction(LOCK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LOCK_GPIO, LOCK_INACTIVE_LEVEL);

    ESP_LOGI(TAG, "Lock GPIO initialized: GPIO%d", LOCK_GPIO);
}

static void lock_open_once(void)
{
    // 打印开始日志，短时拉高 GPIO 实现开锁动作，然后恢复为低电平
    ESP_LOGI(TAG, "Unlock start");

    gpio_set_level(LOCK_GPIO, LOCK_ACTIVE_LEVEL);
    // 延时等待电磁锁动作完成（将任务阻塞，不占用 CPU）
    vTaskDelay(pdMS_TO_TICKS(UNLOCK_TIME_MS));
    gpio_set_level(LOCK_GPIO, LOCK_INACTIVE_LEVEL);

    ESP_LOGI(TAG, "Unlock done");
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    // 返回一个非常简单的 HTML 页面，包含跳转到 /unlock 的按钮
    const char *html =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        "<title>ESP32-S3 Lock</title>"
        "<style>"
        "body{font-family:Arial;text-align:center;margin-top:80px;}"
        "button{font-size:28px;padding:18px 40px;border-radius:12px;}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>ESP32-S3 WiFi Lock</h1>"
        "<p>Phone -> WiFi -> ESP32-S3 -> GPIO4</p>"
        "<a href=\"/unlock\">"
        "<button>UNLOCK</button>"
        "</a>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

static esp_err_t unlock_get_handler(httpd_req_t *req)
{
    // 收到 HTTP GET /unlock 时执行一次开锁动作，并返回文本响应
    lock_open_once();

    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    httpd_resp_sendstr(req, "Unlock OK");

    return ESP_OK;
}

static void start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // 使用默认配置启动嵌入式 HTTP 服务器
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    // 注册根路径处理器，返回 HTML 页面
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL,
    };

    // 注册 /unlock 路径处理器，触发开锁
    httpd_uri_t unlock_uri = {
        .uri = "/unlock",
        .method = HTTP_GET,
        .handler = unlock_get_handler,
        .user_ctx = NULL,
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &unlock_uri));

    ESP_LOGI(TAG, "HTTP server started. Open http://192.168.4.1");
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;

        // 当有 STA（手机或其他设备）连接到 SoftAP 时打印其 MAC 与 AID
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;

        // 当 STA 断开连接时打印相关信息（包括断开原因）
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_init_softap(void)
{
    // 初始化网络接口与默认事件循环
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 创建默认的 WiFi AP 网络接口
    esp_netif_create_default_wifi_ap();

    // 初始化 WiFi 栈
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册 WiFi 事件回调，监听 STA 连接/断开
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL
        )
    );

    // 配置 SoftAP 参数（SSID、密码、信道、最大连接数、认证模式等）
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    // 如果没有设置密码则降级为开放网络
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // 将 WiFi 模式设置为 AP，应用配置并启动
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void app_main(void)
{
    // 初始化 NVS（在 WiFi 使用之前必须初始化）
    esp_err_t ret = nvs_flash_init();

    // 如果 NVS 被损坏或需要升级版本，则擦除后重新初始化
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // 初始化用于控制电子锁的 GPIO
    lock_gpio_init();

    // 启动 WiFi SoftAP，并开启 HTTP 服务以提供控制页面
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    start_webserver();
}
