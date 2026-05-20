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

# Known legacy violations allowed to persist.
KNOWN_LEGACY = [
    os.path.join(ROOT, "app", "demo_blink", "demo_blink.c"),
]

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
        # For UART_CH_ pattern, exclude HC_HAL_UART_CH_ (neutral naming)
        if pattern == r'\bUART_CH_\w+':
            for m in re.finditer(pattern, content):
                name = m.group(0)
                if not name.startswith("HC_HAL_"):
                    violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: project name `{name}`")
        else:
            for m in re.finditer(pattern, content):
                violations.append(f"{rel}:{content[:m.start()].count(chr(10))+1}: project name `{m.group(0)}`")


def check_legacy_violation(filepath):
    """Check if a file has a known legacy violation: include port_oled.h."""
    rel = os.path.relpath(filepath, ROOT)
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()
    if re.search(r'#include\s+["<]port_oled\.h[">]', content):
        violations.append(f"{rel}:1: legacy violation: includes port_oled.h")


def main():
    # 1. Check canonical hc_hal public headers
    for root, dirs, files in os.walk(HC_HAL_DIR):
        dirs[:] = [d for d in dirs if d != "platform"]  # skip platform internals
        for f in files:
            if f.endswith(".h"):
                check_file(os.path.join(root, f))

    # 2. Check known legacy files
    for legacy_file in KNOWN_LEGACY:
        if os.path.isfile(legacy_file):
            check_legacy_violation(legacy_file)

    # 3. Count canonical violations (exclude the known legacy one)
    legacy_violations = [v for v in violations if "legacy violation" in v]
    canonical_violations = [v for v in violations if "legacy violation" not in v]

    if canonical_violations:
        for v in canonical_violations:
            print(f"  ERROR: {v}", file=sys.stderr)

    if legacy_violations:
        for v in legacy_violations:
            print(f"  WARNING: {v}", file=sys.stderr)

    total_legacy = len(legacy_violations)
    total_canonical = len(canonical_violations)

    if total_canonical > 0:
        print(f"架构违例: {total_canonical + total_legacy} 处 (canonical: {total_canonical}, legacy: {total_legacy})", file=sys.stderr)
        sys.exit(1)

    if total_legacy == 1 and "demo_blink" in str(legacy_violations[0]) and "port_oled.h" in str(legacy_violations[0]):
        print(f"架构违例: {total_legacy} 处")
        print(f"  [KNOWN] app/demo_blink/demo_blink.c: includes port_oled.h")
        sys.exit(0)

    if total_legacy > 0:
        print(f"架构违例: {total_legacy} 处", file=sys.stderr)
        sys.exit(1)

    print("架构检查通过: 0 处违例")
    sys.exit(0)


if __name__ == "__main__":
    main()
