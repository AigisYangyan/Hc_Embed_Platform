#!/usr/bin/env python3
"""
HC_EMBED_RULES 静态边界检查脚本

只做扫描，不做自动修复。输出违例清单并返回非零退出码。
覆盖四类违例:
  1. app 直接依赖硬件抽象 (port_key / port_oled / hc_hal)
  2. service 之间互相依赖
  3. middleware 直接依赖厂商 SDK / BSP
  4. driver 反向依赖 service / app

用法:
  python Ai_tools/tools/check_hc_embed_arch.py
"""

import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent

# ---------------------------------------------------------------------------
# 违例规则定义
# ---------------------------------------------------------------------------
# 每条规则: { name, dirs, description, forbidden (regex list) }
# dirs 中的目录如果不存在则跳过该目录的检查。

RULES = [
    {
        "name": "app-to-hal",
        "dirs": ["app", "hc_app"],
        "description": "app 直接依赖硬件抽象层",
        "forbidden": [
            r"port_key\.h",
            r"port_oled\.h",
            r"port_uart",
            r"port_systick",
            r"hc_hal/",
        ],
    },
    {
        "name": "service-cross-dependency",
        "dirs": ["service"],
        "description": "service 之间互相依赖",
        "cross_check": True,
        "forbidden": [],
    },
    {
        "name": "middleware-to-vendor-sdk",
        "dirs": ["hc_middleware"],
        "description": "middleware 直接依赖厂商 SDK / BSP",
        "forbidden": [
            r"stm32",
            r"cmsis",
            r"hal_conf",
            r"FreeRTOS",
            r"freertos",
            r"nrf52",
            r"nrf\b",
            r"esp32",
            r"esp\b",
            r"msp430",
            r"ti/",
            r"ch32",
            r"gd32",
            r"mm32",
        ],
    },
    {
        "name": "driver-to-service",
        "dirs": ["hc_driver"],
        "description": "driver 反向依赖 service / app",
        "forbidden": [
            r"hc_service/",
            r"hc_app/",
            r"service/",
            r"app/",
        ],
    },
]

SKIP_DIRS = {".git", "__pycache__", "Ai_tools", "examples"}


# ---------------------------------------------------------------------------
def should_skip(filepath: Path) -> bool:
    parts = filepath.relative_to(REPO_ROOT).parts
    return bool(SKIP_DIRS & set(parts))


def scan_includes(filepath: Path) -> list:
    """提取 C 源文件中所有 #include 路径"""
    includes = []
    try:
        text = filepath.read_text(encoding="utf-8", errors="ignore")
        for line in text.splitlines():
            m = re.match(r'^\s*#include\s+["<]([^">]+)[">]', line)
            if m:
                includes.append(m.group(1))
    except Exception:
        pass
    return includes


# ---------------------------------------------------------------------------
def check_cross_dependency(dir_path: Path, dir_name: str, rule: dict) -> list:
    """检查同层内子模块间的互相依赖（针对 service/ 目录结构）"""
    violations = []
    # 获取该目录下的所有子目录（各 service 模块）
    subdirs = [d for d in dir_path.iterdir() if d.is_dir()]
    if len(subdirs) < 2:
        return violations

    subdir_names = {d.name for d in subdirs}

    for src_file in dir_path.rglob("*.[ch]"):
        if should_skip(src_file):
            continue
        includes = scan_includes(src_file)

        # 确定源文件所属子模块
        try:
            rel = src_file.relative_to(dir_path)
        except ValueError:
            continue
        src_module = rel.parts[0] if rel.parts else ""

        for inc in includes:
            inc_first = inc.split("/")[0]
            if inc_first in subdir_names and inc_first != src_module:
                violations.append(
                    {
                        "file": str(src_file.relative_to(REPO_ROOT)),
                        "rule": rule["name"],
                        "description": rule["description"],
                        "detail": f'{src_module} 引用 {inc_first}: #include "{inc}"',
                    }
                )
    return violations


def check_forbidden_includes(dir_path: Path, rule: dict) -> list:
    """检查禁止引用的 include 模式"""
    violations = []
    for src_file in dir_path.rglob("*.[ch]"):
        if should_skip(src_file):
            continue
        includes = scan_includes(src_file)
        for inc in includes:
            for pat in rule["forbidden"]:
                if re.search(pat, inc, re.IGNORECASE):
                    violations.append(
                        {
                            "file": str(src_file.relative_to(REPO_ROOT)),
                            "rule": rule["name"],
                            "description": rule["description"],
                            "detail": f'#include "{inc}" 命中禁止规则 "{pat}"',
                        }
                    )
    return violations


# ---------------------------------------------------------------------------
def main() -> int:
    all_violations = []

    for rule in RULES:
        for dir_name in rule["dirs"]:
            dir_path = REPO_ROOT / dir_name
            if not dir_path.is_dir():
                continue

            if rule.get("cross_check"):
                all_violations.extend(
                    check_cross_dependency(dir_path, dir_name, rule)
                )
            else:
                all_violations.extend(check_forbidden_includes(dir_path, rule))

    if all_violations:
        print(
            f"HC_EMBED_RULES 架构违例: {len(all_violations)} 处\n"
        )
        for v in all_violations:
            print(f"  [{v['rule']}] {v['file']}")
            print(f"    {v['detail']}")
        print(
            f"\n共 {len(all_violations)} 处违例。请人工评估后修改，禁止自动修复。"
        )
        return 1

    print("HC_EMBED_RULES 架构检查通过，未发现违例。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
