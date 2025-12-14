# CI helper: run full smoke (LLM all types + batch) and summarize.
param(
    [switch]$SkipBalance,
    [switch]$SkipBatch
)

$ErrorActionPreference = 'Stop'
$repoRoot = Resolve-Path "$PSScriptRoot\.."
$logPath = "d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log"

Write-Host "=== Running smoke tests ==="
pwsh "$PSScriptRoot\run_smoke_tests.ps1" @PSBoundParameters

Write-Host "=== Parsing logs ==="
pwsh "$PSScriptRoot\parse_debug_log.ps1" -LogPath $logPath -RunId "run-smoke-2"


