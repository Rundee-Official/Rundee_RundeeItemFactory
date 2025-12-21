using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Version management and backup system for generated items
/// </summary>
public static class VersionManager
{
    private static string GetBackupDirectory()
    {
        string backupDir = Path.Combine(Application.dataPath, "..", "Deployment", "Backups");
        if (!Directory.Exists(backupDir))
        {
            Directory.CreateDirectory(backupDir);
        }
        return backupDir;
    }
    
    public static string CreateBackup(string filePath)
    {
        if (!File.Exists(filePath))
        {
            Debug.LogWarning($"[VersionManager] File not found: {filePath}");
            return null;
        }
        
        try
        {
            string backupDir = GetBackupDirectory();
            string fileName = Path.GetFileNameWithoutExtension(filePath);
            string extension = Path.GetExtension(filePath);
            string timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            string backupFileName = $"{fileName}_{timestamp}{extension}";
            string backupPath = Path.Combine(backupDir, backupFileName);
            
            File.Copy(filePath, backupPath, true);
            
            Debug.Log($"[VersionManager] Backup created: {backupPath}");
            return backupPath;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[VersionManager] Failed to create backup: {ex.Message}");
            return null;
        }
    }
    
    public static bool RestoreBackup(string backupPath, string targetPath)
    {
        if (!File.Exists(backupPath))
        {
            Debug.LogError($"[VersionManager] Backup file not found: {backupPath}");
            return false;
        }
        
        try
        {
            string targetDir = Path.GetDirectoryName(targetPath);
            if (!string.IsNullOrEmpty(targetDir) && !Directory.Exists(targetDir))
            {
                Directory.CreateDirectory(targetDir);
            }
            
            File.Copy(backupPath, targetPath, true);
            Debug.Log($"[VersionManager] Restored backup to: {targetPath}");
            return true;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[VersionManager] Failed to restore backup: {ex.Message}");
            return false;
        }
    }
    
    public static List<BackupInfo> GetVersionHistory(string filePath)
    {
        List<BackupInfo> history = new List<BackupInfo>();
        
        if (!File.Exists(filePath))
        {
            return history;
        }
        
        try
        {
            string backupDir = GetBackupDirectory();
            string fileName = Path.GetFileNameWithoutExtension(filePath);
            string extension = Path.GetExtension(filePath);
            
            if (!Directory.Exists(backupDir))
            {
                return history;
            }
            
            string pattern = $"{fileName}_*{extension}";
            var backupFiles = Directory.GetFiles(backupDir, pattern);
            
            foreach (var backupFile in backupFiles)
            {
                var fileInfo = new FileInfo(backupFile);
                history.Add(new BackupInfo
                {
                    FilePath = backupFile,
                    CreatedAt = fileInfo.CreationTime,
                    FileSize = fileInfo.Length
                });
            }
            
            history = history.OrderByDescending(h => h.CreatedAt).ToList();
        }
        catch (Exception ex)
        {
            Debug.LogError($"[VersionManager] Failed to get version history: {ex.Message}");
        }
        
        return history;
    }
    
    public static void CleanOldBackups(int keepDays = 30)
    {
        try
        {
            string backupDir = GetBackupDirectory();
            if (!Directory.Exists(backupDir))
            {
                return;
            }
            
            var cutoffDate = DateTime.Now.AddDays(-keepDays);
            var files = Directory.GetFiles(backupDir);
            int deletedCount = 0;
            
            foreach (var file in files)
            {
                var fileInfo = new FileInfo(file);
                if (fileInfo.CreationTime < cutoffDate)
                {
                    File.Delete(file);
                    deletedCount++;
                }
            }
            
            if (deletedCount > 0)
            {
                Debug.Log($"[VersionManager] Cleaned {deletedCount} old backup(s)");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"[VersionManager] Failed to clean old backups: {ex.Message}");
        }
    }
    
    public class BackupInfo
    {
        public string FilePath;
        public DateTime CreatedAt;
        public long FileSize;
    }
}

/// <summary>
/// Unity Editor window for managing backups and versions
/// </summary>
public class VersionManagerWindow : EditorWindow
{
    private string selectedFilePath = "";
    private List<VersionManager.BackupInfo> versionHistory = new List<VersionManager.BackupInfo>();
    private Vector2 scrollPosition;
    
    [MenuItem("Tools/Rundee/Item Factory/Management/Version Manager", false, 2020)]
    public static void ShowWindow()
    {
        var window = GetWindow<VersionManagerWindow>("Version Manager");
        window.minSize = new Vector2(600, 400);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Version Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Manage backups and restore previous versions of generated item files.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // File Selection
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("File:", GUILayout.Width(50));
        EditorGUILayout.TextField(selectedFilePath, GUILayout.ExpandWidth(true));
        if (GUILayout.Button("Browse", GUILayout.Width(80)))
        {
            string path = EditorUtility.OpenFilePanel("Select File", "", "json");
            if (!string.IsNullOrEmpty(path))
            {
                selectedFilePath = path;
                LoadVersionHistory();
            }
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Actions
        EditorGUILayout.BeginHorizontal();
        EditorGUI.BeginDisabledGroup(string.IsNullOrEmpty(selectedFilePath) || !File.Exists(selectedFilePath));
        if (GUILayout.Button("Create Backup", GUILayout.Height(30)))
        {
            CreateBackup();
        }
        EditorGUI.EndDisabledGroup();
        
        if (GUILayout.Button("Clean Old Backups", GUILayout.Height(30)))
        {
            if (ErrorHandler.ShowConfirmation("Clean Old Backups", 
                "Delete backups older than 30 days?"))
            {
                VersionManager.CleanOldBackups();
            }
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Version History
        EditorGUILayout.LabelField("Version History", EditorStyles.boldLabel);
        
        if (versionHistory.Count == 0)
        {
            EditorGUILayout.HelpBox("No backup history found for this file.", MessageType.Info);
        }
        else
        {
            scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
            
            foreach (var backup in versionHistory)
            {
                EditorGUILayout.BeginVertical(EditorStyles.helpBox);
                EditorGUILayout.BeginHorizontal();
                
                EditorGUILayout.LabelField(backup.CreatedAt.ToString("yyyy-MM-dd HH:mm:ss"), GUILayout.Width(150));
                EditorGUILayout.LabelField(FormatFileSize(backup.FileSize), GUILayout.Width(100));
                
                GUILayout.FlexibleSpace();
                
                if (GUILayout.Button("Restore", GUILayout.Width(80)))
                {
                    RestoreBackup(backup);
                }
                
                EditorGUILayout.EndHorizontal();
                EditorGUILayout.EndVertical();
            }
            
            EditorGUILayout.EndScrollView();
        }
    }
    
    private void CreateBackup()
    {
        if (string.IsNullOrEmpty(selectedFilePath) || !File.Exists(selectedFilePath))
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.FileNotFound, selectedFilePath);
            return;
        }
        
        string backupPath = VersionManager.CreateBackup(selectedFilePath);
        if (!string.IsNullOrEmpty(backupPath))
        {
            ErrorHandler.ShowSuccess($"Backup created: {Path.GetFileName(backupPath)}");
            LoadVersionHistory();
        }
    }
    
    private void RestoreBackup(VersionManager.BackupInfo backup)
    {
        if (!ErrorHandler.ShowConfirmation("Restore Backup", 
            $"Restore this backup? Current file will be overwritten."))
        {
            return;
        }
        
        if (VersionManager.RestoreBackup(backup.FilePath, selectedFilePath))
        {
            ErrorHandler.ShowSuccess("Backup restored successfully!");
            LoadVersionHistory();
        }
    }
    
    private void LoadVersionHistory()
    {
        if (string.IsNullOrEmpty(selectedFilePath))
        {
            versionHistory.Clear();
            return;
        }
        
        versionHistory = VersionManager.GetVersionHistory(selectedFilePath);
    }
    
    private string FormatFileSize(long bytes)
    {
        string[] sizes = { "B", "KB", "MB", "GB" };
        double len = bytes;
        int order = 0;
        while (len >= 1024 && order < sizes.Length - 1)
        {
            order++;
            len = len / 1024;
        }
        return $"{len:0.##} {sizes[order]}";
    }
}

