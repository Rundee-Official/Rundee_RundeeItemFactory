# 배포 패키지 준비 스크립트
# 가정: 프로젝트가 빌드되어 있고 필요한 파일들이 존재
# 전제조건: PowerShell 7+, 빌드 완료

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [string]$OutputDir = "Deployment"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$buildDir = Join-Path $repoRoot "RundeeItemFactory\$Platform\$Configuration"
$deployDir = Join-Path $repoRoot $OutputDir

Write-Host "=== 배포 패키지 준비 시작 ===" -ForegroundColor Yellow

# 배포 디렉토리 생성
if (Test-Path $deployDir) {
    Remove-Item -Path $deployDir -Recurse -Force
}
New-Item -ItemType Directory -Path $deployDir | Out-Null

$packageInfo = @{
    Success = $false
    Files = @()
    MissingFiles = @()
    Errors = @()
}

# 1. 실행 파일 확인
$exePath = Join-Path $buildDir "RundeeItemFactory.exe"
if (Test-Path $exePath) {
    Copy-Item -Path $exePath -Destination $deployDir
    $packageInfo.Files += "RundeeItemFactory.exe"
    Write-Host "✓ 실행 파일 복사 완료" -ForegroundColor Green
} else {
    $packageInfo.MissingFiles += "RundeeItemFactory.exe"
    Write-Host "✗ 실행 파일을 찾을 수 없습니다: $exePath" -ForegroundColor Red
}

# 2. config 폴더 확인
$configSource = Join-Path $buildDir "config"
$configDest = Join-Path $deployDir "config"
if (Test-Path $configSource) {
    Copy-Item -Path $configSource -Destination $configDest -Recurse
    $configFiles = Get-ChildItem -Path $configSource -File
    foreach ($file in $configFiles) {
        $packageInfo.Files += "config/$($file.Name)"
    }
    Write-Host "✓ config 폴더 복사 완료 ($($configFiles.Count)개 파일)" -ForegroundColor Green
} else {
    $packageInfo.MissingFiles += "config/"
    Write-Host "⚠ config 폴더를 찾을 수 없습니다: $configSource" -ForegroundColor Yellow
}

# 4. 문서 파일 복사
$docsToCopy = @(
    @{ Source = "README.md"; Required = $true },
    @{ Source = "LICENSE"; Required = $false },
    @{ Source = "CHANGELOG.md"; Required = $false }
)

foreach ($doc in $docsToCopy) {
    $sourcePath = Join-Path $repoRoot $doc.Source
    if (Test-Path $sourcePath) {
        Copy-Item -Path $sourcePath -Destination $deployDir
        $packageInfo.Files += $doc.Source
        Write-Host "✓ $($doc.Source) 복사 완료" -ForegroundColor Green
    } else {
        if ($doc.Required) {
            $packageInfo.MissingFiles += $doc.Source
            Write-Host "✗ 필수 문서를 찾을 수 없습니다: $($doc.Source)" -ForegroundColor Red
        } else {
            Write-Host "⚠ 선택 문서를 찾을 수 없습니다: $($doc.Source)" -ForegroundColor Yellow
        }
    }
}

# 5. 배포 체크리스트 생성
$checklistPath = Join-Path $deployDir "DEPLOYMENT_CHECKLIST.md"
$checklistContent = @"
# 배포 체크리스트

## 필수 파일 확인

### 실행 파일
- [ ] RundeeItemFactory.exe 존재

### 설정 파일
- [ ] config/rundee_config.json 존재 (선택사항)

### 문서
- [ ] README.md 존재

## 의존성 확인

- [ ] Ollama 설치 및 실행 중
- [ ] Visual C++ Redistributable 설치 (필요한 경우)

## 테스트

- [ ] 실행 파일이 정상적으로 실행되는지 확인
- [ ] Ollama 연결 테스트
- [ ] 샘플 아이템 생성 테스트

## 배포 정보

- 빌드 구성: $Configuration
- 플랫폼: $Platform
- 패키지 준비 일시: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
- 파일 수: $($packageInfo.Files.Count)
"@

Set-Content -Path $checklistPath -Value $checklistContent
$packageInfo.Files += "DEPLOYMENT_CHECKLIST.md"
Write-Host "✓ 배포 체크리스트 생성 완료" -ForegroundColor Green

# 결과 요약
Write-Host "`n=== 배포 패키지 준비 결과 ===" -ForegroundColor Yellow
Write-Host "배포 디렉토리: $deployDir" -ForegroundColor Cyan
Write-Host "복사된 파일 수: $($packageInfo.Files.Count)" -ForegroundColor Cyan

if ($packageInfo.MissingFiles.Count -eq 0) {
    Write-Host "✓ 모든 필수 파일이 준비되었습니다!" -ForegroundColor Green
    $packageInfo.Success = $true
} else {
    Write-Host "⚠ 누락된 파일: $($packageInfo.MissingFiles.Count)개" -ForegroundColor Yellow
    foreach ($missing in $packageInfo.MissingFiles) {
        Write-Host "  - $missing" -ForegroundColor Yellow
    }
    $packageInfo.Success = $false
}

return $packageInfo




