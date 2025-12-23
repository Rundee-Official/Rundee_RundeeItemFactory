# Unity Editor에서 RundeeItemFactory 기능 테스트 스크립트
# Unity Editor가 실행 중이어야 합니다

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$unityProjectPath = Join-Path $repoRoot "UnityRundeeItemFactory"
$exePath = Join-Path $unityProjectPath "Assets\RundeeItemFactory\RundeeItemFactory.exe"

Write-Host "`n=== Unity Editor 기능 테스트 ===" -ForegroundColor Yellow

# .exe 파일 확인
if (-not (Test-Path $exePath)) {
    Write-Warning ".exe 파일을 찾을 수 없습니다: $exePath"
    Write-Host "빌드 및 복사를 먼저 실행하세요:" -ForegroundColor Yellow
    Write-Host "  .\scripts\build_and_test.ps1 -Configuration $Configuration" -ForegroundColor Cyan
    Write-Host "  .\scripts\copy_exe_to_unity_project.ps1 -Configuration $Configuration" -ForegroundColor Cyan
    exit 1
}

Write-Host "✓ .exe 파일 확인: $exePath" -ForegroundColor Green

# 간단한 명령줄 테스트
Write-Host "`n=== 명령줄 직접 테스트 ===" -ForegroundColor Yellow
$testDir = Join-Path $repoRoot "TempUnityTest"
if (-not (Test-Path $testDir)) { 
    New-Item -ItemType Directory -Path $testDir -Force | Out-Null 
}

$testFile = Join-Path $testDir "unity_test_food.json"
$testArgs = @(
    "--mode", "dummy",
    "--itemType", "food",
    "--count", "2",
    "--out", $testFile
)

try {
    Write-Host "테스트 실행 중..." -ForegroundColor Cyan
    $p = Start-Process -FilePath $exePath -ArgumentList $testArgs -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    
    if ($p.ExitCode -ne 0) {
        Write-Error "테스트 실패 (Exit Code: $($p.ExitCode))"
        exit 1
    }
    
    if (-not (Test-Path $testFile)) {
        Write-Error "출력 파일이 생성되지 않았습니다: $testFile"
        exit 1
    }
    
    # JSON 파싱 테스트
    $json = Get-Content $testFile -Raw | ConvertFrom-Json
    if ($json.Count -lt 2) {
        Write-Error "생성된 아이템 수가 부족합니다. 예상: 2, 실제: $($json.Count)"
        exit 1
    }
    
    Write-Host "✓ 명령줄 테스트 성공 (생성된 아이템: $($json.Count)개)" -ForegroundColor Green
    Write-Host "출력 파일: $testFile" -ForegroundColor Cyan
    
    # 첫 번째 아이템 정보 출력
    if ($json.Count -gt 0) {
        Write-Host "`n첫 번째 아이템 샘플:" -ForegroundColor Cyan
        $firstItem = $json[0]
        Write-Host "  ID: $($firstItem.id)" -ForegroundColor White
        Write-Host "  Name: $($firstItem.name)" -ForegroundColor White
    }
    
} catch {
    Write-Error "테스트 중 오류: $($_.Exception.Message)"
    exit 1
}

Write-Host "`n=== 테스트 완료 ===" -ForegroundColor Green
Write-Host "Unity Editor에서 다음을 확인하세요:" -ForegroundColor Yellow
Write-Host "  1. Window > Rundee Item Factory 메뉴 열기" -ForegroundColor Cyan
Write-Host "  2. .exe 경로가 올바르게 설정되어 있는지 확인" -ForegroundColor Cyan
Write-Host "  3. 아이템 생성 기능 테스트" -ForegroundColor Cyan
Write-Host "`n.exe 경로: $exePath" -ForegroundColor Cyan
