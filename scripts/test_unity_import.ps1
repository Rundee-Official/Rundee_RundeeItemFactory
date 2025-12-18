# ===============================
# Test Unity Import Script
# ===============================

$repoRoot = "D:\_VisualStudioProjects\_Rundee_RundeeItemFactory"
$testDir = Join-Path $repoRoot "RundeeItemFactory\x64\Release\FullTest_30Items"
$unityProjectPath = Join-Path $repoRoot "UnityRundeeItemFactory"

Write-Host "`n=== Unity 임포트 테스트 ===" -ForegroundColor Cyan

# JSON 파일 경로 확인
$jsonFiles = @{
    "food" = Join-Path $testDir "items_food.json"
    "drink" = Join-Path $testDir "items_drink.json"
    "material" = Join-Path $testDir "items_material.json"
    "weapon" = Join-Path $testDir "items_weapon.json"
    "weaponcomponent" = Join-Path $testDir "items_weaponcomponent.json"
    "ammo" = Join-Path $testDir "items_ammo.json"
    "armor" = Join-Path $testDir "items_armor.json"
    "clothing" = Join-Path $testDir "items_clothing.json"
}

Write-Host "`n생성된 JSON 파일 확인:" -ForegroundColor Yellow
foreach ($key in $jsonFiles.Keys) {
    $path = $jsonFiles[$key]
    if (Test-Path $path) {
        try {
            $json = Get-Content $path -Raw | ConvertFrom-Json
            $itemCount = if ($json -is [Array]) { $json.Count } else { 1 }
            Write-Host "  ✅ $key : $itemCount items" -ForegroundColor Green
        } catch {
            Write-Host "  ❌ $key : JSON 파싱 실패" -ForegroundColor Red
        }
    } else {
        Write-Host "  ❌ $key : 파일 없음" -ForegroundColor Red
    }
}

Write-Host "`nUnity Editor에서 다음 메뉴를 사용하여 임포트하세요:" -ForegroundColor Cyan
Write-Host "  RundeeItemFactory > Test Import All Items" -ForegroundColor Yellow
Write-Host "`n또는 Unity Editor Console에서 다음 명령을 실행하세요:" -ForegroundColor Cyan
Write-Host "  TestImportAll.ShowWindow()" -ForegroundColor Yellow

Write-Host "`n임포트 테스트 완료!" -ForegroundColor Green

