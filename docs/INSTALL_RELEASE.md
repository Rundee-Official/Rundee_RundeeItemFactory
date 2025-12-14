# Install & Release Guide

## Prerequisites
- Windows (Ollama + C++ build tested on Win10/11)
- PowerShell 7+ (for scripts)
- Ollama installed and running (`ollama serve`)

## Contents
- C++ build (`RundeeItemFactory.exe`) + `config/` + `prompts/`
- Unity package (`UnityRundeeItemFactory.unitypackage`) or Unity project folder
- Installer batch: `Assets/RundeeItemFactory/Editor/OllamaSetup/install_ollama_windows.bat`

## Steps (Fresh Environment)
1) Install Ollama  
   - Run `install_ollama_windows.bat` or download from https://ollama.com/download  
   - Start server: `ollama serve`
   - Pull model if needed: `ollama pull llama3`
2) Place C++ build next to `config/` and `prompts/` (e.g., `RundeeItemFactory/x64/Release/`)
3) Unity setup  
   - Import `UnityRundeeItemFactory.unitypackage` OR open the provided Unity project
   - In Unity: Tools > Rundee > Item Factory > Setup (Ollama)  
     - “Check Ollama (default paths)” should show detected path (green)
     - “Install Ollama (run bundled .bat)” skips if already installed
4) Generation & Import  
   - CLI (LLM): `RundeeItemFactory.exe --mode llm --itemType food --count 10 --out items_food.json`  
   - Unity: Tools > Rundee > Item Factory > JSON Importer → select JSON → ScriptableObjects created
5) Optional: Run smoke tests  
   - `pwsh scripts/run_smoke_tests.ps1` (requires Ollama running)

## Packaging Notes
- Include `config/rundee_config.json` with sensible defaults (host/port/timeouts/retries)
- Include `prompts/` for all item types
- For Unity package: include `Assets/RundeeItemFactory/Editor` and `Runtime` (and meta files)
- Document minimal model requirements (e.g., llama3) and disk/memory expectations

## Troubleshooting
- Ollama not detected: run Setup in Unity or run installer; ensure `ollama serve` is active
- LLM call slow: reduce count for tests (e.g., 2–5), verify model is loaded, check GPU/CPU load
- Import issues: ensure JSON schema matches type; check console for duplicate ID skips



