$ErrorActionPreference = "Stop"

$PlanFile = "Ai_tools\agent\codex_plan.json"

if (!(Test-Path $PlanFile)) {
    throw "找不到 $PlanFile"
}

try {
    $plan = Get-Content $PlanFile -Raw | ConvertFrom-Json
} catch {
    throw "codex_plan.json 不是合法 JSON"
}

if ($null -eq $plan.tasks) {
    throw "缺少 tasks 字段"
}

foreach ($task in $plan.tasks) {
    foreach ($field in @("id","title","goal","allowed_files","forbidden_files","steps","check_command","acceptance_criteria","stop_conditions")) {
        if ($null -eq $task.$field) {
            throw "任务缺少字段：$field"
        }
    }
}

Write-Host "codex_plan.json 基础校验通过。任务数量：$($plan.tasks.Count)"
