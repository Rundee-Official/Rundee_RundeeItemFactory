@echo off
setlocal
title Ollama Installer (Rundee Item Factory)
chcp 65001 >nul

echo ============================================================
echo   Installing Ollama (requires winget or manual download)
echo ============================================================
echo.

REM Try winget first (interactive to allow store prompt)
where winget >nul 2>&1
if %ERRORLEVEL%==0 (
    echo [1/2] winget detected. Interactive install (MS Store prompt may appear)...
    winget install --id Ollama.Ollama -e
    if %ERRORLEVEL%==0 (
        echo.
        echo Ollama installed via winget.
        goto DONE
    ) else (
        echo winget install failed, falling back to direct download...
    )
) else (
    echo winget not found. Falling back to direct download...
)

REM Direct download fallback
set "OLLAMA_TMP=%TEMP%\OllamaSetup.exe"
echo [2/2] Downloading installer to %OLLAMA_TMP%
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "Invoke-WebRequest 'https://ollama.com/download/OllamaSetup.exe' -OutFile '%OLLAMA_TMP%'"
if exist "%OLLAMA_TMP%" (
    echo Launching installer (silent)...
    start /wait "" "%OLLAMA_TMP%" /S
    goto DONE
) else (
    echo Download failed. Please download manually from https://ollama.com/download
    goto END
)

:DONE
echo.
echo Installation step completed. If the ollama command is not available immediately,
echo open a new terminal or restart the system.
echo.
pause
goto END

:END
endlocal
@echo off
setlocal
echo [OllamaSetup] Starting Ollama installer...

set "INSTALL_DIR=%ProgramFiles%\Ollama"
set "LOCAL_INSTALL_DIR=%LocalAppData%\Programs\Ollama"

if exist "%INSTALL_DIR%\ollama.exe" (
    echo [OllamaSetup] Ollama already installed at %INSTALL_DIR%.
    exit /b 0
)
if exist "%LOCAL_INSTALL_DIR%\ollama.exe" (
    echo [OllamaSetup] Ollama already installed at %LOCAL_INSTALL_DIR%.
    exit /b 0
)

set "TMP_DIR=%TEMP%\ollama_setup"
if not exist "%TMP_DIR%" (
    mkdir "%TMP_DIR%" >nul 2>nul
)
set "INSTALLER=%TMP_DIR%\OllamaSetup.exe"

echo [OllamaSetup] Downloading Ollama installer...
powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -Command ^
  "try { Invoke-WebRequest -Uri https://ollama.com/download/OllamaSetup.exe -OutFile '%INSTALLER%' -UseBasicParsing } catch { Write-Error $_; exit 1 }"
if not exist "%INSTALLER%" (
    echo [OllamaSetup] Download failed. Check your network connection.
    exit /b 1
)

echo [OllamaSetup] Running installer silently...
start /wait "" "%INSTALLER%" /S
if errorlevel 1 (
    echo [OllamaSetup] Installer returned error code %ERRORLEVEL%.
    exit /b %ERRORLEVEL%
)

echo [OllamaSetup] Installation complete. Please ensure Ollama service is running.
exit /b 0

