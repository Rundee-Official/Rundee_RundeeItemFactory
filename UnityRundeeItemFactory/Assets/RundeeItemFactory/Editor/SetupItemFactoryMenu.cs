using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using UnityEditor;
using UnityEngine;

public static class SetupItemFactoryMenu
{
    // #region agent log helper
    private static void AgentLog(string hypothesisId, string location, string message)
    {
        var payload = $"{{\"sessionId\":\"debug-session\",\"runId\":\"run3\",\"hypothesisId\":\"{hypothesisId}\",\"location\":\"{location}\",\"message\":\"{message}\",\"timestamp\":{DateTimeOffset.UtcNow.ToUnixTimeMilliseconds()}}}\n";
        TryWriteLog(@"d:\_VisualStudioProjects\_Rundee_RundeeItemFactory\.cursor\debug.log", payload);
        TryWriteLog(Path.Combine(Application.dataPath.Replace("/Assets", ""), ".cursor", "debug_fallback.log"), payload);
    }

    private static void TryWriteLog(string path, string payload)
    {
        try
        {
            Directory.CreateDirectory(Path.GetDirectoryName(path));
            File.AppendAllText(path, payload);
        }
        catch
        {
            // swallow logging errors
        }
    }
    // #endregion

    [MenuItem("Tools/Rundee/Item Factory/Setup Item Factory")]
    public static void RunSetup()
    {
        AgentLog("H0", "SetupItemFactoryMenu.RunSetup:entry", "menu clicked");
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string batPath = Path.Combine(projectRoot, "Assets", "RundeeItemFactory", "Editor", "OllamaSetup", "install_ollama_windows.bat");

        AgentLog("H1", "SetupItemFactoryMenu.RunSetup:entry", $"batPath={batPath}");

        if (IsOllamaInstalledInDefaultPaths(out var foundPath))
        {
            AgentLog("H8", "SetupItemFactoryMenu.RunSetup:already_installed", foundPath ?? "unknown");
            EditorUtility.DisplayDialog("Setup Item Factory", $"Ollama already detected at:\n{foundPath}\nInstaller skipped.", "OK");
            return;
        }

        if (!File.Exists(batPath))
        {
            AgentLog("H1", "SetupItemFactoryMenu.RunSetup:bat_missing", "bat file not found");
            EditorUtility.DisplayDialog("Setup Item Factory", $"Batch file not found:\n{batPath}", "OK");
            return;
        }

        try
        {
            ProcessStartInfo psi = new ProcessStartInfo
            {
                FileName = batPath,
                WorkingDirectory = Path.GetDirectoryName(batPath),
                UseShellExecute = true,
                RedirectStandardOutput = false,
                RedirectStandardError = false,
                CreateNoWindow = false
            };

            var proc = Process.Start(psi);
            if (proc == null)
            {
                AgentLog("H6", "SetupItemFactoryMenu.RunSetup:process_null", "Process.Start returned null");
            }
            else
            {
                AgentLog("H2", "SetupItemFactoryMenu.RunSetup:process_started", $"pid={proc.Id}");
                proc.EnableRaisingEvents = true;
                proc.Exited += (_, __) =>
                {
                    try
                    {
                        AgentLog("H5", "SetupItemFactoryMenu.RunSetup:process_exited", $"pid={proc.Id}, exitCode={proc.ExitCode}");
                    }
                    catch
                    {
                        AgentLog("H5", "SetupItemFactoryMenu.RunSetup:process_exited", "exit logging failed");
                    }
                };
            }

            EditorUtility.DisplayDialog(
                "Setup Item Factory",
                "Launched the installer batch.\nIf a console/MS Store window flashed and closed, it may be waiting in the background.\nCheck taskbar/background windows and respond to any prompts (Y/N). Re-run if needed.",
                "OK");
        }
        catch (System.Exception ex)
        {
            AgentLog("H3", "SetupItemFactoryMenu.RunSetup:exception", ex.Message);
            EditorUtility.DisplayDialog("Setup Item Factory", $"An error occurred while running the installer:\n{ex.Message}", "OK");
        }
    }

    private static bool IsOllamaInstalledInDefaultPaths(out string detectedPath)
    {
        try
        {
            string programFilesPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "Ollama", "ollama.exe");
            string localAppPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Ollama", "ollama.exe");
            if (File.Exists(programFilesPath))
            {
                detectedPath = programFilesPath;
                return true;
            }
            if (File.Exists(localAppPath))
            {
                detectedPath = localAppPath;
                return true;
            }
        }
        catch
        {
            // ignore detection errors
        }

        detectedPath = null;
        return false;
    }
}

