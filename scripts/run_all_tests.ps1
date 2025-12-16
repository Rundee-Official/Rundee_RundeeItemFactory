# Integrated test script
# Comprehensive test suite including stress test, balance report, and Unity import

param(
    [string]$DeploymentDir = "Deployment",
    [int]$ItemCountPerType = 50,
    [string]$Model = "llama3",
    [switch]$SkipUnity,
    [switch]$SkipBalanceReport,
    [switch]$CleanTempFiles
)

$ErrorActionPreference = 'Stop'
$repoRoot = Resolve-Path "$PSScriptRoot\.."

Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  Integrated Test Suite" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# 1. Clean temporary files
if ($CleanTempFiles) {
    Write-Host "[1/4] Cleaning Temporary Files" -ForegroundColor Cyan
    
    # Delete FinalTest/*.raw.json
    $rawFiles = Get-ChildItem -Path (Join-Path $repoRoot "FinalTest") -Filter "*.raw.json" -ErrorAction SilentlyContinue
    if ($rawFiles) {
        $rawFiles | Remove-Item -Force
        Write-Host "  ✓ Deleted $($rawFiles.Count) .raw.json files" -ForegroundColor Green
    }
    
    # Delete Deployment/test_*.json.raw.json
    $deployRawFiles = Get-ChildItem -Path (Join-Path $repoRoot $DeploymentDir) -Filter "*.raw.json" -ErrorAction SilentlyContinue
    if ($deployRawFiles) {
        $deployRawFiles | Remove-Item -Force
        Write-Host "  ✓ Deleted $($deployRawFiles.Count) deployment directory .raw.json files" -ForegroundColor Green
    }
} else {
    Write-Host "[1/4] Cleaning Temporary Files (Skipped)" -ForegroundColor Gray
}

# 2. Run stress test
Write-Host "`n[2/4] Running Stress Test" -ForegroundColor Cyan
& "$PSScriptRoot\run_final_release_test.ps1" -ItemCountPerType $ItemCountPerType -Model $Model -SkipUnity -SkipBalanceReport:$SkipBalanceReport

if ($LASTEXITCODE -ne 0) {
    Write-Host "`n✗ Stress test failed" -ForegroundColor Red
    exit 1
}

# 3. Balance report verification
if (-not $SkipBalanceReport) {
    Write-Host "`n[3/4] Balance Report Verification" -ForegroundColor Cyan
    $testDir = Join-Path $repoRoot "FinalTest"
    $itemTypes = @("food", "drink", "material", "weapon", "weaponcomponent", "ammo")
    # Prefer built executable
    $exePath = $null
    $buildPath1 = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\x64\Release\RundeeItemFactory.exe"
    $buildPath2 = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\x64\Debug\RundeeItemFactory.exe"
    $deployPath = Join-Path $repoRoot "$DeploymentDir\RundeeItemFactory.exe"
    
    if (Test-Path $buildPath1) {
        $exePath = (Resolve-Path $buildPath1).Path
    } elseif (Test-Path $buildPath2) {
        $exePath = (Resolve-Path $buildPath2).Path
    } elseif (Test-Path $deployPath) {
        $exePath = (Resolve-Path $deployPath).Path
    } else {
        Write-Error "Executable file not found."
        exit 1
    }

    foreach ($itemType in $itemTypes) {
        $jsonFile = Join-Path $testDir "final_test_${itemType}.json"
        if (Test-Path $jsonFile) {
            Write-Host "  Generating $itemType balance report..." -ForegroundColor Yellow
            $args = @("--report", $jsonFile, "--itemType", $itemType)
            $output = & $exePath $args 2>&1 | Out-String
            if ($LASTEXITCODE -eq 0 -or $output -match "Balance" -or $output -match "Total Items") {
                Write-Host "    ✓ $itemType balance report generated successfully" -ForegroundColor Green
            } else {
                Write-Host "    ✗ $itemType balance report generation failed" -ForegroundColor Red
            }
        }
    }
} else {
    Write-Host "`n[3/4] Balance Report Verification (Skipped)" -ForegroundColor Gray
}

# 4. Unity import test
if (-not $SkipUnity) {
    Write-Host "`n[4/4] Unity Import Test" -ForegroundColor Cyan
    Write-Host "  Please run the following menu in Unity Editor:" -ForegroundColor Yellow
    Write-Host "    Tools > Rundee > Item Factory > [Debug] > Import All Test Items" -ForegroundColor White
    Write-Host "  Or you can run it automatically via Unity MCP." -ForegroundColor Yellow
} else {
    Write-Host "`n[4/4] Unity Import Test (Skipped)" -ForegroundColor Gray
}

Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  Integrated Test Suite Complete" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow

