Param(
    [string]$Model = "llama3",
    [string]$Batch = "food:100,drink:100,material:100,weapon:100,weaponcomponent:100,ammo:100",
    [string]$ExePath = "D:\_VisualStudioProjects\_Rundee_RundeeItemFactory\RundeeItemFactory\x64\Release\RundeeItemFactory.exe",
    [switch]$Quiet
)

# Simple bulk generator (batch only), same logging style as run_smoke_tests (sync stdout/err).
# NOTE: In debug mode, clear d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log manually before run.

$Model   = $Model.Trim("'`"")
$Batch   = $Batch.Trim("'`"")
$ExePath = $ExePath.Trim("'`"")

if ([string]::IsNullOrWhiteSpace($Model)) { $Model = "llama3" }
if ([string]::IsNullOrWhiteSpace($Batch)) { $Batch = "food:100,drink:100,material:100,weapon:100,weaponcomponent:100,ammo:100" }

function Write-Info($msg) { if (-not $Quiet) { Write-Host "[BulkSmoke] $msg" } }

if (-not (Test-Path $ExePath)) {
    Write-Error "Executable not found: $ExePath"
    exit 1
}

$cmd = @(
    "--mode", "batch",
    "--batch", $Batch,
    "--model", $Model,
    "--preset", "default"
)

Write-Info "Exe: $ExePath"
Write-Info "Batch: $Batch"
Write-Info "Model: $Model"

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $ExePath
foreach ($c in $cmd) { [void]$psi.ArgumentList.Add($c) }
$psi.WorkingDirectory = Split-Path $ExePath
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true

$p = New-Object System.Diagnostics.Process
$p.StartInfo = $psi
$null = $p.Start()
$stdOut = $p.StandardOutput.ReadToEnd()
$stdErr = $p.StandardError.ReadToEnd()
$p.WaitForExit()

if ($stdOut -and -not $Quiet) {
    $stdOut.TrimEnd("`r", "`n").Split("`n") | ForEach-Object { Write-Host "[exe][stdout] $_" }
}
if ($stdErr) {
    $stdErr.TrimEnd("`r", "`n").Split("`n") | ForEach-Object { Write-Host "[exe][stderr] $_" }
}

Write-Info "Exit code: $($p.ExitCode)"
if ($p.ExitCode -ne 0) { exit $p.ExitCode }


