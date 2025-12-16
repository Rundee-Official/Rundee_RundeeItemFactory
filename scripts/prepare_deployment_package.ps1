# Deployment package preparation script
# Assumes: Project is built and required files exist
# Prerequisites: PowerShell 7+, Build completed

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [string]$OutputDir = "Deployment"
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$buildDir = Join-Path $repoRoot "RundeeItemFactory\$Platform\$Configuration"
$deployDir = Join-Path $repoRoot $OutputDir

Write-Host "=== Deployment Package Preparation Start ===" -ForegroundColor Yellow

# Create deployment directory
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

# 1. Check executable file
$exePath = Join-Path $buildDir "RundeeItemFactory.exe"
if (Test-Path $exePath) {
    Copy-Item -Path $exePath -Destination $deployDir
    $packageInfo.Files += "RundeeItemFactory.exe"
    Write-Host "✓ Executable file copied" -ForegroundColor Green
} else {
    $packageInfo.MissingFiles += "RundeeItemFactory.exe"
    Write-Host "✗ Executable file not found: $exePath" -ForegroundColor Red
}

# 2. Check prompts folder
$promptsSource = Join-Path $buildDir "prompts"
$promptsDest = Join-Path $deployDir "prompts"
if (Test-Path $promptsSource) {
    Copy-Item -Path $promptsSource -Destination $promptsDest -Recurse
    $promptFiles = Get-ChildItem -Path $promptsSource -File
    foreach ($file in $promptFiles) {
        $packageInfo.Files += "prompts/$($file.Name)"
    }
    Write-Host "✓ prompts folder copied ($($promptFiles.Count) files)" -ForegroundColor Green
} else {
    $packageInfo.MissingFiles += "prompts/"
    Write-Host "⚠ prompts folder not found: $promptsSource" -ForegroundColor Yellow
}

# 3. Check config folder
$configSource = Join-Path $buildDir "config"
$configDest = Join-Path $deployDir "config"
if (Test-Path $configSource) {
    Copy-Item -Path $configSource -Destination $configDest -Recurse
    $configFiles = Get-ChildItem -Path $configSource -File
    foreach ($file in $configFiles) {
        $packageInfo.Files += "config/$($file.Name)"
    }
    Write-Host "✓ config folder copied ($($configFiles.Count) files)" -ForegroundColor Green
} else {
    $packageInfo.MissingFiles += "config/"
    Write-Host "⚠ config folder not found: $configSource" -ForegroundColor Yellow
}

# 4. Copy documentation files
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
        Write-Host "✓ $($doc.Source) copied" -ForegroundColor Green
    } else {
        if ($doc.Required) {
            $packageInfo.MissingFiles += $doc.Source
            Write-Host "✗ Required document not found: $($doc.Source)" -ForegroundColor Red
        } else {
            Write-Host "⚠ Optional document not found: $($doc.Source)" -ForegroundColor Yellow
        }
    }
}

# 5. Create deployment checklist
$checklistPath = Join-Path $deployDir "DEPLOYMENT_CHECKLIST.md"
$checklistContent = @"
# Deployment Checklist

## Required Files Verification

### Executable
- [ ] RundeeItemFactory.exe exists

### Configuration Files
- [ ] config/rundee_config.json exists
- [ ] prompts/ folder and all prompt files exist
  - [ ] prompts/food.txt
  - [ ] prompts/drink.txt
  - [ ] prompts/material.txt
  - [ ] prompts/weapon.txt
  - [ ] prompts/weapon_component.txt
  - [ ] prompts/ammo.txt

### Documentation
- [ ] README.md exists

## Dependencies Verification

- [ ] Ollama installed and running
- [ ] Visual C++ Redistributable installed (if needed)

## Testing

- [ ] Verify executable runs correctly
- [ ] Ollama connection test
- [ ] Sample item generation test

## Deployment Information

- Build Configuration: $Configuration
- Platform: $Platform
- Package Preparation Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
- File Count: $($packageInfo.Files.Count)
"@

Set-Content -Path $checklistPath -Value $checklistContent
$packageInfo.Files += "DEPLOYMENT_CHECKLIST.md"
Write-Host "✓ Deployment checklist created" -ForegroundColor Green

# Results summary
Write-Host "`n=== Deployment Package Preparation Results ===" -ForegroundColor Yellow
Write-Host "Deployment Directory: $deployDir" -ForegroundColor Cyan
Write-Host "Files Copied: $($packageInfo.Files.Count)" -ForegroundColor Cyan

if ($packageInfo.MissingFiles.Count -eq 0) {
    Write-Host "✓ All required files are ready!" -ForegroundColor Green
    $packageInfo.Success = $true
} else {
    Write-Host "⚠ Missing files: $($packageInfo.MissingFiles.Count)" -ForegroundColor Yellow
    foreach ($missing in $packageInfo.MissingFiles) {
        Write-Host "  - $missing" -ForegroundColor Yellow
    }
    $packageInfo.Success = $false
}

return $packageInfo




