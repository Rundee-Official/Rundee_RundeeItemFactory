using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using UnityEditor;
using UnityEngine;

public static class SetupItemFactoryMenu
{
    // Debug logging disabled for release

    [MenuItem("Tools/Rundee/Item Factory/Setup/Setup Item Factory", false, 6000)]
    public static void RunSetup()
    {
        // Setup menu clicked
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string batPath = Path.Combine(projectRoot, "Assets", "RundeeItemFactory", "Editor", "OllamaSetup", "install_ollama_windows.bat");

        // Batch file path determined

        if (IsOllamaInstalledInDefaultPaths(out var foundPath))
        {
            // Ollama already installed
            EditorUtility.DisplayDialog("Setup Item Factory", $"Ollama already detected at:\n{foundPath}\nInstaller skipped.", "OK");
            return;
        }

        if (!File.Exists(batPath))
        {
            // Batch file not found
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
                // Process start failed
            }
            else
            {
                // Process started successfully
                proc.EnableRaisingEvents = true;
                proc.Exited += (_, __) =>
                {
                    try
                    {
                        // Process exited
                    }
                    catch
                    {
                        // Exit logging failed
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
            // Exception occurred
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

