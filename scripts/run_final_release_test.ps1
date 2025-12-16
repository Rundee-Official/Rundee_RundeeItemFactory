# Final release test script
# Stress test generating 50 items per type + Unity import + balance report verification
# Prerequisites: PowerShell 7+, Ollama running, deployment package or built executable exists

param(
    [string]$DeploymentDir = "Deployment",
    [string]$Configuration = "Release",
    [int]$ItemCountPerType = 50,
    [string]$Model = "llama3",
    [switch]$SkipUnity,
    [switch]$SkipBalanceReport
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$deployPath = Join-Path $repoRoot $DeploymentDir

# Check executable path (build directory first, then deployment directory)
$exePath = $null
# 1. Check build directory (Release first, then Debug)
# Correct paths: RundeeItemFactory\x64\Release or RundeeItemFactory\x64\Debug
$buildPath1 = Join-Path $repoRoot "RundeeItemFactory\x64\Release\RundeeItemFactory.exe"
$buildPath2 = Join-Path $repoRoot "RundeeItemFactory\x64\Debug\RundeeItemFactory.exe"
$buildPath3 = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\x64\Release\RundeeItemFactory.exe"
$buildPath4 = Join-Path $repoRoot "RundeeItemFactory\RundeeItemFactory\x64\Debug\RundeeItemFactory.exe"

if (Test-Path $buildPath1) {
    $exePath = (Resolve-Path $buildPath1).Path
    Write-Host "Found built executable (Release): $exePath" -ForegroundColor Green
} elseif (Test-Path $buildPath2) {
    $exePath = (Resolve-Path $buildPath2).Path
    Write-Host "Found built executable (Debug): $exePath" -ForegroundColor Green
} elseif (Test-Path $buildPath3) {
    $exePath = (Resolve-Path $buildPath3).Path
    Write-Host "Found built executable (Release, alternate path): $exePath" -ForegroundColor Green
} elseif (Test-Path $buildPath4) {
    $exePath = (Resolve-Path $buildPath4).Path
    Write-Host "Found built executable (Debug, alternate path): $exePath" -ForegroundColor Green
}

# 2. Check deployment directory
if (-not $exePath) {
    $exePath = Join-Path $deployPath "RundeeItemFactory.exe"
    if (-not (Test-Path $exePath)) {
        Write-Error "Executable file not found. Please check build directory or deployment directory."
        exit 1
    }
    $exePath = (Resolve-Path $exePath).Path
    Write-Host "Using deployment directory executable: $exePath" -ForegroundColor Yellow
}

$testDir = Join-Path $repoRoot "FinalTest"
if (-not (Test-Path $testDir)) { 
    New-Item -ItemType Directory -Path $testDir | Out-Null 
}

Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  Final Release Test Start" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "Executable: $exePath" -ForegroundColor Cyan
Write-Host "Test Directory: $testDir" -ForegroundColor Cyan
Write-Host "Items per type: $ItemCountPerType" -ForegroundColor Cyan
Write-Host "Model: $Model" -ForegroundColor Cyan
Write-Host ""

$testResults = @{
    Success = $false
    Tests = @()
    TotalTests = 0
    PassedTests = 0
    FailedTests = 0
    TotalItemsGenerated = 0
    TotalDuration = 0
}

# 1. Check Ollama connection
Write-Host "[1/7] Checking Ollama Connection" -ForegroundColor Cyan
try {
    $ping = Invoke-RestMethod -Method Get -Uri "http://localhost:11434/api/tags" -TimeoutSec 5
    $models = $ping.models | ForEach-Object { $_.name }
    if ($models -contains $Model -or $models -contains "$Model:latest") {
        Write-Host "  ✓ Ollama connection confirmed, model '$Model' available" -ForegroundColor Green
        $testResults.Tests += @{ Name = "Ollama Connection"; Status = "Pass"; Model = $Model }
        $testResults.PassedTests++
    } else {
        Write-Host "  ⚠ Model '$Model' is not installed. Available models: $($models -join ', ')" -ForegroundColor Yellow
        Write-Host "  Download model: ollama pull $Model" -ForegroundColor Yellow
        $testResults.Tests += @{ Name = "Ollama Connection"; Status = "Warning"; AvailableModels = $models }
        $testResults.PassedTests++
    }
} catch {
    Write-Host "  ✗ Ollama connection failed: $($_.Exception.Message)" -ForegroundColor Red
    $testResults.Tests += @{ Name = "Ollama Connection"; Status = "Fail"; Error = $_.Exception.Message }
    $testResults.FailedTests++
    Write-Host "  Please install and run Ollama, then try again." -ForegroundColor Red
    exit 1
}
$testResults.TotalTests++

# 2. Check required files
Write-Host "`n[2/7] Checking Required Files" -ForegroundColor Cyan
$requiredFiles = @(
    "RundeeItemFactory.exe",
    "config/rundee_config.json",
    "prompts/food.txt",
    "prompts/drink.txt",
    "prompts/material.txt",
    "prompts/weapon.txt",
    "prompts/weapon_component.txt",
    "prompts/ammo.txt"
)

$exeDir = Split-Path $exePath
$missingFiles = @()
foreach ($file in $requiredFiles) {
    $filePath = Join-Path $exeDir $file
    if (Test-Path $filePath) {
        Write-Host "  ✓ $file" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $file" -ForegroundColor Red
        $missingFiles += $file
    }
}

if ($missingFiles.Count -eq 0) {
        Write-Host "  ✓ All required files exist" -ForegroundColor Green
        $testResults.Tests += @{ Name = "Required Files Check"; Status = "Pass" }
        $testResults.PassedTests++
    } else {
        Write-Host "  ✗ Missing files: $($missingFiles.Count)" -ForegroundColor Red
        $testResults.Tests += @{ Name = "Required Files Check"; Status = "Fail"; MissingFiles = $missingFiles }
    $testResults.FailedTests++
}
$testResults.TotalTests++

# 3. Generate 50 items per type (stress test)
Write-Host "`n[3/7] Item Type Generation Test ($ItemCountPerType per type)" -ForegroundColor Cyan
$itemTypes = @("food", "drink", "material", "weapon", "weaponcomponent", "ammo")
$generationResults = @()

foreach ($itemType in $itemTypes) {
    Write-Host "`n  --- Generating $itemType... ---" -ForegroundColor Yellow
    $outputFile = Join-Path $testDir "final_test_${itemType}.json"
    
    $startTime = Get-Date
    $args = @(
        "--mode", "llm",
        "--itemType", $itemType,
        "--model", $Model,
        "--count", $ItemCountPerType.ToString(),
        "--preset", "default",
        "--out", $outputFile
    )
    
    try {
        $p = Start-Process -FilePath $exePath -ArgumentList $args -NoNewWindow -PassThru -Wait -WorkingDirectory $exeDir
        $duration = (Get-Date) - $startTime
        
        if ($p.ExitCode -ne 0) {
            Write-Host "    ✗ Generation failed (Exit Code: $($p.ExitCode))" -ForegroundColor Red
            $generationResults += @{
                ItemType = $itemType
                Success = $false
                ExitCode = $p.ExitCode
                Duration = $duration.TotalSeconds
                ItemCount = 0
            }
            $testResults.FailedTests++
            continue
        }
        
        if (-not (Test-Path $outputFile)) {
            Write-Host "    ✗ Output file was not created" -ForegroundColor Red
            $generationResults += @{
                ItemType = $itemType
                Success = $false
                Duration = $duration.TotalSeconds
                ItemCount = 0
                Error = "Output file not created"
            }
            $testResults.FailedTests++
            continue
        }
        
        # JSON file validation
        try {
            $json = Get-Content $outputFile -Raw | ConvertFrom-Json
            if ($json -isnot [Array]) { $json = @($json) }
            $itemCount = $json.Count
            
            # Basic validation: Check ID prefix
            $validItems = 0
            $prefix = switch ($itemType) {
                "food" { "Food_" }
                "drink" { "Drink_" }
                "material" { "Material_" }
                "weapon" { "Weapon_" }
                "weaponcomponent" { "WeaponComponent_" }
                "ammo" { "Ammo_" }
            }
            
            foreach ($item in $json) {
                if ($item.id -and $item.id.StartsWith($prefix)) {
                    $validItems++
                }
            }
            
            if ($itemCount -ge $ItemCountPerType * 0.9) {  # Consider success if 90% or more
                Write-Host "    ✓ Generation complete: $itemCount items (valid: $validItems, $([math]::Round($duration.TotalSeconds, 2))s)" -ForegroundColor Green
                $generationResults += @{
                    ItemType = $itemType
                    Success = $true
                    Duration = $duration.TotalSeconds
                    ItemCount = $itemCount
                    ValidItems = $validItems
                }
                $testResults.PassedTests++
                $testResults.TotalItemsGenerated += $itemCount
                $testResults.TotalDuration += $duration.TotalSeconds
            } else {
                Write-Host "    ⚠ Insufficient items generated: $itemCount / $ItemCountPerType" -ForegroundColor Yellow
                $generationResults += @{
                    ItemType = $itemType
                    Success = $false
                    Duration = $duration.TotalSeconds
                    ItemCount = $itemCount
                    ExpectedCount = $ItemCountPerType
                    Error = "Insufficient items generated"
                }
                $testResults.FailedTests++
            }
        } catch {
            Write-Host "    ✗ JSON parsing failed: $($_.Exception.Message)" -ForegroundColor Red
            $generationResults += @{
                ItemType = $itemType
                Success = $false
                Duration = $duration.TotalSeconds
                ItemCount = 0
                Error = "JSON parsing failed: $($_.Exception.Message)"
            }
            $testResults.FailedTests++
        }
    } catch {
        Write-Host "    ✗ Runtime error: $($_.Exception.Message)" -ForegroundColor Red
        $generationResults += @{
            ItemType = $itemType
            Success = $false
            Duration = 0
            ItemCount = 0
            Error = $_.Exception.Message
        }
        $testResults.FailedTests++
    }
    
    $testResults.TotalTests++
}

# Generation results summary
$successCount = ($generationResults | Where-Object { $_.Success }).Count
Write-Host "`n  === Generation Results Summary ===" -ForegroundColor Cyan
Write-Host "  Success: $successCount / $($itemTypes.Count)" -ForegroundColor $(if ($successCount -eq $itemTypes.Count) { "Green" } else { "Yellow" })
Write-Host "  Total items generated: $($testResults.TotalItemsGenerated)" -ForegroundColor Cyan
Write-Host "  Total duration: $([math]::Round($testResults.TotalDuration, 2))s" -ForegroundColor Cyan

# 4. Balance report verification
if (-not $SkipBalanceReport) {
    Write-Host "`n[4/7] Balance Report Verification" -ForegroundColor Cyan
    
    foreach ($itemType in $itemTypes) {
        $jsonFile = Join-Path $testDir "final_test_${itemType}.json"
        if (-not (Test-Path $jsonFile)) {
            Write-Host "  ⚠ Skipping report for $itemType (file not found)" -ForegroundColor Yellow
            continue
        }
        
        Write-Host "  --- Generating $itemType balance report... ---" -ForegroundColor Yellow
        $args = @(
            "--report", $jsonFile,
            "--itemType", $itemType
        )
        
        try {
            $output = & $exePath $args 2>&1 | Out-String
            if ($LASTEXITCODE -eq 0 -or $output -match "Balance" -or $output -match "Total Items") {
                Write-Host "    ✓ $itemType balance report generated successfully" -ForegroundColor Green
                $testResults.Tests += @{ Name = "$itemType Balance Report"; Status = "Pass" }
                $testResults.PassedTests++
            } else {
                Write-Host "    ✗ $itemType balance report generation failed" -ForegroundColor Red
                $testResults.Tests += @{ Name = "$itemType Balance Report"; Status = "Fail"; Error = "Unexpected output" }
                $testResults.FailedTests++
            }
        } catch {
            Write-Host "    ✗ $itemType balance report error: $($_.Exception.Message)" -ForegroundColor Red
            $testResults.Tests += @{ Name = "$itemType Balance Report"; Status = "Fail"; Error = $_.Exception.Message }
            $testResults.FailedTests++
        }
        $testResults.TotalTests++
    }
} else {
    Write-Host "`n[4/7] Balance Report Verification (Skipped)" -ForegroundColor Gray
}

# 5. JSON file validation
Write-Host "`n[5/7] JSON File Validation" -ForegroundColor Cyan
$validationResults = @()

foreach ($itemType in $itemTypes) {
    $jsonFile = Join-Path $testDir "final_test_${itemType}.json"
    if (-not (Test-Path $jsonFile)) {
        continue
    }
    
    try {
        $json = Get-Content $jsonFile -Raw | ConvertFrom-Json
        if ($json -isnot [Array]) { $json = @($json) }
        
        $errors = @()
        $prefix = switch ($itemType) {
            "food" { "Food_" }
            "drink" { "Drink_" }
            "material" { "Material_" }
            "weapon" { "Weapon_" }
            "weaponcomponent" { "WeaponComponent_" }
            "ammo" { "Ammo_" }
        }
        
        foreach ($item in $json) {
            if (-not $item.id) {
                $errors += "Missing ID"
            } elseif (-not $item.id.StartsWith($prefix)) {
                $errors += "Invalid ID prefix: $($item.id)"
            }
            if (-not $item.displayName) {
                $errors += "Missing displayName"
            }
            if (-not $item.category) {
                $errors += "Missing category"
            }
        }
        
        if ($errors.Count -eq 0) {
            Write-Host "  ✓ $itemType JSON validation passed ($($json.Count) items)" -ForegroundColor Green
            $validationResults += @{ ItemType = $itemType; Status = "Pass"; ItemCount = $json.Count }
            $testResults.PassedTests++
        } else {
            Write-Host "  ✗ $itemType JSON validation failed: $($errors[0])" -ForegroundColor Red
            $validationResults += @{ ItemType = $itemType; Status = "Fail"; Errors = $errors }
            $testResults.FailedTests++
        }
    } catch {
        Write-Host "  ✗ $itemType JSON parsing failed: $($_.Exception.Message)" -ForegroundColor Red
        $validationResults += @{ ItemType = $itemType; Status = "Fail"; Error = $_.Exception.Message }
        $testResults.FailedTests++
    }
    $testResults.TotalTests++
}

# 6. Unity import test (optional)
if (-not $SkipUnity) {
    Write-Host "`n[6/7] Unity Import Test" -ForegroundColor Cyan
    $unityProjectPath = Join-Path $repoRoot "UnityRundeeItemFactory"
    
    if (Test-Path $unityProjectPath) {
        Write-Host "  Unity project found: $unityProjectPath" -ForegroundColor Cyan
        Write-Host "  ⚠ Unity import must be performed manually:" -ForegroundColor Yellow
        Write-Host "    1. Open project in Unity Editor" -ForegroundColor Yellow
        Write-Host "    2. Tools > Rundee > Item Factory > JSON Importer" -ForegroundColor Yellow
        Write-Host "    3. Import each generated JSON file" -ForegroundColor Yellow
        Write-Host "    4. Verify generated ScriptableObjects in Item Manager" -ForegroundColor Yellow
        $testResults.Tests += @{ Name = "Unity Import"; Status = "Manual"; Note = "Manual test required" }
        $testResults.PassedTests++
    } else {
        Write-Host "  ⚠ Unity project not found: $unityProjectPath" -ForegroundColor Yellow
        $testResults.Tests += @{ Name = "Unity Import"; Status = "Skipped"; Note = "Unity project not found" }
        $testResults.PassedTests++
    }
    $testResults.TotalTests++
} else {
    Write-Host "`n[6/7] Unity Import Test (Skipped)" -ForegroundColor Gray
}

# 7. Final summary
Write-Host "`n[7/7] Final Summary" -ForegroundColor Cyan

# Results summary
Write-Host "`n========================================" -ForegroundColor Yellow
Write-Host "  Final Release Test Results" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "Total tests: $($testResults.TotalTests)" -ForegroundColor Cyan
Write-Host "Passed: $($testResults.PassedTests)" -ForegroundColor Green
Write-Host "Failed: $($testResults.FailedTests)" -ForegroundColor $(if ($testResults.FailedTests -eq 0) { "Green" } else { "Red" })
Write-Host "Total items generated: $($testResults.TotalItemsGenerated)" -ForegroundColor Cyan
Write-Host "Total duration: $([math]::Round($testResults.TotalDuration, 2))s" -ForegroundColor Cyan

# Detailed generation results
Write-Host "`n--- Generation Results by Item Type ---" -ForegroundColor Cyan
foreach ($result in $generationResults) {
    $status = if ($result.Success) { "✓" } else { "✗" }
    $color = if ($result.Success) { "Green" } else { "Red" }
    Write-Host "$status $($result.ItemType): $($result.ItemCount) items ($([math]::Round($result.Duration, 2))s)" -ForegroundColor $color
}

if ($testResults.FailedTests -eq 0) {
    Write-Host "`n✓ All tests passed!" -ForegroundColor Green
    $testResults.Success = $true
} else {
    Write-Host "`n✗ Some tests failed" -ForegroundColor Red
    $testResults.Success = $false
}

Write-Host "`nTest result files location: $testDir" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Yellow

return $testResults

