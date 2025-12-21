using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Preset customization and management window
/// </summary>
public class PresetManagerWindow : EditorWindow
{
    private Vector2 scrollPosition;
    private List<CustomPreset> presets = new List<CustomPreset>();
    private CustomPreset selectedPreset = null;
    private bool showPresetEditor = false;
    private string newPresetName = "";
    private string newPresetContext = "";
    
    [Serializable]
    public class CustomPreset
    {
        public string name;
        public string displayName;
        public string context;
        public string filePath;
        public DateTime createdAt;
        public DateTime lastModified;
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Tools/Preset Manager", false, 3020)]
    public static void ShowWindow()
    {
        var window = GetWindow<PresetManagerWindow>("Preset Manager");
        window.minSize = new Vector2(700, 500);
        window.Show();
    }
    
    private void OnEnable()
    {
        LoadPresets();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Preset Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Create and manage custom presets for item generation. " +
            "Presets define the world context that influences generated items.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Create New Preset
        EditorGUILayout.LabelField("Create New Preset", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        newPresetName = EditorGUILayout.TextField("Preset Name:", newPresetName);
        newPresetContext = EditorGUILayout.TextArea(newPresetContext, GUILayout.Height(100));
        newPresetContext = EditorGUILayout.TextArea(newPresetContext, GUILayout.Height(100));
        
        if (GUILayout.Button("Create Preset", GUILayout.Height(30)))
        {
            CreatePreset();
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Preset List
        EditorGUILayout.LabelField("Custom Presets", EditorStyles.boldLabel);
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        foreach (var preset in presets)
        {
            DrawPresetItem(preset);
        }
        
        EditorGUILayout.EndScrollView();
        
        // Preset Editor
        if (selectedPreset != null && showPresetEditor)
        {
            DrawPresetEditor();
        }
    }
    
    private void DrawPresetItem(CustomPreset preset)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        bool isSelected = selectedPreset == preset;
        bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
        
        if (newSelected != isSelected)
        {
            selectedPreset = newSelected ? preset : null;
            showPresetEditor = newSelected;
        }
        
        EditorGUILayout.LabelField(preset.displayName, EditorStyles.boldLabel, GUILayout.Width(200));
        EditorGUILayout.LabelField($"Created: {preset.createdAt:yyyy-MM-dd}", GUILayout.Width(150));
        
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Edit", GUILayout.Width(60)))
        {
            selectedPreset = preset;
            showPresetEditor = true;
        }
        
        if (GUILayout.Button("Delete", GUILayout.Width(60)))
        {
            if (ErrorHandler.ShowConfirmation("Delete Preset", 
                $"Delete preset '{preset.displayName}'?"))
            {
                DeletePreset(preset);
            }
        }
        
        EditorGUILayout.EndHorizontal();
        
        if (isSelected)
        {
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Context:", EditorStyles.miniLabel);
            EditorGUILayout.TextArea(preset.context, EditorStyles.wordWrappedLabel, GUILayout.Height(50));
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawPresetEditor()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Edit Preset", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        selectedPreset.displayName = EditorGUILayout.TextField("Display Name:", selectedPreset.displayName);
        selectedPreset.context = EditorGUILayout.TextArea(selectedPreset.context, GUILayout.Height(150));
        
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Save", GUILayout.Height(30)))
        {
            SavePreset(selectedPreset);
        }
        if (GUILayout.Button("Cancel", GUILayout.Height(30)))
        {
            selectedPreset = null;
            showPresetEditor = false;
            LoadPresets();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
    }
    
    private void CreatePreset()
    {
        if (string.IsNullOrEmpty(newPresetName) || string.IsNullOrEmpty(newPresetContext))
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.Unknown, "Preset name and context are required.");
            return;
        }
        
        string presetDir = GetPresetDirectory();
        if (!Directory.Exists(presetDir))
        {
            Directory.CreateDirectory(presetDir);
        }
        
        string fileName = newPresetName.ToLower().Replace(" ", "_") + ".json";
        string filePath = Path.Combine(presetDir, fileName);
        
        var preset = new CustomPreset
        {
            name = newPresetName.ToLower().Replace(" ", "_"),
            displayName = newPresetName,
            context = newPresetContext,
            filePath = filePath,
            createdAt = DateTime.Now,
            lastModified = DateTime.Now
        };
        
        SavePreset(preset);
        presets.Add(preset);
        
        newPresetName = "";
        newPresetContext = "";
        
        ErrorHandler.ShowSuccess($"Preset '{preset.displayName}' created successfully!");
    }
    
    private void SavePreset(CustomPreset preset)
    {
        try
        {
            preset.lastModified = DateTime.Now;
            
            var presetData = new PresetData
            {
                name = preset.name,
                displayName = preset.displayName,
                context = preset.context
            };
            
            string json = JsonUtility.ToJson(presetData, true);
            File.WriteAllText(preset.filePath, json);
            
            Debug.Log($"[PresetManager] Saved preset: {preset.filePath}");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.PermissionDenied, ex.Message);
        }
    }
    
    private void DeletePreset(CustomPreset preset)
    {
        try
        {
            if (File.Exists(preset.filePath))
            {
                File.Delete(preset.filePath);
            }
            
            presets.Remove(preset);
            if (selectedPreset == preset)
            {
                selectedPreset = null;
                showPresetEditor = false;
            }
            
            ErrorHandler.ShowSuccess($"Preset '{preset.displayName}' deleted.");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.PermissionDenied, ex.Message);
        }
    }
    
    private void LoadPresets()
    {
        presets.Clear();
        
        string presetDir = GetPresetDirectory();
        if (!Directory.Exists(presetDir))
        {
            return;
        }
        
        var presetFiles = Directory.GetFiles(presetDir, "*.json");
        foreach (var file in presetFiles)
        {
            try
            {
                string json = File.ReadAllText(file);
                var presetData = JsonUtility.FromJson<PresetData>(json);
                
                var fileInfo = new FileInfo(file);
                var preset = new CustomPreset
                {
                    name = presetData.name,
                    displayName = presetData.displayName,
                    context = presetData.context,
                    filePath = file,
                    createdAt = fileInfo.CreationTime,
                    lastModified = fileInfo.LastWriteTime
                };
                
                presets.Add(preset);
            }
            catch (Exception ex)
            {
                Debug.LogError($"[PresetManager] Failed to load preset {file}: {ex.Message}");
            }
        }
    }
    
    private string GetPresetDirectory()
    {
        return Path.Combine(Application.dataPath, "..", "Deployment", "Presets", "Custom");
    }
    
    [Serializable]
    private class PresetData
    {
        public string name;
        public string displayName;
        public string context;
    }
}

