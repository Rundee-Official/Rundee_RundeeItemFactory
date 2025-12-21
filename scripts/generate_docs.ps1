# Generate documentation for RundeeItemFactory
# Creates Doxygen documentation for C++ code and XML documentation for C# code

param(
    [switch]$CppOnly,
    [switch]$CSharpOnly,
    [switch]$OpenAfter
)

$ErrorActionPreference = "Stop"

Write-Host "`n📚 RundeeItemFactory Documentation Generator" -ForegroundColor Cyan
Write-Host "==========================================`n" -ForegroundColor Cyan

# Check for Doxygen
$doxygenPath = $null
if (-not $CSharpOnly) {
    $possiblePaths = @(
        "C:\Program Files\doxygen\bin\doxygen.exe",
        "C:\Program Files (x86)\doxygen\bin\doxygen.exe",
        "$env:ProgramFiles\doxygen\bin\doxygen.exe",
        "$env:ProgramFiles(x86)\doxygen\bin\doxygen.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $doxygenPath = $path
            break
        }
    }
    
    if (-not $doxygenPath) {
        Write-Host "⚠️  Doxygen not found. Please install Doxygen from https://www.doxygen.nl/" -ForegroundColor Yellow
        Write-Host "   Or use -CSharpOnly to generate only C# documentation`n" -ForegroundColor Yellow
        if (-not $CSharpOnly) {
            exit 1
        }
    }
}

# Generate C++ documentation
if (-not $CSharpOnly -and $doxygenPath) {
    Write-Host "📖 Generating C++ Documentation (Doxygen)..." -ForegroundColor Yellow
    
    if (-not (Test-Path "Doxyfile")) {
        Write-Host "❌ Doxyfile not found!" -ForegroundColor Red
        exit 1
    }
    
    $doxyfilePath = Resolve-Path "Doxyfile"
    & $doxygenPath $doxyfilePath
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ C++ documentation generated successfully!" -ForegroundColor Green
        Write-Host "   Location: docs/cpp/html/index.html" -ForegroundColor Gray
    } else {
        Write-Host "❌ Doxygen generation failed!" -ForegroundColor Red
        exit 1
    }
}

# Generate C# documentation
if (-not $CppOnly) {
    Write-Host "`n📖 Generating C# Documentation (XML Comments)..." -ForegroundColor Yellow
    
    # Check if Unity is available
    $unityPath = $null
    $possibleUnityPaths = @(
        "C:\Program Files\Unity\Hub\Editor\*\Editor\Unity.exe",
        "C:\Program Files (x86)\Unity\Editor\Unity.exe",
        "$env:ProgramFiles\Unity\Hub\Editor\*\Editor\Unity.exe"
    )
    
    foreach ($pattern in $possibleUnityPaths) {
        $found = Get-ChildItem -Path (Split-Path $pattern -Parent) -Filter "Unity.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $unityPath = $found.FullName
            break
        }
    }
    
    if ($unityPath) {
        Write-Host "   Found Unity at: $unityPath" -ForegroundColor Gray
        
        # Create XML documentation output directory
        $xmlDocPath = "docs/csharp"
        if (-not (Test-Path $xmlDocPath)) {
            New-Item -ItemType Directory -Path $xmlDocPath -Force | Out-Null
        }
        
        Write-Host "   Note: C# XML documentation is generated during Unity compilation." -ForegroundColor Gray
        Write-Host "   XML files are located in: UnityRundeeItemFactory/Library/ScriptAssemblies/" -ForegroundColor Gray
        Write-Host "   To enable XML generation, set 'Generate XML Documentation' in Unity Project Settings." -ForegroundColor Gray
    } else {
        Write-Host "⚠️  Unity not found. C# documentation requires Unity compilation." -ForegroundColor Yellow
    }
}

# Create index page
Write-Host "`n📄 Creating documentation index..." -ForegroundColor Yellow

$indexContent = @"
<!DOCTYPE html>
<html>
<head>
    <title>RundeeItemFactory API Documentation</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 40px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }
        .section { margin: 30px 0; padding: 20px; background: #f8f9fa; border-left: 4px solid #3498db; }
        .link { display: inline-block; margin: 10px 0; padding: 12px 24px; background: #3498db; color: white; text-decoration: none; border-radius: 4px; }
        .link:hover { background: #2980b9; }
        .info { background: #e8f4f8; padding: 15px; border-radius: 4px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>RundeeItemFactory API Documentation</h1>
        <p>Comprehensive API reference for RundeeItemFactory - LLM-powered item generator for Unity & Unreal Engine games.</p>
        
        <div class="section">
            <h2>C++ API Documentation</h2>
            <div class="info">
                <p><strong>Location:</strong> <a href="cpp/html/index.html">docs/cpp/html/index.html</a></p>
                <p>Complete reference for all C++ classes, functions, and data structures.</p>
            </div>
            <a href="cpp/html/index.html" class="link">View C++ Documentation →</a>
        </div>
        
        <div class="section">
            <h2>C# Unity API Documentation</h2>
            <div class="info">
                <p><strong>Note:</strong> C# XML documentation is generated during Unity compilation.</p>
                <p>XML files are located in: <code>UnityRundeeItemFactory/Library/ScriptAssemblies/</code></p>
                <p>To enable: Unity → Edit → Project Settings → Player → Other Settings → Generate XML Documentation</p>
            </div>
        </div>
        
        <div class="section">
            <h2>Project Structure</h2>
            <ul>
                <li><strong>C++ Backend:</strong> Item generation, validation, LLM communication</li>
                <li><strong>Unity Integration:</strong> Editor windows, ScriptableObject import, GUI tools</li>
            </ul>
        </div>
        
        <div class="section">
            <h2>Quick Links</h2>
            <ul>
                <li><a href="cpp/html/namespace_item_generator.html">ItemGenerator Namespace</a></li>
                <li><a href="cpp/html/class_item_json_parser.html">ItemJsonParser Class</a></li>
                <li><a href="cpp/html/class_food_item_validator.html">FoodItemValidator Class</a></li>
            </ul>
        </div>
    </div>
</body>
</html>
"@

$indexPath = "docs/index.html"
$docsDir = "docs"
if (-not (Test-Path $docsDir)) {
    New-Item -ItemType Directory -Path $docsDir -Force | Out-Null
}
$indexContent | Out-File -FilePath $indexPath -Encoding UTF8

Write-Host "✅ Documentation index created: $indexPath" -ForegroundColor Green

# Open documentation if requested
if ($OpenAfter) {
    $indexFullPath = Resolve-Path $indexPath
    Write-Host "`n🌐 Opening documentation..." -ForegroundColor Cyan
    Start-Process $indexFullPath
}

Write-Host "`n✅ Documentation generation complete!" -ForegroundColor Green
Write-Host "`n📚 Documentation locations:" -ForegroundColor Cyan
Write-Host "   • Main index: docs/index.html" -ForegroundColor White
if (-not $CSharpOnly) {
    Write-Host "   • C++ API: docs/cpp/html/index.html" -ForegroundColor White
}
Write-Host "   • C# XML: UnityRundeeItemFactory/Library/ScriptAssemblies/*.xml" -ForegroundColor White





