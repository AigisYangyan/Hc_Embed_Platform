# Codex：审查当前 Diff

你是 code reviewer，不要修改代码。

请审查当前 git diff。

重点检查：

1. Claude 是否只执行了 `Ai_tools/agent/codex_plan.json` 里的指定 task。
2. 是否修改了 forbidden_files。
3. 是否存在任务外重构。
4. 是否存在隐藏 bug。
5. 是否存在编译风险。
6. 是否建议提交。

如果不建议提交，请只创建或更新：

Ai_tools/agent/codex_fix_plan.json

不要改业务代码。
