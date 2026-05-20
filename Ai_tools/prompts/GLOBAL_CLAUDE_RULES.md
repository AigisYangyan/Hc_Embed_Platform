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
