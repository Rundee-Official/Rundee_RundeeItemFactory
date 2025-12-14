# Requires: PowerShell 7+
# Smoke tests for RundeeItemFactory CLI (LLM mode).
# Requires Ollama running and prompts available.
# - Finds RundeeItemFactory.exe in standard locations.
# - Runs LLM generation for all item types (Food/Drink/Material/Weapon/WeaponComponent/Ammo) with small counts.
# - Validates generated JSON is parseable.
# - Optionally runs a balance report on the first file.
# Logs: NDJSON appended to d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log

param(
    [switch]$SkipBalance,
    [switch]$SkipBatch,
    [switch]$UnityImport
)

$ErrorActionPreference = 'Stop'

$sessionId = "debug-session"
$runId = "run-smoke-2"
$logPath = "d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log"

function Write-AgentLog {
    param(
        [string]$HypothesisId,
        [string]$Location,
        [string]$Message
    )
    $payload = @{
        sessionId    = $sessionId
        runId        = $runId
        hypothesisId = $HypothesisId
        location     = $Location
        message      = $Message
        timestamp    = [int64]([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())
    } | ConvertTo-Json -Compress
    Add-Content -Path $logPath -Value $payload
}

# Discover executable
$repoRoot = Resolve-Path "$PSScriptRoot\.."
$unityProject = Join-Path $repoRoot "UnityRundeeItemFactory"
$unityAssets = Join-Path $unityProject "Assets"
$unityImporterExe = Join-Path $unityProject "RundeeImporterStub.exe"  # placeholder if we add a stub
$possiblePaths = @(
    (Join-Path $repoRoot "RundeeItemFactory\x64\Debug\RundeeItemFactory.exe")
    (Join-Path $repoRoot "RundeeItemFactory\x64\Release\RundeeItemFactory.exe")
    (Join-Path $repoRoot "..\RundeeItemFactory\x64\Debug\RundeeItemFactory.exe")
    (Join-Path $repoRoot "..\RundeeItemFactory\x64\Release\RundeeItemFactory.exe")
)

$exePath = $possiblePaths | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $exePath) {
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "discover_exe" -Message "exe_not_found"
    Write-Error "RundeeItemFactory.exe not found in standard locations. Build the C++ project first."
}
Write-AgentLog -HypothesisId "H_SMOKE" -Location "discover_exe" -Message "exe=$exePath"

# Ping Ollama
try {
    $ping = Invoke-RestMethod -Method Get -Uri "http://localhost:11434/api/tags" -TimeoutSec 5
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "ollama_ping" -Message "ok"
} catch {
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "ollama_ping" -Message "failed"
    Write-Error "Ollama HTTP API not reachable at http://localhost:11434. Start ollama first."
}

$outDir = Join-Path $repoRoot "TempSmoke"
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

$tests = @(
    # Keep counts small to reduce LLM time
    @{ itemType="food"; out="items_food.json" },
    @{ itemType="drink"; out="items_drink.json" },
    @{ itemType="material"; out="items_material.json" },
    @{ itemType="weapon"; out="items_weapon.json" },
    @{ itemType="weaponcomponent"; out="items_weaponcomponent.json" },
    @{ itemType="ammo"; out="items_ammo.json" }
)

$firstJson = $null

foreach ($t in $tests) {
    $outFile = Join-Path $outDir $t.out
    $args = @(
        "--mode", "llm",
        "--itemType", $t.itemType,
        "--count", "2",
        "--out", $outFile
    )
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_llm:start" -Message "$($t.itemType)"
    $p = Start-Process -FilePath "$exePath" -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_llm:exit" -Message "$($t.itemType):$($p.ExitCode)"

    if ($p.ExitCode -ne 0) {
        Write-Error "LLM generation failed for $($t.itemType) (exit $($p.ExitCode))"
    }

    if (-not (Test-Path $outFile)) {
        Write-Error "Output not found: $outFile"
    }

    try {
        Get-Content $outFile -Raw | ConvertFrom-Json | Out-Null
    }
    catch {
        $msg = $_.Exception.Message
        Write-Error ("JSON parse failed for {0}: {1}" -f $outFile, $msg)
    }

    if (-not $firstJson) { $firstJson = $outFile }
}

if (-not $SkipBalance -and $firstJson) {
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_report:start" -Message $firstJson
    $args = @(
        "--report", $firstJson,
        "--itemType", "food"
    )
    $p = Start-Process -FilePath "$exePath" -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_report:exit" -Message "exit=$($p.ExitCode)"
    if ($p.ExitCode -ne 0) {
        Write-Error "Balance report failed (exit $($p.ExitCode))"
    }
}

# Batch smoke (optional)
if (-not $SkipBatch) {
    $batchFood = Join-Path $outDir "batch_food.json"
    $batchDrink = Join-Path $outDir "batch_drink.json"
    $batchMaterial = Join-Path $outDir "batch_material.json"
    $batchStr = "food:1:$batchFood,drink:1:$batchDrink,material:1:$batchMaterial"
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_batch:start" -Message $batchStr
    $args = @(
        "--mode", "batch",
        "--batch", $batchStr,
        "--preset", "default",
        "--model", "llama3"
    )
    $p = Start-Process -FilePath "$exePath" -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "run_batch:exit" -Message "exit=$($p.ExitCode)"
    if ($p.ExitCode -ne 0) {
        Write-Error "Batch generation failed (exit $($p.ExitCode))"
    }
}

Write-AgentLog -HypothesisId "H_SMOKE" -Location "smoke:done" -Message "ok"
Write-Host "Smoke tests completed. Outputs: $outDir"

# Unity import stub hook (future)
if ($UnityImport) {
    Write-Host "Unity import step is not automated in this script. Please import JSONs via Unity JSON Importer."
    Write-AgentLog -HypothesisId "H_SMOKE" -Location "unity_import" -Message "manual_step_required"
}

