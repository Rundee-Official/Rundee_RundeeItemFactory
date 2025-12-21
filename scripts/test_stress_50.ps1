# 스트레스 테스트 스크립트 - 각 타입 50개씩 생성
# 정확히 50개가 생성되는지, 레지스트리 파일이 생성되는지 확인

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$deployDir = Join-Path $repoRoot "Deployment"
$deployExePath = Join-Path $deployDir "RundeeItemFactory.exe"
$testOutputDir = Join-Path $repoRoot "TestOutput"
$registryDir = Join-Path $deployDir "Registry"

# 모든 아이템 타입 (medicine 포함)
$itemTypes = @("food", "drink", "medicine", "material", "weapon", "weaponcomponent", "ammo", "armor", "clothing")

# 테스트 결과
$results = @{}
$registryResults = @{}

Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "스트레스 테스트: 각 타입 50개씩 생성" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

# 실행 파일 확인
if (-not (Test-Path $deployExePath)) {
    Write-Host "실행 파일을 찾을 수 없습니다: $deployExePath" -ForegroundColor Red
    exit 1
}

Write-Host "실행 파일: $deployExePath" -ForegroundColor Gray
Write-Host "출력 디렉토리: $testOutputDir" -ForegroundColor Gray
Write-Host "레지스트리 디렉토리: $registryDir" -ForegroundColor Gray
Write-Host ""

# 각 타입별 테스트
foreach ($itemType in $itemTypes) {
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "[$itemType] 50개 생성 테스트 시작" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    
    $outputFile = Join-Path $testOutputDir "items_$itemType.json"
    $registryFile = Join-Path $registryDir "id_registry_$itemType.json"
    
    # 기존 파일 삭제
    if (Test-Path $outputFile) {
        Remove-Item $outputFile -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path $registryFile) {
        Remove-Item $registryFile -Force -ErrorAction SilentlyContinue
    }
    
    Write-Host "생성 중: $itemType (50개)..." -ForegroundColor Yellow
    
    # 실행 (WorkingDirectory를 Deployment로 설정하여 레지스트리가 Deployment\Registry에 생성되도록)
    $startTime = Get-Date
    $process = Start-Process -FilePath $deployExePath -ArgumentList @(
        "--mode", "llm",
        "--itemType", $itemType,
        "--count", "50",
        "--out", $outputFile,
        "--model", "llama3"
    ) -Wait -PassThru -NoNewWindow -WorkingDirectory $deployDir
    
    $endTime = Get-Date
    $duration = ($endTime - $startTime).TotalSeconds
    
    if ($process.ExitCode -ne 0) {
        Write-Host "  실패: Exit Code $($process.ExitCode)" -ForegroundColor Red
        $results[$itemType] = @{
            success = $false
            count = 0
            exitCode = $process.ExitCode
            duration = $duration
        }
        continue
    }
    
    # 결과 확인
    if (-not (Test-Path $outputFile)) {
        Write-Host "  실패: 출력 파일이 생성되지 않음" -ForegroundColor Red
        $results[$itemType] = @{
            success = $false
            count = 0
            exitCode = 0
            duration = $duration
        }
        continue
    }
    
    # JSON 파싱
    try {
        $jsonContent = Get-Content $outputFile -Raw | ConvertFrom-Json
        $actualCount = if ($jsonContent -is [System.Array]) { $jsonContent.Count } else { $jsonContent.items.Count }
        
        Write-Host "  생성된 아이템 수: $actualCount" -ForegroundColor $(if ($actualCount -eq 50) { "Green" } else { "Yellow" })
        Write-Host "  소요 시간: $([math]::Round($duration, 2))초" -ForegroundColor Gray
        
        if ($actualCount -eq 50) {
            Write-Host "  PASS: 정확히 50개 생성됨" -ForegroundColor Green
            $results[$itemType] = @{
                success = $true
                count = $actualCount
                exitCode = 0
                duration = $duration
            }
        } elseif ($actualCount -gt 50) {
            Write-Host "  FAIL: $actualCount 개 생성됨 (50개 초과)" -ForegroundColor Red
            $results[$itemType] = @{
                success = $false
                count = $actualCount
                exitCode = 0
                duration = $duration
            }
        } else {
            Write-Host "  FAIL: $actualCount 개만 생성됨 (50개 미만)" -ForegroundColor Red
            $results[$itemType] = @{
                success = $false
                count = $actualCount
                exitCode = 0
                duration = $duration
            }
        }
        
        # 레지스트리 파일 확인
        if (Test-Path $registryFile) {
            try {
                $registryContent = Get-Content $registryFile -Raw | ConvertFrom-Json
                $registryCount = if ($registryContent.ids) { $registryContent.ids.Count } else { 0 }
                Write-Host "  레지스트리: $registryCount 개 ID 저장됨" -ForegroundColor Green
                $registryResults[$itemType] = $true
            } catch {
                Write-Host "  레지스트리: 파일 읽기 실패" -ForegroundColor Yellow
                $registryResults[$itemType] = $false
            }
        } else {
            Write-Host "  레지스트리: 파일이 생성되지 않음" -ForegroundColor Red
            $registryResults[$itemType] = $false
        }
        
    } catch {
        Write-Host "  실패: JSON 파싱 오류 - $($_.Exception.Message)" -ForegroundColor Red
        $results[$itemType] = @{
            success = $false
            count = 0
            exitCode = 0
            duration = $duration
            error = $_.Exception.Message
        }
    }
}

# 결과 요약
Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "테스트 결과 요약" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host ""

$allPassed = $true
$totalDuration = 0

foreach ($itemType in $itemTypes) {
    $result = $results[$itemType]
    $registryOk = $registryResults[$itemType]
    
    if ($result.success -and $registryOk) {
        $status = "PASS"
        $color = "Green"
    } else {
        $status = "FAIL"
        $color = "Red"
        $allPassed = $false
    }
    
    $count = $result.count
    $duration = [math]::Round($result.duration, 2)
    $totalDuration += $result.duration
    
    Write-Host "  $itemType : $status (Count: $count, Duration: ${duration}s, Registry: $(if ($registryOk) { 'OK' } else { 'FAIL' }))" -ForegroundColor $color
}

Write-Host ""
Write-Host "총 소요 시간: $([math]::Round($totalDuration, 2))초" -ForegroundColor Cyan
Write-Host ""

if ($allPassed) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "모든 테스트 통과!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    exit 0
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "일부 테스트 실패" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
}

