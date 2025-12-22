# Comprehensive System Test Script
$ErrorActionPreference = 'Continue'

$repoRoot = Resolve-Path "."
Write-Host "=== RundeeItemFactory System Test ===" -ForegroundColor Cyan
Write-Host "Repository Root: $repoRoot" -ForegroundColor Gray

# Find executable
$exePath = Join-Path $repoRoot "Deployment\RundeeItemFactory.exe"
if (-not (Test-Path $exePath)) {
    $exePath = Get-ChildItem -Path $repoRoot -Recurse -Filter "RundeeItemFactory.exe" | 
        Where-Object { $_.DirectoryName -notlike "*\.git*" -and $_.DirectoryName -notlike "*\obj\*" } | 
        Select-Object -First 1 -ExpandProperty FullName
}

if (-not $exePath -or -not (Test-Path $exePath)) {
    Write-Host "ERROR: Executable not found. Please build the project first." -ForegroundColor Red
    exit 1
}

Write-Host "`nExecutable: $exePath" -ForegroundColor Green
$exeDir = [System.IO.Path]::GetDirectoryName($exePath)
Write-Host "Working Directory: $exeDir" -ForegroundColor Gray

# Change to executable directory
Push-Location $exeDir

try {
    # Test 1: Check if profiles directory exists or gets created
    Write-Host "`n[Test 1] Profile System Initialization" -ForegroundColor Yellow
    $profilesDir = Join-Path $exeDir "profiles"
    Write-Host "Profiles directory: $profilesDir" -ForegroundColor Gray
    
    # Test 2: Generate food items with profile
    Write-Host "`n[Test 2] Generate Food Items (Profile-based)" -ForegroundColor Yellow
    $testFile = "test_food_profile.json"
    if (Test-Path $testFile) { Remove-Item $testFile }
    
    Write-Host "Command: RundeeItemFactory.exe --mode llm --itemType food --count 2 --preset default --out $testFile" -ForegroundColor Gray
    $result = & $exePath --mode llm --itemType food --count 2 --preset default --out $testFile 2>&1
    
    Write-Host "Exit Code: $LASTEXITCODE" -ForegroundColor $(if ($LASTEXITCODE -eq 0) { "Green" } else { "Red" })
    $result | ForEach-Object { Write-Host $_ }
    
    if ($LASTEXITCODE -eq 0 -and (Test-Path $testFile)) {
        $json = Get-Content $testFile -Raw | ConvertFrom-Json
        Write-Host "SUCCESS: Generated $($json.Count) items" -ForegroundColor Green
        if ($json.Count -gt 0) {
            Write-Host "`nFirst item sample:" -ForegroundColor Cyan
            $json[0] | ConvertTo-Json -Depth 5
        }
    } else {
        Write-Host "FAILED: File not created or exit code non-zero" -ForegroundColor Red
    }
    
    # Test 3: Check profiles directory after execution
    Write-Host "`n[Test 3] Profile Directory Check" -ForegroundColor Yellow
    if (Test-Path $profilesDir) {
        $profiles = Get-ChildItem $profilesDir -Filter "*.json"
        Write-Host "Found $($profiles.Count) profile(s):" -ForegroundColor Green
        $profiles | ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor Gray }
    } else {
        Write-Host "Profiles directory not created" -ForegroundColor Yellow
    }
    
    # Test 4: Test with custom preset name
    Write-Host "`n[Test 4] Custom Preset Test" -ForegroundColor Yellow
    $testFile2 = "test_food_custom_preset.json"
    if (Test-Path $testFile2) { Remove-Item $testFile2 }
    
    Write-Host "Command: RundeeItemFactory.exe --mode llm --itemType food --count 1 --preset forest --out $testFile2" -ForegroundColor Gray
    $result2 = & $exePath --mode llm --itemType food --count 1 --preset forest --out $testFile2 2>&1
    
    Write-Host "Exit Code: $LASTEXITCODE" -ForegroundColor $(if ($LASTEXITCODE -eq 0) { "Green" } else { "Red" })
    if ($LASTEXITCODE -eq 0 -and (Test-Path $testFile2)) {
        Write-Host "SUCCESS: Custom preset test passed" -ForegroundColor Green
    } else {
        Write-Host "FAILED or skipped (Ollama may not be running)" -ForegroundColor Yellow
    }
    
    Write-Host "`n=== Test Summary ===" -ForegroundColor Cyan
    Write-Host "All tests completed. Check results above." -ForegroundColor Gray
    
} finally {
    Pop-Location
}

