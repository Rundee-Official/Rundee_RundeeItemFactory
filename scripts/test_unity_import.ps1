# Unity 임포트 테스트 스크립트
# 생성된 JSON 파일들을 Unity로 임포트하는 테스트

param(
    [string]$UnityPath = "",
    [string]$ProjectPath = ""
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$testOutputDir = Join-Path $repoRoot "TestOutput"

# Unity 경로 자동 감지
if ([string]::IsNullOrEmpty($UnityPath)) {
    $possiblePaths = @(
        "C:\Program Files\Unity\Hub\Editor\*\Editor\Unity.exe",
        "C:\Program Files (x86)\Unity\Editor\Unity.exe",
        "$env:LOCALAPPDATA\Programs\Unity\Hub\Editor\*\Editor\Unity.exe"
    )
    
    foreach ($pattern in $possiblePaths) {
        $found = Get-ChildItem -Path $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $UnityPath = $found.FullName
            break
        }
    }
}

if ([string]::IsNullOrEmpty($UnityPath) -or -not (Test-Path $UnityPath)) {
    Write-Host "Unity 경로를 찾을 수 없습니다. UnityPath 매개변수를 지정하세요." -ForegroundColor Red
    exit 1
}

# 프로젝트 경로
if ([string]::IsNullOrEmpty($ProjectPath)) {
    $ProjectPath = Join-Path $repoRoot "UnityRundeeItemFactory"
}

if (-not (Test-Path $ProjectPath)) {
    Write-Host "Unity 프로젝트 경로를 찾을 수 없습니다: $ProjectPath" -ForegroundColor Red
    exit 1
}

Write-Host "=== Unity 임포트 테스트 ===" -ForegroundColor Cyan
Write-Host "Unity: $UnityPath" -ForegroundColor Yellow
Write-Host "Project: $ProjectPath" -ForegroundColor Yellow
Write-Host ""

# JSON 파일 확인
$jsonFiles = Get-ChildItem -Path $testOutputDir -Filter "items_*.json" | Where-Object { $_.Name -notmatch "\.raw\.json" }

if ($jsonFiles.Count -eq 0) {
    Write-Host "임포트할 JSON 파일이 없습니다. 먼저 아이템을 생성하세요." -ForegroundColor Red
    exit 1
}

Write-Host "임포트할 파일:" -ForegroundColor Yellow
foreach ($file in $jsonFiles) {
    $content = Get-Content $file.FullName -Raw | ConvertFrom-Json
    Write-Host "  $($file.Name) : $($content.Count) items" -ForegroundColor Green
}

Write-Host ""
Write-Host "Unity Editor에서 다음 단계를 수행하세요:" -ForegroundColor Cyan
Write-Host "1. Tools > Rundee > Item Factory > JSON Importer 열기" -ForegroundColor White
Write-Host "2. 각 JSON 파일을 선택하고 해당 Item Type을 선택" -ForegroundColor White
Write-Host "3. 'Import Selected JSON' 버튼 클릭" -ForegroundColor White
Write-Host ""
Write-Host "또는 Unity를 열고 자동으로 임포트할 수 있습니다:" -ForegroundColor Cyan

# Unity 프로젝트 열기
Write-Host "Unity 프로젝트를 여는 중..." -ForegroundColor Yellow
Start-Process -FilePath $UnityPath -ArgumentList "-projectPath", "`"$ProjectPath`"" -NoNewWindow

Write-Host ""
Write-Host "Unity가 열리면 JSON Importer를 사용하여 파일들을 임포트하세요." -ForegroundColor Green



