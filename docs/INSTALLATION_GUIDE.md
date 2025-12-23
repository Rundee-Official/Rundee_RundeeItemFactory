# Installation Guide

This guide provides detailed instructions for installing and setting up RundeeItemFactory.

## Prerequisites

### Required Software

1. **Windows 10/11** (64-bit)
2. **Visual Studio 2019 or later** (with C++ workload)
   - Download from: https://visualstudio.microsoft.com/
   - Ensure "Desktop development with C++" workload is installed
3. **Unity 2020.3 or later** (for Unity integration)
   - Download from: https://unity.com/download
4. **Ollama** (Local LLM server)
   - Download from: https://ollama.ai
   - See [Ollama Setup](#ollama-setup) section below

### Optional Software

- **Git** (for cloning the repository)
- **PowerShell 7+** (for running test scripts)

## Step-by-Step Installation

### 1. Clone the Repository

```bash
git clone <repository-url>
cd RundeeItemFactory
```

### 2. Install Ollama

#### Windows

1. Download Ollama from https://ollama.ai/download
2. Run the installer (OllamaSetup.exe)
3. Verify installation by opening PowerShell and running:
   ```powershell
   ollama --version
   ```

4. Start Ollama (it should start automatically):
   ```powershell
   ollama serve
   ```

5. Pull a model (we recommend llama3 for best results):
   ```powershell
   ollama pull llama3
   ```

   **Note:** The first model pull may take several minutes depending on your internet connection. Model size is approximately 4.7 GB.

#### Alternative: Using Unity Installer

If you're using Unity integration, you can also install Ollama directly from Unity:

1. Open Unity Editor
2. Go to **Tools > Rundee > Item Factory > Setup > Setup Item Factory**
3. Click the "Install Ollama" button
4. Follow the on-screen instructions

### 3. Build the C++ Project

1. Open `RundeeItemFactory/RundeeItemFactory.sln` in Visual Studio

2. Select configuration:
   - **Debug**: For development/testing (slower but with debug symbols)
   - **Release**: For production use (faster, optimized)

3. Build the solution:
   - Press `Ctrl+Shift+B` or go to **Build > Build Solution**

4. Verify build output:
   - Debug build: `x64/Debug/RundeeItemFactory.exe`
   - Release build: `x64/Release/RundeeItemFactory.exe`

5. Verify required files are copied:
   - `prompts/` directory should exist next to the executable
   - `config/rundee_config.json` should exist (optional)

### 4. Test the Installation

#### Command Line Test

Open PowerShell in the build output directory and run:

```powershell
.\RundeeItemFactory.exe --mode dummy --itemType food --count 3
```

Expected output:
```
[DummyGenerator] Generated 3 dummy food items.
[ItemJsonWriter] Wrote 3 food items to ItemJson/items_food.json
```

#### LLM Test (requires Ollama)

Ensure Ollama is running, then:

```powershell
.\RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 5 --preset default
```

This will generate 5 food items using the LLM. First run may take 1-2 minutes.

### 5. Unity Integration Setup

1. Open Unity Editor

2. Import the Unity project:
   - Open `UnityRundeeItemFactory` folder in Unity

3. Configure executable path:
   - Go to **Tools > Rundee > Item Factory > Item Factory Window**
   - In the "Executable Path" field, browse to your `RundeeItemFactory.exe`
   - Default locations:
     - Debug: `RundeeItemFactory/x64/Debug/RundeeItemFactory.exe`
     - Release: `RundeeItemFactory/x64/Release/RundeeItemFactory.exe`

4. Test Unity integration:
   - In Item Factory Window, click "Verify Executable"
   - Should show: "✅ Executable found and valid"

## Configuration

### Config File

Create or edit `config/rundee_config.json` next to the executable:

```json
{
  "ollama": {
    "host": "localhost",
    "port": 11434,
    "maxRetries": 3,
    "requestTimeoutSeconds": 120
  }
}
```

**Settings:**
- `host`: Ollama server hostname (default: "localhost")
- `port`: Ollama server port (default: 11434)
- `maxRetries`: Maximum retry attempts for failed LLM requests (default: 3)
- `requestTimeoutSeconds`: HTTP request timeout in seconds (default: 120)

### Profiles Directory

Item profiles are stored in `profiles/` directory next to the executable. Default profiles are created automatically on first run.

## Troubleshooting

### Ollama Not Found

**Symptoms:**
- Error: "Ollama not found" or connection errors
- LLM generation fails

**Solutions:**
1. Verify Ollama is installed:
   ```powershell
   ollama --version
   ```

2. Check if Ollama is running:
   ```powershell
   ollama list
   ```
   If this fails, start Ollama:
   ```powershell
   ollama serve
   ```

3. Verify Ollama is accessible:
   - Open browser: http://localhost:11434
   - Should see Ollama API documentation

4. Check firewall settings - Ollama uses port 11434

### Model Not Found

**Symptoms:**
- Error: "model not found" or "model does not exist"

**Solutions:**
1. List available models:
   ```powershell
   ollama list
   ```

2. Pull the required model:
   ```powershell
   ollama pull llama3
   ```

3. Verify model is available:
   ```powershell
   ollama run llama3 "test"
   ```

### Build Errors

**Symptoms:**
- Visual Studio build fails
- Missing headers or libraries

**Solutions:**
1. Verify C++ workload is installed in Visual Studio
2. Ensure you're building for x64 platform (not x86)
3. Check that `json.hpp` is present in `include/` directory
4. Try cleaning and rebuilding:
   - **Build > Clean Solution**
   - **Build > Rebuild Solution**

### Unity Integration Issues

**Symptoms:**
- Executable not found
- Generation fails from Unity

**Solutions:**
1. Verify executable path is correct (use absolute path if relative path fails)
2. Check file permissions (ensure Unity has read/execute permissions)
3. Test executable manually from command line first
4. Check Unity Console for error messages

### JSON Parsing Errors

**Symptoms:**
- "JSON parse error" messages
- Generated items are incomplete

**Solutions:**
1. Try a different LLM model (some models generate better JSON than others)
2. Increase `requestTimeoutSeconds` in config (model may need more time)
3. Reduce batch size (generate fewer items at once)
4. Check LLM response quality by examining raw output

## System Requirements

### Minimum Requirements

- **CPU**: Intel Core i5 or AMD equivalent (4 cores recommended)
- **RAM**: 8 GB (16 GB recommended for larger models)
- **Storage**: 10 GB free space (5 GB for models, 5 GB for project)
- **GPU**: Not required (CPU-only inference works, but GPU significantly faster)

### Recommended Requirements

- **CPU**: Intel Core i7/AMD Ryzen 7 or better (8+ cores)
- **RAM**: 16 GB or more
- **Storage**: SSD with 20+ GB free space
- **GPU**: NVIDIA GPU with 8+ GB VRAM (for faster LLM inference)
  - Supports CUDA for accelerated inference
  - Without GPU, inference is 5-10x slower but still functional

## Next Steps

After installation:

1. Read the [README.md](../README.md) for usage instructions
2. Check [UNITY_IMPORT_GUIDE.md](UNITY_IMPORT_GUIDE.md) for Unity-specific features
3. Explore [CODE_STRUCTURE.md](CODE_STRUCTURE.md) to understand the codebase
4. Review [FEATURE_IMPLEMENTATION_SUMMARY.md](FEATURE_IMPLEMENTATION_SUMMARY.md) for available features

## Getting Help

If you encounter issues not covered in this guide:

1. Check the Unity Console or command-line output for error messages
2. Review the troubleshooting section above
3. Check project documentation in the `docs/` directory
4. Review error logs (if available)

## Uninstallation

To uninstall RundeeItemFactory:

1. Delete the project directory
2. Uninstall Ollama (if desired):
   - Windows: Settings > Apps > Ollama > Uninstall
3. Remove Unity package (if integrated):
   - Delete `Assets/RundeeItemFactory` folder from Unity project

No system-level changes are made during installation, so no additional cleanup is required.






