# Claude Code 执行总规则

你是执行型工程师，不是架构师。

## 总原则

1. 先读任务单，再改代码。
2. 只执行用户指定的 task id。
3. 严格遵守 allowed_files。
4. 严格禁止修改 forbidden_files。
5. 如果必须改 allowed_files 之外的文件，必须先停止并说明原因。
6. 禁止大规模重构。
7. 禁止顺手优化。
8. 禁止删除功能代码来通过检查。
9. 每次只做一个小步骤。
10. 修改完成后必须汇报：
   - 修改了哪些文件
   - 为什么修改
   - 是否运行检查命令
   - 检查是否通过
   - 当前 git diff 摘要

## 架构基线：HC_EMBED_RULES（唯一目标架构）

所有新增代码和修改必须遵守 HC_EMBED_RULES 分层，禁止继续沿用旧的 HC_Sys_Menu 菜单框架分层。

### 目标分层

| 层 | 目录 | 职责 |
|---|---|---|
| hc_common | `hc_common/` | 平台公共类型、错误码、基础宏 |
| hc_cfg | `hc_cfg/` | 编译期配置（开关、引脚、功能裁剪） |
| hc_hal | `hc_hal/` | 硬件抽象层（MCU 薄封装） |
| hc_driver | `hc_driver/` | 设备驱动层（外设芯片驱动） |
| hc_middleware | `hc_middleware/` | 通用中间件（协议栈、算法库） |
| hc_service | `hc_service/` | 业务服务层 |
| hc_task | `hc_task/` | 任务调度层 |
| hc_app | `hc_app/` | 应用入口与场景逻辑 |

### 依赖方向

```
hc_app → hc_service → hc_middleware → hc_driver → hc_hal
              ↓            ↓
           hc_task ←──────┘

hc_cfg, hc_common ← 所有层均可引用
```

### 编码红线

禁止：
1. hc_app 直接依赖 hc_hal / hc_driver
2. hc_service 之间互相依赖
3. hc_middleware 直接依赖厂商 SDK / BSP
4. hc_driver 反向依赖 hc_service / hc_app
5. hc_hal 依赖除 hc_common / hc_cfg 之外的任何层
6. 跨层直接访问全局变量（必须通过接口函数）
7. 动态内存分配（全部静态分配）
8. ISR 中做业务处理（仅标记/缓冲）
9. 向旧的 port/runtime/service/app/ui/scheduler/system 目录增加新模块或新耦合

### 旧目录处理

`port/` `runtime/` `service/` `app/` `ui/` `system/` `scheduler/` 为 HC_Sys_Menu v1.0 遗留代码，处于待迁移状态。

- **禁止**在上列旧目录中新增文件、模块或依赖关系。
- **禁止**以旧分层为参考设计新功能。
- 修改旧目录中的代码仅限 bug 修复，且不得扩大其耦合范围。

### 静态边界检查

在执行任何架构相关任务前，先运行：

```bash
python Ai_tools/tools/check_hc_embed_arch.py
```

该脚本扫描违例模式并返回非零退出码。查出的违例不可自动修复，必须人工评估。

## RAM 优先执行优先级

所有代码生成和修改必须按以下优先级决策（1 为最高）：

| 优先级 | 规则 | 说明 |
|--------|------|------|
| **1** | 编译期静态分配 | 所有数据结构编译期确定大小，禁止 `malloc` / `calloc` / `realloc` |
| **2** | 功能开关裁剪 | 通过 `hc_feature_cfg.h` 关闭不需要的模块，未使能代码完全不参与编译链接 |
| **3** | 零拷贝传递 | 模块间传指针不复制数据，缓冲区所有权在 `init()` 一次性分配绑定 |
| **4** | 静态对象池 | 同类型小对象使用编译期配置大小的静态池，禁止运行时动态扩容 |
| **5** | 惰性初始化 | 非启动关键模块在首次使用时才初始化，避免启动阶段全部展开占用 RAM |

## 轻量注入规则

### 允许静态注入的场景

只有以下场景允许使用**静态函数表**或**初始化期句柄注入**：

| 场景 | 注入方式 | 示例 |
|------|---------|------|
| 可替换通信后端 | 静态函数表 | UART/SPI/I2C 不同实例的 `comm_ops->send()` |
| 可替换存储后端 | 静态函数表 | Flash/EEPROM/SD 卡不同实现的 `storage_ops->read()` |
| 可替换执行器/传感器 | 句柄注入 | 不同型号温度传感器的 `sensor_ctx` 句柄 |
| 需要 mock 的模块 | 静态函数表 | 单元测试中替换硬件依赖 |

### 注入方式限制

| 方式 | 允许 | 说明 |
|------|------|------|
| 编译期宏选择 | ✅ | `hc_cfg` 通过 `#define` 选择具体实现文件 |
| 链接期替换 | ✅ | Makefile/CMake 选择编译单元，同名函数不同实现 |
| 初始化期一次性注入 | ✅ | `init(&ops_table)` 设置后运行期不变 |
| 运行时 setter 注入 | ❌ | 禁止运行中替换函数表指针 |
| 动态注册表 | ❌ | 禁止运行时 `register` / `unregister` 机制 |
| 全局服务容器 | ❌ | 禁止服务定位器、IOC 容器 |

### 禁止注入的场景

- hc_service 之间互相注入（各 service 独立）
- hc_app 通过容器解析依赖（hc_app 应在入口直接组装所有依赖）
- 单例硬件资源不需要"可替换"抽象（SysTick、唯一调试 UART）

## 业务层禁止依赖清单

以下符号和头文件**绝对禁止**出现在 hc_service、hc_task、hc_app 中：

```text
禁止类别                          示例
──────────────────────────────────────────────────────
芯片型号宏                        __STM32F1xx__、__MSPM0__、CHIP_FEATURE_*
厂商 SDK 头文件                   stm32f1xx_hal.h、driverlib.h、esp_idf_*
GPIO 操作宏/函数                  HAL_GPIO_WritePin()、gpio_set_level()
UART 操作宏/函数                  HAL_UART_Transmit()、uart_write_bytes()
PWM 操作宏/函数                   __HAL_TIM_SET_COMPARE()、ledc_set_duty()
板级引脚号宏                      PIN_LED、BOARD_SPI_MISO、LED_GPIO_Port
寄存器地址                        0x40010800、(volatile uint32_t*)0x...
中断相关宏/函数                   NVIC_EnableIRQ()、__enable_irq()
DMA 操作宏/函数                   HAL_DMA_Start()、dma_start()
```

业务层如需使用上述能力，必须通过 hc_hal 或 hc_driver 的抽象接口调用，不得直接引用具体实现。

## 禁止运行时容器式 DI

以下模式**绝对禁止**在整个代码库中出现：

```text
1. 依赖注入容器 / 服务定位器 —— 如 ServiceContainer、IocContainer、ServiceLocator
2. 动态注册式对象图 —— 运行时 register_service() / unregister_service() 机制
3. 反射 / 序列化驱动的依赖解析 —— 通过字符串名称查找并创建对象
4. 运行时配置驱动的模块加载 —— 解析 JSON/TOML/XML 来决定加载哪些模块
5. 虚函数表（vtable）驱动的多态 —— C++ virtual / 手写 vtable 分发
```

允许的替代方式：

```text
1. 编译期宏选择具体实现 → hc_cfg/ 控制
2. 链接期同名函数替换 → Makefile/CMake 控制
3. 初始化期静态函数表一次性注入 → init() 参数传递
```
