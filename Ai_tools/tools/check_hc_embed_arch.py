#!/usr/bin/env python3
"""Check canonical hc_hal public header boundary rules per HC_EMBED_RULES."""

import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HC_HAL_DIR = os.path.join(ROOT, "hc_hal")

# Patterns that MUST NOT appear in canonical hc_hal public headers.
FORBIDDEN_INCLUDES = [
    r'#include\s+["<]port/hc_common\.h[">]',
    r'#include\s+["<]usart\.h[">]',
    r'#include\s+["<]ti_msp_dl_config\.h[">]',
    r'#include\s+["<]driverlib[^>"]*[">]',
    r'#include\s+<ti/',
]

FORBIDDEN_NAMES = [
    r'\bVPIN_\w+',
    r'\bUART_CH_\w+',       # bare project UART names (not HC_HAL_UART_CH_ which is neutral)
    r'\bSTEPMOTOR\b',
    r'\bVOFA\b',
    r'\bVISION\b',
    r'\bGRAY_\w+',
    r'\bCOUNT_\w+',
    r'\bMOTOR_L\b',
    r'\bMOTOR_R\b',
]

# Legacy port compat patterns — zero tolerance, fail immediately.
LEGACY_PORT_PATTERNS = [
    r'#include\s+["<]port_key\.h[">]',
    r'#include\s+["<]port_oled\.h[">]',
    r'#include\s+["<]port_systick\.h[">]',
    r'#include\s+["<]port_uart_vofa\.h[">]',
    r'#include\s+["<]hc_common\.h[">]',
    r'#include\s+["<]port/hc_hal[^>"]*[">]',
]

# Directories excluded from legacy port scan.
LEGACY_SCAN_EXCLUDE = {'Ai_tools', '.git', '.claude'}

violations = []


def is_public_header(path):
    """True if path is a canonical hc_hal public .h file, excluding platform internals."""
    rel = os.path.relpath(path, ROOT)
    if not rel.startswith("hc_hal") or not rel.endswith(".h"):
        return False
    if os.sep + "platform" + os.sep in rel:
        return False
    return True


def check_file(filepath):
    rel = os.path.relpath(filepath, ROOT)
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    for pattern in FORBIDDEN_INCLUDES:
        for m in re.finditer(pattern, content):
            violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: forbidden include `{m.group(0)}`")

    for pattern in FORBIDDEN_NAMES:
        if pattern == r'\bUART_CH_\w+':
            for m in re.finditer(pattern, content):
                name = m.group(0)
                if not name.startswith("HC_HAL_"):
                    violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: project name `{name}`")
        else:
            for m in re.finditer(pattern, content):
                violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: project name `{m.group(0)}`")


def check_legacy_port(filepath):
    """Check for any legacy port compat includes (zero tolerance)."""
    rel = os.path.relpath(filepath, ROOT)
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    for pattern in LEGACY_PORT_PATTERNS:
        for m in re.finditer(pattern, content):
            violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: legacy port include `{m.group(0)}`")


def main():
    # 1. Check canonical hc_hal public headers
    for root, dirs, files in os.walk(HC_HAL_DIR):
        dirs[:] = [d for d in dirs if d != "platform"]  # skip platform internals
        for f in files:
            if f.endswith(".h"):
                check_file(os.path.join(root, f))

    # 2. Scan entire repo for legacy port compat includes (zero tolerance)
    for root, dirs, files in os.walk(ROOT, topdown=True):
        dirs[:] = [d for d in dirs if d not in LEGACY_SCAN_EXCLUDE and not d.startswith('.')]
        for f in files:
            if f.endswith(('.c', '.h')):
                check_legacy_port(os.path.join(root, f))

    if violations:
        for v in violations:
            print(f"  ERROR: {v}", file=sys.stderr)
        print(f"架构违例: {len(violations)} 处", file=sys.stderr)
        sys.exit(1)

    print("架构检查通过: 0 处违例")
    sys.exit(0)


if __name__ == "__main__":
    main()
