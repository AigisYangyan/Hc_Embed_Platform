# Claude：根据 Codex 修复计划返工

读取：

Ai_tools/agent/codex_fix_plan.json

如果用户没有指定 task id，只列出 fix tasks，不修改代码。

如果用户指定 task id，只修复该 task。

规则：

1. 只修复 Codex 指出的具体问题。
2. 禁止顺手重构。
3. 禁止修改 fix task 未授权的文件。
4. 修复后运行 check_command。
5. 输出修改摘要和检查结果。
