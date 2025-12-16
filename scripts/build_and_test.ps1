# ItemFactory 자동 빌드 및 테스트 스크립트
# 가정: Visual Studio가 설치되어 있고 MSBuild를 사용할 수 있음
# 전제조건: PowerShell 7+, Visual Studio 2019 이상, Ollama 실행 중

param(
    [string]$Configuration = "Debug",
    [string]$Platform = "x64"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$solutionPath = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory.sln"
$projectPath = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\RundeeItemFactory.vcxproj"

# MSBuild 경로 찾기
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
    # vswhere 사용
    $vswhere = "C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
    }
}

if (-not $msbuild) {
    # PATH에서 찾기
    $msbuildCmd = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuildCmd) {
        $msbuild = $msbuildCmd.Source
    }
}

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Error "MSBuild를 찾을 수 없습니다. Visual Studio가 설치되어 있는지 확인하세요."
    exit 1
}

Write-Host "MSBuild 경로: $msbuild" -ForegroundColor Cyan

# 빌드 실행
Write-Host "`n=== C++ 프로젝트 빌드 시작 ===" -ForegroundColor Yellow
Write-Host "Configuration: $Configuration, Platform: $Platform" -ForegroundColor Cyan

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
    Write-Host "✓ 빌드 성공!" -ForegroundColor Green
} catch {
    Write-Error "빌드 중 오류 발생: $($_.Exception.Message)"
    exit 1
}

# 실행 파일 경로 확인
$exePath = Join-Path $repoRoot "RundeeItemFactory\x64\$Configuration\RundeeItemFactory.exe"
if (-not (Test-Path $exePath)) {
    Write-Error "실행 파일을 찾을 수 없습니다: $exePath"
    exit 1
}

Write-Host "✓ 실행 파일 확인: $exePath" -ForegroundColor Green

# Ollama 확인
Write-Host "`n=== Ollama 연결 확인 ===" -ForegroundColor Yellow
try {
    $ping = Invoke-RestMethod -Method Get -Uri "http://localhost:11434/api/tags" -TimeoutSec 5
    Write-Host "✓ Ollama 연결 성공" -ForegroundColor Green
} catch {
    Write-Warning "Ollama가 실행 중이 아닙니다. 스모크 테스트는 LLM 모드를 건너뜁니다."
    $skipLLM = $true
}

# 간단한 테스트 실행 (dummy 모드)
Write-Host "`n=== Dummy 모드 테스트 ===" -ForegroundColor Yellow
$testDir = Join-Path $repoRoot "TempTest"
if (-not (Test-Path $testDir)) { New-Item -ItemType Directory -Path $testDir | Out-Null }

$testFile = Join-Path $testDir "test_food_dummy.json"
$testArgs = @(
    "--mode", "dummy",
    "--itemType", "food",
    "--count", "3",
    "--out", $testFile
)

try {
    $p = Start-Process -FilePath $exePath -ArgumentList $testArgs -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    if ($p.ExitCode -ne 0) {
        Write-Error "Dummy 모드 테스트 실패 (Exit Code: $($p.ExitCode))"
        exit 1
    }
    
    if (-not (Test-Path $testFile)) {
        Write-Error "출력 파일이 생성되지 않았습니다: $testFile"
        exit 1
    }
    
    # JSON 파싱 테스트
    $json = Get-Content $testFile -Raw | ConvertFrom-Json
    if ($json.Count -lt 3) {
        Write-Error "생성된 아이템 수가 부족합니다. 예상: 3, 실제: $($json.Count)"
        exit 1
    }
    
    Write-Host "✓ Dummy 모드 테스트 성공 (생성된 아이템: $($json.Count)개)" -ForegroundColor Green
} catch {
    Write-Error "Dummy 모드 테스트 중 오류: $($_.Exception.Message)"
    exit 1
}

# 밸런스 리포트 테스트
Write-Host "`n=== 밸런스 리포트 테스트 ===" -ForegroundColor Yellow
$reportArgs = @(
    "--report", $testFile,
    "--itemType", "food"
)

try {
    $p = Start-Process -FilePath $exePath -ArgumentList $reportArgs -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
    if ($p.ExitCode -ne 0) {
        Write-Warning "밸런스 리포트 테스트 실패 (Exit Code: $($p.ExitCode))"
    } else {
        Write-Host "✓ 밸런스 리포트 테스트 성공" -ForegroundColor Green
    }
} catch {
    Write-Warning "밸런스 리포트 테스트 중 오류: $($_.Exception.Message)"
}

# LLM 모드 테스트 (Ollama가 실행 중인 경우)
if (-not $skipLLM) {
    Write-Host "`n=== LLM 모드 테스트 (간단) ===" -ForegroundColor Yellow
    $llmTestFile = Join-Path $testDir "test_food_llm.json"
    $llmArgs = @(
        "--mode", "llm",
        "--itemType", "food",
        "--count", "1",
        "--model", "llama3",
        "--out", $llmTestFile
    )
    
    try {
        Write-Host "LLM 생성 중... (시간이 걸릴 수 있습니다)" -ForegroundColor Cyan
        $p = Start-Process -FilePath $exePath -ArgumentList $llmArgs -NoNewWindow -PassThru -Wait -WorkingDirectory (Split-Path $exePath)
        if ($p.ExitCode -ne 0) {
            Write-Warning "LLM 모드 테스트 실패 (Exit Code: $($p.ExitCode))"
        } else {
            if (Test-Path $llmTestFile) {
                $llmJson = Get-Content $llmTestFile -Raw | ConvertFrom-Json
                Write-Host "✓ LLM 모드 테스트 성공 (생성된 아이템: $($llmJson.Count)개)" -ForegroundColor Green
            } else {
                Write-Warning "LLM 모드 테스트: 파일이 생성되지 않았습니다"
            }
        }
    } catch {
        Write-Warning "LLM 모드 테스트 중 오류: $($_.Exception.Message)"
    }
}

Write-Host "`n=== 모든 테스트 완료 ===" -ForegroundColor Green
Write-Host "실행 파일: $exePath" -ForegroundColor Cyan
Write-Host "테스트 출력: $testDir" -ForegroundColor Cyan

return @{
    Success = $true
    ExePath = $exePath
    TestDir = $testDir
}

