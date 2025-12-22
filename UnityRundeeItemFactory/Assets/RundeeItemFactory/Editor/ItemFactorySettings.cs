using System;
using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Built-in preset types for item generation
/// Each preset provides different world context and flavor text for LLM prompts.
/// </summary>
public enum PresetType
{
    Default,  ///< Default/generic survival game setting
    Forest,   ///< Forest/wilderness survival setting
    Desert,   ///< Desert/arid survival setting
    Coast,    ///< Coastal/beach survival setting
    City      ///< Urban/city survival setting
}

/// <summary>
/// ScriptableObject-based settings for Item Factory
/// Provides persistent settings that can be shared across team members
/// </summary>
[CreateAssetMenu(fileName = "ItemFactorySettings", menuName = "Rundee/Item Factory Settings", order = 1)]
public class ItemFactorySettings : ScriptableObject
{
    private static ItemFactorySettings _instance;
    private static string SettingsPath => "Assets/RundeeItemFactory/Editor/ItemFactorySettings.asset";

    /// <summary>
    /// Get or create the settings instance
    /// </summary>
    public static ItemFactorySettings Instance
    {
        get
        {
            if (_instance == null)
            {
                _instance = AssetDatabase.LoadAssetAtPath<ItemFactorySettings>(SettingsPath);
                
                if (_instance == null)
                {
                    // Create default settings
                    _instance = CreateInstance<ItemFactorySettings>();
                    _instance.InitializeDefaults();
                    
                    // Ensure directory exists
                    string directory = Path.GetDirectoryName(SettingsPath);
                    if (!Directory.Exists(directory))
                    {
                        Directory.CreateDirectory(directory);
                    }
                    
                    AssetDatabase.CreateAsset(_instance, SettingsPath);
                    AssetDatabase.SaveAssets();
                }
            }
            
            return _instance;
        }
    }

    [Header("Generation Settings")]
    [Tooltip("Default model to use for item generation")]
    public string defaultModel = "llama3";
    
    [Tooltip("Default preset type")]
    public PresetType defaultPreset = PresetType.Default;
    
    [Tooltip("Default item count")]
    [Range(1, 100)]
    public int defaultItemCount = 10;
    
    [Tooltip("Default max hunger value")]
    [Range(0, 200)]
    public int defaultMaxHunger = 100;
    
    [Tooltip("Default max thirst value")]
    [Range(0, 200)]
    public int defaultMaxThirst = 100;

    [Header("Output Settings")]
    [Tooltip("Default output directory (relative to project root)")]
    public string defaultOutputDirectory = "ItemJson";
    
    [Tooltip("Auto-import items after generation")]
    public bool autoImportAfterGeneration = true;
    
    [Tooltip("Auto-clean raw JSON files after generation")]
    public bool autoCleanRawFiles = true;

    [Header("Executable Settings")]
    [Tooltip("Path to RundeeItemFactory.exe (relative to project root or absolute)")]
    public string executablePath = "";

    [Header("Advanced Settings")]
    [Tooltip("Use test mode by default")]
    public bool useTestMode = false;
    
    [Tooltip("Enable verbose logging")]
    public bool verboseLogging = false;
    
    [Tooltip("Timeout for LLM requests (seconds)")]
    [Range(30, 600)]
    public int requestTimeoutSeconds = 120;
    
    [Tooltip("Maximum retry attempts")]
    [Range(0, 10)]
    public int maxRetries = 3;

    [Header("UI Settings")]
    [Tooltip("Show setup section by default")]
    public bool showSetupSection = true;
    
    [Tooltip("Show advanced options by default")]
    public bool showAdvancedOptions = false;
    
    [Tooltip("Show balance report section by default")]
    public bool showBalanceReport = false;

    /// <summary>
    /// Initialize default values
    /// </summary>
    private void InitializeDefaults()
    {
        defaultModel = "llama3";
        defaultPreset = PresetType.Default;
        defaultItemCount = 10;
        defaultMaxHunger = 100;
        defaultMaxThirst = 100;
        defaultOutputDirectory = "ItemJson";
        autoImportAfterGeneration = true;
        autoCleanRawFiles = true;
        executablePath = "";
        useTestMode = false;
        verboseLogging = false;
        requestTimeoutSeconds = 120;
        maxRetries = 3;
        showSetupSection = true;
        showAdvancedOptions = false;
        showBalanceReport = false;
    }

    /// <summary>
    /// Save settings to disk
    /// </summary>
    public void Save()
    {
        EditorUtility.SetDirty(this);
        AssetDatabase.SaveAssets();
    }

    /// <summary>
    /// Reset to default values
    /// </summary>
    [ContextMenu("Reset to Defaults")]
    public void ResetToDefaults()
    {
        InitializeDefaults();
        Save();
    }

    /// <summary>
    /// Get full executable path (resolves relative paths)
    /// </summary>
    public string GetFullExecutablePath()
    {
        if (string.IsNullOrEmpty(executablePath))
        {
            return "";
        }

        // If absolute path, return as-is
        if (Path.IsPathRooted(executablePath))
        {
            return executablePath;
        }

        // Resolve relative to project root
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        return Path.Combine(projectRoot, executablePath);
    }

    /// <summary>
    /// Get full output directory path
    /// </summary>
    public string GetFullOutputDirectory()
    {
        if (string.IsNullOrEmpty(defaultOutputDirectory))
        {
            defaultOutputDirectory = "ItemJson";
        }

        string projectRoot = Application.dataPath.Replace("/Assets", "");
        return Path.Combine(projectRoot, defaultOutputDirectory);
    }

    /// <summary>
    /// Validate settings
    /// </summary>
    public bool Validate(out string errorMessage)
    {
        if (string.IsNullOrEmpty(defaultModel))
        {
            errorMessage = "Default model cannot be empty";
            return false;
        }

        if (defaultItemCount < 1 || defaultItemCount > 100)
        {
            errorMessage = "Default item count must be between 1 and 100";
            return false;
        }

        if (defaultMaxHunger < 0 || defaultMaxHunger > 200)
        {
            errorMessage = "Default max hunger must be between 0 and 200";
            return false;
        }

        if (defaultMaxThirst < 0 || defaultMaxThirst > 200)
        {
            errorMessage = "Default max thirst must be between 0 and 200";
            return false;
        }

        errorMessage = "";
        return true;
    }
}

/// <summary>
/// Custom editor for ItemFactorySettings
/// </summary>
[CustomEditor(typeof(ItemFactorySettings))]
public class ItemFactorySettingsEditor : Editor
{
    public override void OnInspectorGUI()
    {
        ItemFactorySettings settings = (ItemFactorySettings)target;

        EditorGUILayout.Space();
        EditorGUILayout.HelpBox("These settings are shared across all users of this project. " +
            "Changes are saved automatically.", MessageType.Info);
        EditorGUILayout.Space();

        DrawDefaultInspector();

        EditorGUILayout.Space();
        EditorGUILayout.BeginHorizontal();
        
        if (GUILayout.Button("Reset to Defaults"))
        {
            if (EditorUtility.DisplayDialog("Reset Settings", 
                "Are you sure you want to reset all settings to defaults?", 
                "Yes", "No"))
            {
                settings.ResetToDefaults();
            }
        }

        if (GUILayout.Button("Validate Settings"))
        {
            if (settings.Validate(out string error))
            {
                EditorUtility.DisplayDialog("Validation", "All settings are valid!", "OK");
            }
            else
            {
                EditorUtility.DisplayDialog("Validation Error", error, "OK");
            }
        }

        EditorGUILayout.EndHorizontal();

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Paths:", EditorStyles.boldLabel);
        EditorGUILayout.LabelField("Executable:", settings.GetFullExecutablePath(), EditorStyles.wordWrappedLabel);
        EditorGUILayout.LabelField("Output Directory:", settings.GetFullOutputDirectory(), EditorStyles.wordWrappedLabel);
    }
}






