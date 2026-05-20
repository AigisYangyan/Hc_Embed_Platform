# Codex：生成 JSON 任务单

你是架构规划 agent，不是执行 agent。

不要修改业务代码。

请只创建或更新这个文件：

Ai_tools/agent/codex_plan.json

要求它是合法 JSON，格式参考：

Ai_tools/agent/codex_plan.schema.json

请根据当前项目结构和用户需求，拆出 1 到 3 个小任务。

## 每个 task 必须包含

- id
- title
- goal
- allowed_files
- forbidden_files
- steps
- check_command
- acceptance_criteria
- stop_conditions

## 规则

1. 每个任务必须能独立交给 Claude Code 执行。
2. allowed_files 必须具体，尽量不要写通配符。
3. forbidden_files 必须具体。
4. check_command 如果项目没有构建命令，就用 `git diff --check`。
5. stop_conditions 必须写清楚什么时候停止。
6. 不要改任何业务代码。
7. 只生成或更新 `Ai_tools/agent/codex_plan.json`。
8. 任务必须小，不要一次性大重构。
9. 如果需求过大，拆成多个小 task。

输出完成后，请说明：

- 生成了几个任务
- 第一个建议执行哪个 task id
