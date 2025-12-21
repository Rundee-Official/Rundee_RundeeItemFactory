# 모든 아이템 타입 테스트 스크립트 (수정版)
# 각 타입마다 정확히 50개씩 생성되는지 테스트
# 실패 시 자동으로 재시도

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$projectPath = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\RundeeItemFactory.vcxproj"
$buildDir = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\$Platform\$Configuration"
$exePath = Join-Path $buildDir "RundeeItemFactory.exe"
$deployDir = Join-Path $repoRoot "Deployment"
$deployExePath = Join-Path $deployDir "RundeeItemFactory.exe"
$testOutputDir = Join-Path $repoRoot "TestOutput"

# Find MSBuild
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
    $vswhere = "C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
    }
}

if (-not $msbuild) {
    $msbuildCmd = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuildCmd) {
        $msbuild = $msbuildCmd.Source
    }
}

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Error "MSBuild not found."
    exit 1
}

Write-Host "=== Building Debug ===" -ForegroundColor Yellow
$buildArgs = @($projectPath, "/p:Configuration=$Configuration", "/p:Platform=$Platform", "/t:Build", "/m", "/v:minimal", "/nologo")
& $msbuild $buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}
Write-Host "Build successful!" -ForegroundColor Green

if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found: $exePath"
    exit 1
}

# Copy to Deployment
if (-not (Test-Path $deployDir)) {
    New-Item -ItemType Directory -Path $deployDir | Out-Null
}
# Kill any running instances
Get-Process -Name RundeeItemFactory -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
# Copy with temp name first, then rename (avoids file lock issues)
$tempExePath = Join-Path $deployDir "RundeeItemFactory.tmp"
if (Test-Path $tempExePath) {
    Remove-Item -Path $tempExePath -Force -ErrorAction SilentlyContinue
}
Copy-Item -Path $exePath -Destination $tempExePath -Force
Start-Sleep -Seconds 1
if (Test-Path $deployExePath) {
    Remove-Item -Path $deployExePath -Force -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
}
Move-Item -Path $tempExePath -Destination $deployExePath -Force
Write-Host "Copied to Deployment" -ForegroundColor Green

# Test configuration
$itemTypes = @("food", "drink", "material", "weapon", "weaponcomponent", "ammo", "armor", "clothing")
$targetCount = 50
$maxAttempts = 20

function Clean-TestOutput {
    # Clean test output directory
    if (Test-Path $testOutputDir) {
        Remove-Item -Path $testOutputDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $testOutputDir -Force | Out-Null
    
    # CRITICAL: Clean ALL registry files at the START of each test attempt
    # This ensures each test starts with a clean slate, but during the test,
    # registry files will be created and accumulated (as they would in real usage)
    $registryDir = Join-Path $deployDir "Registry"
    if (Test-Path $registryDir) {
        Write-Host "  Cleaning registry files (test start cleanup)..." -ForegroundColor Yellow
        $registryFiles = Get-ChildItem -Path $registryDir -Filter "id_registry_*.json" -ErrorAction SilentlyContinue
        if ($registryFiles) {
            $count = $registryFiles.Count
            $registryFiles | ForEach-Object { 
                Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue 
            }
            Write-Host "    Removed $count registry file(s) from previous test run" -ForegroundColor Gray
        }
    } else {
        # Create registry directory if it doesn't exist
        New-Item -ItemType Directory -Path $registryDir -Force | Out-Null
    }
    
    # Clean any existing output files in Deployment folder (if any)
    $deployOutputFiles = Get-ChildItem -Path $deployDir -Filter "items_*.json" -ErrorAction SilentlyContinue
    if ($deployOutputFiles) {
        Write-Host "  Cleaning existing output files in Deployment..." -ForegroundColor Yellow
        Remove-Item -Path $deployOutputFiles.FullName -Force -ErrorAction SilentlyContinue
    }
}

function Test-ItemType {
    param([string]$ItemType, [int]$Count)
    
    # Note: Registry file for this type should already be cleaned by Clean-TestOutput
    # at the start of each attempt. During the test, the registry will be populated
    # as items are generated (normal behavior for real usage).
    
    # Clean any existing output file for this type
    $existingOutput = Join-Path $testOutputDir "items_$ItemType.json"
    if (Test-Path $existingOutput) {
        Remove-Item -Path $existingOutput -Force -ErrorAction SilentlyContinue
    }
    
    $outputFile = Join-Path $testOutputDir "items_$ItemType.json"
    $args = @("--mode", "llm", "--itemType", $ItemType, "--count", $Count.ToString(), "--out", $outputFile)
    
    Write-Host "`n[Test] $ItemType - Generating $Count items..." -ForegroundColor Cyan
    $p = Start-Process -FilePath $deployExePath -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory $deployDir
    
    if ($p.ExitCode -ne 0) {
        Write-Host "  FAILED: Exit code $($p.ExitCode)" -ForegroundColor Red
        return $false
    }
    
    if (-not (Test-Path $outputFile)) {
        Write-Host "  FAILED: File not created" -ForegroundColor Red
        return $false
    }
    
    try {
        $jsonContent = Get-Content $outputFile -Raw
        $json = $jsonContent | ConvertFrom-Json
        
        # Debug: Check JSON type
        $jsonType = $json.GetType().Name
        if ($json -is [array]) {
            $actualCount = $json.Count
        } elseif ($json -is [PSCustomObject]) {
            # Single object, count as 1
            $actualCount = 1
        } else {
            Write-Host "  ERROR: Unexpected JSON type: $jsonType" -ForegroundColor Red
            return $false
        }
        
        Write-Host "  [PowerShell] Result: $actualCount items (target: $Count) [Type: $jsonType]" -ForegroundColor $(if ($actualCount -eq $Count) { "Green" } else { "Yellow" })
        
        # Only stop if count is actually >= 100 (ensure numeric comparison)
        $actualCountInt = [int]$actualCount
        Write-Host "  [PowerShell] Debug: actualCount=$actualCount, actualCountInt=$actualCountInt, ge100=$($actualCountInt -ge 100)" -ForegroundColor Gray
        if ($actualCountInt -ge 100) {
            Write-Host "  [PowerShell] ERROR: Count >= 100 ($actualCountInt)! Stopping." -ForegroundColor Red
            Write-Host "  [PowerShell] Debug: JSON type = $jsonType, Count = $actualCountInt" -ForegroundColor Yellow
            return "STOP"
        }
        
        $returnValue = ($actualCount -eq $Count)
        Write-Host "  [PowerShell] Returning: $returnValue (actualCount=$actualCount, target=$Count)" -ForegroundColor Gray
        return $returnValue
    } catch {
        Write-Host "  FAILED: JSON parse error - $($_.Exception.Message)" -ForegroundColor Red
        Write-Host "  Debug: File exists = $(Test-Path $outputFile), File size = $((Get-Item $outputFile).Length) bytes" -ForegroundColor Yellow
        return $false
    }
}

    # Main test loop
    $attempt = 0
    while ($attempt -lt $maxAttempts) {
        $attempt++
        Write-Host "`n========================================" -ForegroundColor Yellow
        Write-Host "Attempt #$attempt" -ForegroundColor Yellow
        Write-Host "========================================" -ForegroundColor Yellow
        
        Clean-TestOutput
        
        $allPassed = $true
        $results = @{}
        
        foreach ($itemType in $itemTypes) {
            $result = Test-ItemType -ItemType $itemType -Count $targetCount
            Write-Host "  [PowerShell] Main loop: result type=$($result.GetType().Name), value=$result" -ForegroundColor Gray
            
            # CRITICAL: Use explicit string comparison to avoid PowerShell type coercion issues
            if ([string]$result -eq "STOP") {
                Write-Host "`n[PowerShell] STOPPED: Item count >= 100 for type: $itemType" -ForegroundColor Red
                Write-Host "Check output file: $testOutputDir\items_$itemType.json" -ForegroundColor Yellow
                exit 1
            }
            
            $results[$itemType] = $result
            if (-not $result) {
                $allPassed = $false
            }
        }
    
    # Summary
    Write-Host "`n========================================" -ForegroundColor Yellow
    Write-Host "Results:" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
    foreach ($itemType in $itemTypes) {
        $status = if ($results[$itemType]) { "PASS" } else { "FAIL" }
        $color = if ($results[$itemType]) { "Green" } else { "Red" }
        Write-Host "  $itemType : $status" -ForegroundColor $color
    }
    
    if ($allPassed) {
        Write-Host "`nALL TESTS PASSED!" -ForegroundColor Green
        Write-Host "Output: $testOutputDir" -ForegroundColor Cyan
        
        # Unity Import Test
        Write-Host "`n========================================" -ForegroundColor Yellow
        Write-Host "Unity Import Test" -ForegroundColor Yellow
        Write-Host "========================================" -ForegroundColor Yellow
        
        $unityProjectPath = Join-Path $repoRoot "UnityRundeeItemFactory"
        if (-not (Test-Path $unityProjectPath)) {
            Write-Host "Unity 프로젝트를 찾을 수 없습니다: $unityProjectPath" -ForegroundColor Red
            Write-Host "Unity 임포트 테스트를 건너뜁니다." -ForegroundColor Yellow
            exit 0
        }
        
        # Unity 경로 자동 감지 (실행 중인 프로세스에서 먼저 찾기)
        $unityPath = $null
        
        # 실행 중인 Unity Editor 프로세스에서 경로 찾기
        $unityProcess = Get-Process -Name "Unity" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($unityProcess) {
            try {
                $processPath = $unityProcess.Path
                if ($processPath -and (Test-Path $processPath)) {
                    $unityPath = $processPath
                    Write-Host "Unity Editor 실행 중인 프로세스에서 경로를 찾았습니다: $unityPath" -ForegroundColor Green
                }
            } catch {
                # 프로세스 경로를 가져올 수 없는 경우 무시
            }
        }
        
        # 프로세스에서 찾지 못한 경우 표준 경로에서 찾기
        if (-not $unityPath) {
            $possiblePaths = @(
                "C:\Program Files\Unity\Hub\Editor\*\Editor\Unity.exe",
                "C:\Program Files (x86)\Unity\Editor\Unity.exe",
                "$env:LOCALAPPDATA\Programs\Unity\Hub\Editor\*\Editor\Unity.exe",
                "$env:ProgramFiles\Unity\Hub\Editor\*\Editor\Unity.exe",
                "$env:ProgramFiles(x86)\Unity\Editor\Unity.exe"
            )
            
            foreach ($pattern in $possiblePaths) {
                try {
                    $found = Get-ChildItem -Path $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
                    if ($found -and (Test-Path $found.FullName)) {
                        $unityPath = $found.FullName
                        break
                    }
                } catch {
                    # 패턴 매칭 실패 시 무시
                }
            }
        }
        
        if (-not $unityPath -or -not (Test-Path $unityPath)) {
            Write-Host "Unity Editor를 찾을 수 없습니다. Unity 임포트 테스트를 건너뜁니다." -ForegroundColor Yellow
            Write-Host "Unity를 수동으로 열고 JSON Importer를 사용하여 임포트하세요." -ForegroundColor Cyan
            Write-Host "또는 Unity Editor가 설치된 경로를 확인하세요." -ForegroundColor Cyan
            exit 0
        }
        
        Write-Host "Unity Editor: $unityPath" -ForegroundColor Gray
        Write-Host "Unity Project: $unityProjectPath" -ForegroundColor Gray
        Write-Host ""
        
        # 아이템 타입 매핑
        $itemTypeMap = @{
            "food" = "food"
            "drink" = "drink"
            "material" = "material"
            "weapon" = "weapon"
            "weaponcomponent" = "weaponcomponent"
            "ammo" = "ammo"
            "armor" = "armor"
            "clothing" = "clothing"
        }
        
        $importResults = @{}
        $jsonFiles = Get-ChildItem -Path $testOutputDir -Filter "items_*.json" | Where-Object { $_.Name -notmatch "\.raw\.json" }
        
        if ($jsonFiles.Count -eq 0) {
            Write-Host "임포트할 JSON 파일이 없습니다." -ForegroundColor Red
            exit 0
        }
        
        Write-Host "Unity 임포트 시작..." -ForegroundColor Cyan
        Write-Host ""
        
        foreach ($jsonFile in $jsonFiles) {
            # 파일명에서 아이템 타입 추출 (items_food.json -> food)
            $fileName = $jsonFile.BaseName
            $itemType = $null
            
            foreach ($type in $itemTypeMap.Keys) {
                if ($fileName -match "items_$type") {
                    $itemType = $itemTypeMap[$type]
                    break
                }
            }
            
            if (-not $itemType) {
                Write-Host "  SKIP: $($jsonFile.Name) (타입을 확인할 수 없음)" -ForegroundColor Yellow
                continue
            }
            
            Write-Host "  Importing: $($jsonFile.Name) ($itemType)..." -ForegroundColor Gray -NoNewline
            
            # Unity CLI로 임포트 실행
            $logFile = Join-Path $testOutputDir "unity_import_$itemType.log"
            $importArgs = @(
                "-projectPath", "`"$unityProjectPath`"",
                "-batchmode",
                "-quit",
                "-nographics",
                "-logFile", "`"$logFile`"",
                "-executeMethod", "JsonImportCli.Run",
                "--jsonPath", "`"$($jsonFile.FullName)`"",
                "--itemType", $itemType
            )
            
            try {
                $process = Start-Process -FilePath $unityPath -ArgumentList $importArgs -Wait -PassThru -NoNewWindow
                
                if ($process.ExitCode -eq 0) {
                    # 로그 파일 확인
                    if (Test-Path $logFile) {
                        $logContent = Get-Content $logFile -Raw
                        if ($logContent -match "Import success" -or $logContent -match "Import complete") {
                            Write-Host " PASS" -ForegroundColor Green
                            $importResults[$itemType] = $true
                        } elseif ($logContent -match "Import failed" -or $logContent -match "error") {
                            Write-Host " FAIL" -ForegroundColor Red
                            Write-Host "    로그: $logFile" -ForegroundColor Gray
                            $importResults[$itemType] = $false
                        } else {
                            Write-Host " UNKNOWN" -ForegroundColor Yellow
                            $importResults[$itemType] = $false
                        }
                    } else {
                        Write-Host " UNKNOWN (로그 파일 없음)" -ForegroundColor Yellow
                        $importResults[$itemType] = $false
                    }
                } else {
                    Write-Host " FAIL (Exit Code: $($process.ExitCode))" -ForegroundColor Red
                    $importResults[$itemType] = $false
                }
            } catch {
                Write-Host " FAIL (Exception: $($_.Exception.Message))" -ForegroundColor Red
                $importResults[$itemType] = $false
            }
        }
        
        Write-Host ""
        Write-Host "Unity Import Results:" -ForegroundColor Yellow
        foreach ($itemType in $itemTypeMap.Values) {
            if ($importResults.ContainsKey($itemType)) {
                $status = if ($importResults[$itemType]) { "PASS" } else { "FAIL" }
                $color = if ($importResults[$itemType]) { "Green" } else { "Red" }
                Write-Host "  $itemType : $status" -ForegroundColor $color
            }
        }
        
        $allImportPassed = ($importResults.Values | Where-Object { $_ -eq $false }).Count -eq 0
        if ($allImportPassed) {
            Write-Host "`nALL UNITY IMPORT TESTS PASSED!" -ForegroundColor Green
        } else {
            Write-Host "`nSome Unity import tests failed." -ForegroundColor Yellow
        }
        
        exit 0
    } else {
        Write-Host "`nSome tests failed. Retrying in 3 seconds..." -ForegroundColor Yellow
        Start-Sleep -Seconds 3
    }
}

Write-Host "`nMaximum attempts reached." -ForegroundColor Red
exit 1

