# 모든 아이템 타입 테스트 스크립트
# 각 타입마다 정확히 50개씩 생성되는지 테스트
# 실패 시 자동으로 재시도

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$solutionPath = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory.sln"
$projectPath = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\RundeeItemFactory.vcxproj"
# Build output is in RundeeItemFactory\RundeeItemFactory\x64\Debug, not RundeeItemFactory\x64\Debug
$buildDir = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\$Platform\$Configuration"
$exePath = Join-Path $buildDir "RundeeItemFactory.exe"
$deployDir = Join-Path $repoRoot "Deployment"
$deployExePath = Join-Path $deployDir "RundeeItemFactory.exe"
$testOutputDir = Join-Path $repoRoot "TestOutput"

# Find MSBuild path
$msbuildPaths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
)

$msbuild = $msbuildPaths | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $msbuild) {
    # Try vswhere
    $vswhere = "C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
    }
}

if (-not $msbuild) {
    # Try PATH
    $msbuildCmd = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuildCmd) {
        $msbuild = $msbuildCmd.Source
    }
}

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Error "MSBuild not found. Please install Visual Studio or Build Tools."
    exit 1
}

Write-Host "MSBuild path: $msbuild" -ForegroundColor Cyan

Write-Host "=== Starting Debug Build ===" -ForegroundColor Yellow

# 빌드
$buildArgs = @(
    $projectPath,
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/t:Build",
    "/m",
    "/v:minimal",
    "/nologo"
)

try {
    & $msbuild $buildArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error "빌드 실패 (Exit Code: $LASTEXITCODE)"
        exit 1
    }
    Write-Host "Build successful!" -ForegroundColor Green
} catch {
    Write-Error "Build error: $($_.Exception.Message)"
    exit 1
}

if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found: $exePath"
    exit 1
}

# Copy to Deployment folder
Write-Host "`n=== Copying to Deployment folder ===" -ForegroundColor Yellow
if (-not (Test-Path $deployDir)) {
    New-Item -ItemType Directory -Path $deployDir | Out-Null
}
Copy-Item -Path $exePath -Destination $deployExePath -Force
Write-Host "Copy completed: $deployExePath" -ForegroundColor Green

# Item types to test
$itemTypes = @("food", "drink", "material", "weapon", "weaponcomponent", "ammo")
$targetCount = 50
$maxAttempts = 10
$attempt = 0

function Clean-TestOutput {
    if (Test-Path $testOutputDir) {
        Remove-Item -Path $testOutputDir -Recurse -Force
        Write-Host "Test output folder cleaned" -ForegroundColor Cyan
    }
    New-Item -ItemType Directory -Path $testOutputDir -Force | Out-Null
}

function Test-ItemType {
    param(
        [string]$ItemType,
        [int]$Count
    )
    
    $outputFile = Join-Path $testOutputDir "items_$ItemType.json"
    $args = @(
        "--mode", "llm",
        "--itemType", $ItemType,
        "--count", $Count.ToString(),
        "--out", $outputFile
    )
    
    Write-Host "`n[Test] Generating $Count items of type $ItemType..." -ForegroundColor Cyan
    $p = Start-Process -FilePath $deployExePath -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory $deployDir
    
    if ($p.ExitCode -ne 0) {
        Write-Host "Execution failed (Exit Code: $($p.ExitCode))" -ForegroundColor Red
        return $false
    }
    
    if (-not (Test-Path $outputFile)) {
        Write-Host "Output file not created" -ForegroundColor Red
        return $false
    }
    
    try {
        $jsonContent = Get-Content $outputFile -Raw
        $json = $jsonContent | ConvertFrom-Json
        
        # Handle both array and single object
        if ($json -is [array]) {
            $actualCount = $json.Count
        } elseif ($json -is [PSCustomObject]) {
            $actualCount = 1
        } else {
            Write-Host "Unexpected JSON format" -ForegroundColor Red
            return $false
        }
        
        Write-Host "  Generated items: $actualCount (target: $Count)" -ForegroundColor $(if ($actualCount -eq $Count) { "Green" } else { "Yellow" })
        
        # Stop if count >= 100
        if ($actualCount -ge 100) {
            Write-Host "`nWARNING: Item count is 100 or more ($actualCount)!" -ForegroundColor Red
            Write-Host "Stopping test and entering debug mode." -ForegroundColor Red
            Write-Host "Output file: $outputFile" -ForegroundColor Yellow
            return "STOP"
        }
        
        if ($actualCount -eq $Count) {
            Write-Host "SUCCESS: Exactly $Count items generated" -ForegroundColor Green
            return $true
        } else {
            Write-Host "FAILED: $actualCount items generated (target: $Count)" -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "JSON parsing failed: $($_.Exception.Message)" -ForegroundColor Red
        return $false
    }
}

# Main test loop
while ($attempt -lt $maxAttempts) {
    $attempt++
    Write-Host "`n========================================" -ForegroundColor Yellow
    Write-Host "Test Attempt #$attempt" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    
    Clean-TestOutput
    
    $allPassed = $true
    $results = @{}
    
    foreach ($itemType in $itemTypes) {
        $result = Test-ItemType -ItemType $itemType -Count $targetCount
        
        if ($result -eq "STOP") {
            Write-Host "`nTest stopped: Item count is 100 or more." -ForegroundColor Red
            Write-Host "Check output files for debugging: $testOutputDir" -ForegroundColor Yellow
            exit 1
        }
        
        $results[$itemType] = $result
        
        if (-not $result) {
            $allPassed = $false
        }
    }
    
    # Summary
    Write-Host "`n========================================" -ForegroundColor Yellow
    Write-Host "Test Results Summary" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    
    foreach ($itemType in $itemTypes) {
        $status = if ($results[$itemType]) { "PASS" } else { "FAIL" }
        $color = if ($results[$itemType]) { "Green" } else { "Red" }
        Write-Host "$itemType : $status" -ForegroundColor $color
    }
    
    if ($allPassed) {
        Write-Host "`nAll tests passed!" -ForegroundColor Green
        Write-Host "Output folder: $testOutputDir" -ForegroundColor Cyan
        exit 0
    } else {
        Write-Host "`nSome tests failed. Retrying..." -ForegroundColor Yellow
        Start-Sleep -Seconds 2
    }
}

Write-Host "`nMaximum attempts ($maxAttempts) reached." -ForegroundColor Red
exit 1

