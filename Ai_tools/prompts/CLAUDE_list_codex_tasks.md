# Claude：列出 Codex JSON Tasks

读取：

Ai_tools/agent/codex_plan.json

只列出 tasks，不修改任何文件。

输出格式：

| task id | title | allowed_files | check_command |
|---|---|---|---|

最后建议用户下一步输入：

```text
读取 @Ai_tools/prompts/CLAUDE_execute_codex_json.md 并执行。
任务单路径：@Ai_tools/agent/codex_plan.json
执行 task id：T001
先只输出执行计划，不做任何文件修改。等我回复“开始执行”后再动手。
```
