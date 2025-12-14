param(
    [string]$LogPath = "d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log",
    [string]$RunId = "run-smoke-2"
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $LogPath)) {
    Write-Error "Log file not found: $LogPath"
}

$lines = Get-Content $LogPath | Where-Object { $_ -match $RunId }

if (-not $lines) {
    Write-Warning "No entries found for runId=$RunId"
    return
}

$json = $lines | ForEach-Object { $_ | ConvertFrom-Json }

function Summarize($entries, $location) {
    $filtered = $entries | Where-Object { $_.location -eq $location }
    if ($filtered) {
        Write-Host "$location"
        $filtered | ForEach-Object { Write-Host "  " $_.message }
    }
}

Summarize $json "ollama_ping"
Summarize $json "run_llm:exit"
Summarize $json "run_report:exit"
Summarize $json "run_batch:exit"
Summarize $json "smoke:done"


