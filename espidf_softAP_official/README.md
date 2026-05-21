# ESP32-S3 SoftAP 示例（第一个 ESP32-S3 Demo）

这是一个基于 ESP-IDF 的简单演示：将 ESP32-S3 作为 WiFi SoftAP（热点），通过网页按钮触发 GPIO，控制电子锁或继电器。

重点说明
- 这是你仓库中的第一个 ESP32-S3 示例，侧重于快速上手与硬件验证。
- 主程序位于 [main/softap_example_main.c](main/softap_example_main.c) ，示例会在 `GPIO4` 上输出短时脉冲作为“开锁”动作。

功能概览
- 启动 SoftAP（热点）并监听 STA（手机/电脑）连接事件。
- 启动嵌入式 HTTP 服务器，根页面提供一个 `UNLOCK` 按钮，访问 `/unlock` 会触发一次开锁脉冲。

硬件连接建议
- 开发板：任意支持 ESP32-S3 的开发板。
- 控制引脚：GPIO4（代码中定义为 `LOCK_GPIO`）。
- 驱动：请使用继电器模块或驱动级电路（如 MOSFET、驱动器），避免直接用 MCU 引脚驱动高电流设备。

配置
- 在 `menuconfig` 中（`idf.py menuconfig`）设置 WiFi 参数：
    - `Example Configuration` → `WiFi SSID`（CONFIG_ESP_WIFI_SSID）
    - `Example Configuration` → `WiFi Password`（CONFIG_ESP_WIFI_PASSWORD）
    - 其他参数：`CONFIG_ESP_WIFI_CHANNEL`, `CONFIG_ESP_MAX_STA_CONN`

构建与烧录
1. 进入项目根目录（包含 `CMakeLists.txt`）：

```bash
cd path/to/espidf_softAP_official
```

2. 构建：

```bash
idf.py build
```

3. 烧录并打开串口监视（将 `COM3` 替换为你的串口）：

```bash
idf.py -p COM3 flash monitor
```

4. 退出监视器：按 `Ctrl+]`。

运行说明
- 设备默认 SoftAP IP 为 `192.168.4.1`。
- 手机/电脑连接到该热点后，访问 `http://192.168.4.1`，点击 `UNLOCK` 按钮将触发一次开锁脉冲（默认 800ms，可在源码调整）。

安全与注意事项
- 确认外设供电与接线安全，避免 MCU 引脚承受大电流。
- 根据你的电路，可能需要调整 `LOCK_ACTIVE_LEVEL`/`LOCK_INACTIVE_LEVEL`（高电平或低电平有效）。

扩展建议
- 增加认证（如基于密码的 Web 表单或简单 token）以避免未经授权的访问。
- 增加日志记录或使用 HTTPS（更复杂，需要证书与额外配置）。

参考
- ESP-IDF 官方文档与入门指南：
    - https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

如需我：
- 将 README 英文化或补充接线图（示意图 PNG/SVG）。
- 运行一次 `idf.py build` 并把输出日志贴给你进行问题排查。

---
已由仓库内 `main/softap_example_main.c` 示例实现，本 README 为快速上手指南。
