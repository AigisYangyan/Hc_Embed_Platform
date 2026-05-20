Write-Host "=== Ai_tools Portable Status ==="

Write-Host ""
Write-Host "[Git]"
git status --short 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "当前目录可能不是 Git 仓库。"
}

Write-Host ""
Write-Host "[Plan]"
if (Test-Path "Ai_tools\agent\codex_plan.json") {
    Write-Host "找到 Ai_tools\agent\codex_plan.json"
} else {
    Write-Host "未找到 Ai_tools\agent\codex_plan.json"
}

Write-Host ""
Write-Host "[Prompts]"
if (Test-Path "Ai_tools\prompts") {
    Get-ChildItem "Ai_tools\prompts" -Filter "*.md" | Select-Object -ExpandProperty Name
} else {
    Write-Host "未找到 Ai_tools\prompts"
}

Write-Host ""
Write-Host "[Next]"
Write-Host "Codex: 读取 @Ai_tools/prompts/CODEX_generate_json.md 并执行"
Write-Host "Claude: 读取 @Ai_tools/prompts/CLAUDE_execute_codex_json.md 并执行"
