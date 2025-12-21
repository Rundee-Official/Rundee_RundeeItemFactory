using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEditorInternal;
using UnityEngine;

public enum PresetType
{
    Default,
    Forest,
    Desert,
    Coast,
    City
}

public enum ModelType
{
    Llama3,
    Llama2,
    Mistral,
    Gemma,
    Phi3,
    Qwen,
    Custom
}

/// <summary>
/// Unity Editor window for generating game items using LLM
/// </summary>
/// <remarks>
/// Provides a GUI interface for:
/// - Configuring item generation parameters
/// - Selecting LLM models and presets
/// - Batch generation of multiple item types
/// - Auto-importing generated items into Unity ScriptableObjects
/// - Managing custom presets
/// - Generating balance reports
/// </remarks>
public class ItemFactoryWindow : EditorWindow
{
    /// <summary>
    /// Log entry with color information for colored log display
    /// </summary>
    private class LogEntry
    {
        public string message;
        public LogType type;
        public DateTime timestamp;
        
        public LogEntry(string msg, LogType logType = LogType.Log)
        {
            message = msg;
            type = logType;
            timestamp = DateTime.Now;
        }
    }
    
    /// <summary>
    /// Log entry type for color coding
    /// </summary>
    private enum LogType
    {
        /// <summary>Standard log message</summary>
        Log,
        /// <summary>Warning message (orange)</summary>
        Warning,
        /// <summary>Error message (red)</summary>
        Error,
        /// <summary>Success message (green)</summary>
        Success,
        /// <summary>Info message (blue)</summary>
        Info
    }
    // Debug logging disabled for release

    // Configuration
    private string executablePath = "";
    private string defaultExecutablePath = "";

    // Generation Parameters
    private ModelType selectedModel = ModelType.Llama3;
    private string customModelName = "";
    private PresetType selectedPreset = PresetType.Default;
    private ItemType selectedItemType = ItemType.Food;
    private int itemCount = 10;
    private int maxHunger = 100;
    private int maxThirst = 100;
    private string outputPath = "";
    private string additionalPrompt = "";  // User-defined additional prompt
    private bool useTestMode = false;  // Test mode: outputs to Test/ folder instead of ItemJson/
    
    // Generation Profile
    private ItemGenerationProfile selectedProfile = null;
    private bool useProfile = false;

    // UI State
    private Vector2 mainScrollPosition;  // Main window scroll position
    private Vector2 logScrollPosition;
    private string logText = "";
    private List<LogEntry> logEntries = new List<LogEntry>();
    private bool isGenerating = false;
    
    // Progress tracking
    private int generatedItemCount = 0;
    private int targetItemCount = 0;
    private Dictionary<ItemType, int> batchProgress = new Dictionary<ItemType, int>();
    
    // Settings persistence
    private const string PREFS_KEY_PREFIX = "RundeeItemFactory_";
    private bool autoCleanRawFiles = true;
    private Process currentProcess = null;
    private bool autoImportAfterGeneration = true;
    private bool isBatchGeneration = false;  // Track if current generation is batch mode

    // Setup / Ollama detection
    private bool showSetupSection = true;
    private bool ollamaInstalled = false;
    private string ollamaStatus = "Not checked";
    private string ollamaVersion = "";
    private string ollamaCheckError = "";
    private string ollamaSetupScriptPath = "";
    private bool? ollamaExistsInDefaultPath = null;
    private string ollamaDetectedPath = "";
    private string ollamaDetectMessage = "";

    // Tab System
    private enum TabType
    {
        Settings,
        SingleGeneration,
        BatchGeneration,
        BalanceReport,
        PresetManager
    }
    private TabType selectedTab = TabType.Settings;
    private readonly string[] tabNames = { "Settings", "Single Generation", "Batch Generation", "Balance Report", "Preset Manager" };
    
    // UI Foldouts
    private bool showConfigSection = true;
    private bool showGenerationParams = true;
    private bool showAdvancedOptions = false;
    
    // Separate log areas for each generation tab
    private Vector2 singleGenerationLogScrollPosition;
    private List<LogEntry> singleGenerationLogEntries = new List<LogEntry>();
    private Vector2 batchGenerationLogScrollPosition;
    private List<LogEntry> batchGenerationLogEntries = new List<LogEntry>();

    // Balance Report
    private string balanceReportJsonPath = "";
    private ItemType balanceReportItemType = ItemType.Food;
    private Vector2 balanceReportScrollPosition;
    private string balanceReportText = "";

    // Batch Generation
    [System.Serializable]
    public class BatchItem
    {
        public ItemType itemType = ItemType.Food;
        public int count = 10;
        public string outputPath = "";
    }
    private List<BatchItem> batchItems = new List<BatchItem>();
    private Vector2 batchScrollPosition;
    private ReorderableList batchItemsList;

    // Weapon component editing
    private WeaponComponentItemDataSO selectedWeaponComponentAsset;
    private SerializedObject weaponComponentSerializedObject;
    private ReorderableList loadedRoundsList;
    private bool showLoadPlanEditor = true;

    // Preset options
    private readonly string[] presetNames = { "Default", "Forest", "Desert", "Coast", "City" };
    private readonly PresetType[] presetValues = { PresetType.Default, PresetType.Forest, PresetType.Desert, PresetType.Coast, PresetType.City };
    
    // Custom Preset Management
    [System.Serializable]
    public class CustomPresetData
    {
        public string id = "";
        public string displayName = "";
        public string description = "";
        public string flavorText = "";
        public string author = "";
        public string version = "1.0";
        public List<string> tags = new List<string>();
    }
    private List<CustomPresetData> customPresets = new List<CustomPresetData>();
    private CustomPresetData editingPreset = null;
    private bool useCustomPreset = false;
    private string customPresetPath = "";
    private Vector2 presetManagerScrollPosition;

    // Model options
    private readonly string[] modelNames = { "Llama 3", "Llama 2", "Mistral", "Gemma", "Phi-3", "Qwen", "Custom" };
    private readonly ModelType[] modelValues = { ModelType.Llama3, ModelType.Llama2, ModelType.Mistral, ModelType.Gemma, ModelType.Phi3, ModelType.Qwen, ModelType.Custom };

    // Item type options
    private readonly string[] itemTypeNames = { "Food", "Drink", "Medicine", "Material", "Weapon", "Weapon Component", "Ammo", "Armor", "Clothing" };
    private readonly ItemType[] itemTypeValues = { ItemType.Food, ItemType.Drink, ItemType.Medicine, ItemType.Material, ItemType.Weapon, ItemType.WeaponComponent, ItemType.Ammo, ItemType.Armor, ItemType.Clothing };

    private string GetModelName()
    {
        if (selectedModel == ModelType.Custom)
        {
            return string.IsNullOrEmpty(customModelName) ? "llama3" : customModelName;
        }
        
        switch (selectedModel)
        {
            case ModelType.Llama3: return "llama3";
            case ModelType.Llama2: return "llama2";
            case ModelType.Mistral: return "mistral";
            case ModelType.Gemma: return "gemma";
            case ModelType.Phi3: return "phi3";
            case ModelType.Qwen: return "qwen";
            default: return "llama3";
        }
    }

    [MenuItem("Tools/Rundee/Item Factory/Generation/Item Factory Window", false, 1000)]
    public static void ShowWindow()
    {
        ItemFactoryWindow window = GetWindow<ItemFactoryWindow>("Item Factory");
        window.minSize = new Vector2(500, 600);
        window.InitializeExecutablePath();
        window.InitializeOutputPath();
        window.InitializeSetupPaths();
        window.CheckOllamaStatus();
    }

    /// <summary>
    /// Public static method to generate items programmatically from other windows
    /// </summary>
    public static bool GenerateItemsAsync(ItemType itemType, int count, string outputPath, 
        string modelName = "llama3", PresetType? preset = null, string customPresetPath = null,
        Action<bool, string> onComplete = null)
    {
        var window = GetWindow<ItemFactoryWindow>("Item Factory", false);
        if (window == null)
        {
            window = CreateInstance<ItemFactoryWindow>();
        }

        if (window.isGenerating)
        {
            UnityEngine.Debug.LogWarning("[ItemFactoryWindow] Generation already in progress.");
            onComplete?.Invoke(false, "Generation already in progress");
            return false;
        }

        // Set parameters
        window.selectedItemType = itemType;
        window.itemCount = count;
        window.outputPath = outputPath;
        window.customModelName = modelName;
        window.selectedModel = ModelType.Custom;
        
        if (customPresetPath != null)
        {
            window.useCustomPreset = true;
            window.customPresetPath = customPresetPath;
        }
        else if (preset.HasValue)
        {
            window.useCustomPreset = false;
            window.selectedPreset = preset.Value;
        }

        // Find executable
        window.InitializeExecutablePath();

        if (string.IsNullOrEmpty(window.executablePath) || !File.Exists(window.executablePath))
        {
            UnityEngine.Debug.LogError("[ItemFactoryWindow] Executable not found. Please set the path in Item Factory Window.");
            onComplete?.Invoke(false, "Executable not found");
            return false;
        }

        // Store callback
        window.generationCompleteCallback = onComplete;

        // Start generation
        window.GenerateItems();
        return true;
    }

    private Action<bool, string> generationCompleteCallback = null;

    private void OnEnable()
    {
        InitializeExecutablePath();
        InitializeOutputPath();
        InitializeSetupPaths();
        CheckOllamaStatus();
        LoadSettings();
    }
    
    private void OnDisable()
    {
        SaveSettings();
    }

    private void InitializeOutputPath()
    {
        if (string.IsNullOrEmpty(outputPath))
        {
            UpdateOutputPathForItemType();
        }
    }

    private void InitializeSetupPaths()
    {
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string defaultScript = Path.Combine(projectRoot, "Assets", "RundeeItemFactory", "Editor", "OllamaSetup", "install_ollama_windows.bat");
        ollamaSetupScriptPath = defaultScript;
    }

    private void UpdateOutputPathForItemType()
    {
        // Default to ItemJson or Test folder in project root with a descriptive filename
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string defaultFileName = $"items_{selectedItemType.ToString().ToLower()}.json";
        string outputDir = useTestMode ? "Test" : "ItemJson";
        outputPath = Path.Combine(projectRoot, outputDir, defaultFileName);
    }

    /// <summary>
    /// Get default output path for a batch item
    /// </summary>
    private string GetDefaultBatchOutputPath(ItemType itemType)
    {
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string itemTypeName = itemType.ToString().ToLower();
        
        // Handle WeaponComponent -> weaponcomponent conversion
        if (itemType == ItemType.WeaponComponent)
        {
            itemTypeName = "weaponcomponent";
        }
        
        string defaultFileName = $"items_{itemTypeName}.json";
        string outputDir = useTestMode ? "Test" : "ItemJson";
        return Path.Combine(projectRoot, outputDir, defaultFileName);
    }

    private void InitializeExecutablePath()
    {
        // Try to find the executable relative to Unity project
        // Auto-detecting executable path
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string[] possiblePaths = {
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Debug", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Release", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Debug", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Release", "RundeeItemFactory.exe")
        };

        foreach (var path in possiblePaths)
        {
            if (File.Exists(path))
            {
                executablePath = path;
                defaultExecutablePath = path;
                // Executable found
                break;
            }
        }

        if (string.IsNullOrEmpty(executablePath))
        {
            AddLog("[Warning] Executable not found in standard locations.");
            // Executable not found
        }
    }

    private void OnGUI()
    {
        // Main scroll view for entire window
        mainScrollPosition = EditorGUILayout.BeginScrollView(mainScrollPosition);
        
        EditorGUILayout.Space(10);

        // Title with improved styling
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        GUIStyle titleStyle = new GUIStyle(EditorStyles.boldLabel);
        titleStyle.fontSize = 20;
        titleStyle.alignment = TextAnchor.MiddleCenter;
        EditorGUILayout.LabelField("Rundee Item Factory", titleStyle);
        
        EditorGUILayout.Space(3);
        EditorGUILayout.LabelField("LLM-Powered Item Generator for Unity", EditorStyles.centeredGreyMiniLabel);
        EditorGUILayout.EndVertical();
        EditorGUILayout.Space(5);

        // Tab Selection
        EditorGUILayout.BeginHorizontal();
        for (int i = 0; i < tabNames.Length; i++)
        {
            TabType tab = (TabType)i;
            bool isSelected = selectedTab == tab;
            GUI.backgroundColor = isSelected ? Color.white : Color.grey;
            if (GUILayout.Button(tabNames[i], GUILayout.Height(25)))
            {
                selectedTab = tab;
            }
        }
        GUI.backgroundColor = Color.white;
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        EditorGUILayout.Space(5);

        // Render selected tab content
        switch (selectedTab)
        {
            case TabType.Settings:
                DrawSettingsTab();
                break;
            case TabType.SingleGeneration:
                DrawSingleGenerationTab();
                break;
            case TabType.BatchGeneration:
                DrawBatchGenerationTab();
                break;
            case TabType.BalanceReport:
                DrawBalanceReportTab();
                break;
            case TabType.PresetManager:
                DrawPresetManagerTab();
                break;
        }
        
        // End main scroll view
        EditorGUILayout.EndScrollView();
    }

    /// <summary>
    /// Draw Settings tab - Configuration and Generation Parameters
    /// </summary>
    private void DrawSettingsTab()
    {
        // Setup / Ollama installer
        showSetupSection = EditorGUILayout.Foldout(showSetupSection, "Setup (Ollama)", true);
        if (showSetupSection)
        {
            EditorGUI.indentLevel++;
            EditorGUILayout.HelpBox("Check whether Ollama is installed and run the bundled installer (.bat) if missing. Windows only.", MessageType.Info);
            EditorGUILayout.LabelField("Status:", string.IsNullOrEmpty(ollamaStatus) ? (ollamaInstalled ? "Installed" : "Not detected") : ollamaStatus);
            if (!string.IsNullOrEmpty(ollamaVersion))
            {
                EditorGUILayout.LabelField("Version:", ollamaVersion);
            }
            if (!string.IsNullOrEmpty(ollamaCheckError))
            {
                EditorGUILayout.HelpBox(ollamaCheckError, MessageType.Warning);
            }

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Check Ollama (default paths)", GUILayout.Height(22)))
            {
                CheckOllamaInDefaultPaths();
            }
            if (GUILayout.Button("Open Ollama Download Page", GUILayout.Height(22)))
            {
                Application.OpenURL("https://ollama.com/download");
            }
            EditorGUILayout.EndHorizontal();
            if (ollamaExistsInDefaultPath.HasValue)
            {
                var color = ollamaExistsInDefaultPath.Value ? Color.green : Color.red;
                var oldColor = GUI.color;
                GUI.color = color;
                EditorGUILayout.LabelField(ollamaDetectMessage);
                GUI.color = oldColor;
            }
            else
            {
                EditorGUILayout.LabelField("Not checked yet.");
            }

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Re-check Ollama", GUILayout.Height(25)))
            {
                CheckOllamaStatus();
            }
            GUI.enabled = File.Exists(ollamaSetupScriptPath);
            if (GUILayout.Button("Install Ollama (run bundled .bat)", GUILayout.Height(25)))
            {
                RunOllamaInstaller();
            }
            GUI.enabled = true;
            EditorGUILayout.EndHorizontal();

            EditorGUI.indentLevel--;
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        }

        // Configuration Section
        showConfigSection = EditorGUILayout.Foldout(showConfigSection, "Configuration", true);
        if (showConfigSection)
        {
            EditorGUI.indentLevel++;
            
            // Executable status (auto-detected; path not shown)
            if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
            {
                EditorGUILayout.HelpBox("Executable not found in standard locations. Please build the C++ solution.", MessageType.Warning);
            }
            else
            {
                EditorGUILayout.HelpBox("Executable auto-detected and ready.", MessageType.Info);
            }

            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Generation Parameters
        showGenerationParams = EditorGUILayout.Foldout(showGenerationParams, "Generation Parameters", true);
        if (showGenerationParams)
        {
            EditorGUI.indentLevel++;

            // Model Selection
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Model:", GUILayout.Width(120));
            int modelIndex = Array.IndexOf(modelValues, selectedModel);
            if (modelIndex < 0) modelIndex = 0;
            modelIndex = EditorGUILayout.Popup(modelIndex, modelNames);
            selectedModel = modelValues[modelIndex];
            EditorGUILayout.EndHorizontal();

            // Custom Model Name (only show if Custom is selected)
            if (selectedModel == ModelType.Custom)
            {
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Custom Model:", GUILayout.Width(120));
                customModelName = EditorGUILayout.TextField(customModelName);
                EditorGUILayout.EndHorizontal();
                
                if (string.IsNullOrEmpty(customModelName))
                {
                    EditorGUILayout.HelpBox("Please enter a model name (e.g., 'llama3:8b', 'mistral:7b')", MessageType.Info);
                }
            }

            // Preset Selection
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Preset:", GUILayout.Width(120));
            useCustomPreset = EditorGUILayout.Toggle("Use Custom", useCustomPreset);
            EditorGUILayout.EndHorizontal();
            
            if (useCustomPreset)
            {
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Custom Preset:", GUILayout.Width(120));
                
                GUI.SetNextControlName("customPresetPath");
                customPresetPath = EditorGUILayout.TextField(customPresetPath);
                
                if (GUILayout.Button("Browse", GUILayout.Width(60)))
                {
                    string defaultDir = string.IsNullOrEmpty(customPresetPath) 
                        ? Application.dataPath 
                        : Path.GetDirectoryName(customPresetPath);
                    string path = EditorUtility.OpenFilePanel("Select Custom Preset", defaultDir, "json");
                    if (!string.IsNullOrEmpty(path))
                    {
                        customPresetPath = path;
                        SaveRecentPresetPath(path);
                    }
                }
                EditorGUILayout.EndHorizontal();
                
                // Show recent presets suggestion
                if (!string.IsNullOrEmpty(customPresetPath) && GUI.GetNameOfFocusedControl() == "customPresetPath")
                {
                    string[] recentPresets = GetRecentPresetPaths();
                    if (recentPresets.Length > 0)
                    {
                        string[] suggestions = recentPresets.Where(p => 
                            Path.GetFileName(p).IndexOf(customPresetPath, StringComparison.OrdinalIgnoreCase) >= 0 ||
                            p.IndexOf(customPresetPath, StringComparison.OrdinalIgnoreCase) >= 0)
                            .Take(3).ToArray();
                        if (suggestions.Length > 0)
                        {
                            EditorGUILayout.HelpBox($"Recent: {string.Join(", ", suggestions.Select(Path.GetFileName))}", MessageType.None);
                        }
                    }
                }
                
                EditorGUILayout.HelpBox("Select a custom preset JSON file. The file must contain id, displayName, description, and flavorText fields.", MessageType.Info);
            }
            else
            {
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Built-in Preset:", GUILayout.Width(120));
                int presetIndex = Array.IndexOf(presetValues, selectedPreset);
                if (presetIndex < 0) presetIndex = 0;
                presetIndex = EditorGUILayout.Popup(presetIndex, presetNames);
                selectedPreset = presetValues[presetIndex];
                EditorGUILayout.EndHorizontal();
                EditorGUILayout.HelpBox(GetPresetDescription(selectedPreset), MessageType.None);
            }

            // Item Type
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Item Type:", GUILayout.Width(120));
            int itemTypeIndex = Array.IndexOf(itemTypeValues, selectedItemType);
            if (itemTypeIndex < 0) itemTypeIndex = 0;
            int newItemTypeIndex = EditorGUILayout.Popup(itemTypeIndex, itemTypeNames);
            ItemType newItemType = itemTypeValues[newItemTypeIndex];
            
            // Update output path if item type changed
            if (newItemType != selectedItemType)
            {
                selectedItemType = newItemType;
                UpdateOutputPathForItemType();
            }
            EditorGUILayout.EndHorizontal();

            // Test Mode Toggle
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Test Mode:", GUILayout.Width(120));
            bool newTestMode = EditorGUILayout.Toggle(useTestMode);
            if (newTestMode != useTestMode)
            {
                useTestMode = newTestMode;
                UpdateOutputPathForItemType();
            }
            EditorGUILayout.EndHorizontal();
            EditorGUILayout.HelpBox(useTestMode 
                ? "Test Mode: Outputs will be saved to Test/ folder. Use this for testing and development." 
                : "Normal Mode: Outputs will be saved to ItemJson/ folder. Use this for production.", 
                MessageType.Info);

            // Count
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Count:", GUILayout.Width(120));
            itemCount = EditorGUILayout.IntField(itemCount);
            itemCount = Mathf.Clamp(itemCount, 1, 200);
            EditorGUILayout.EndHorizontal();
            EditorGUILayout.HelpBox($"Will generate {itemCount} items. Recommended: 10-20 for testing, 50-100 for production.", MessageType.None);

            // Generation Profile
            EditorGUILayout.Space(5);
            EditorGUILayout.LabelField("Generation Profile", EditorStyles.boldLabel);
            useProfile = EditorGUILayout.Toggle("Use Character Profile", useProfile);
            
            if (useProfile)
            {
                EditorGUI.indentLevel++;
                selectedProfile = (ItemGenerationProfile)EditorGUILayout.ObjectField(
                    "Profile", 
                    selectedProfile, 
                    typeof(ItemGenerationProfile), 
                    false);
                
                if (selectedProfile != null)
                {
                    EditorGUILayout.HelpBox(selectedProfile.GetSummary(), MessageType.Info);
                    
                    EditorGUILayout.BeginHorizontal();
                    if (GUILayout.Button("Create New Profile", GUILayout.Width(150)))
                    {
                        CreateNewProfile();
                    }
                    if (GUILayout.Button("Edit Profile", GUILayout.Width(100)))
                    {
                        Selection.activeObject = selectedProfile;
                        EditorGUIUtility.PingObject(selectedProfile);
                    }
                    EditorGUILayout.EndHorizontal();
                }
                else
                {
                    EditorGUILayout.HelpBox("No profile selected. Click 'Create New Profile' to create one.", MessageType.Warning);
                    if (GUILayout.Button("Create New Profile", GUILayout.Width(150)))
                    {
                        CreateNewProfile();
                    }
                }
                EditorGUI.indentLevel--;
            }
            else
            {
                // Legacy: Max Hunger/Thirst (only for Food/Drink) - shown when profile is not used
                if (selectedItemType == ItemType.Food || selectedItemType == ItemType.Drink)
                {
                    EditorGUILayout.BeginHorizontal();
                    EditorGUILayout.LabelField("Max Hunger:", GUILayout.Width(120));
                    maxHunger = EditorGUILayout.IntField(maxHunger);
                    maxHunger = Mathf.Clamp(maxHunger, 1, 1000);
                    EditorGUILayout.EndHorizontal();

                    // Max Thirst
                    EditorGUILayout.BeginHorizontal();
                    EditorGUILayout.LabelField("Max Thirst:", GUILayout.Width(120));
                    maxThirst = EditorGUILayout.IntField(maxThirst);
                    maxThirst = Mathf.Clamp(maxThirst, 1, 1000);
                    EditorGUILayout.EndHorizontal();
                }
            }

            // Output Path with autocomplete
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Output File:", GUILayout.Width(120));
            
            GUI.SetNextControlName("outputPath");
            outputPath = EditorGUILayout.TextField(outputPath);
            
            if (GUILayout.Button("Browse", GUILayout.Width(60)))
            {
                string defaultName = $"items_{selectedItemType.ToString().ToLower()}";
                string defaultDir = string.IsNullOrEmpty(outputPath) 
                    ? Application.dataPath 
                    : Path.GetDirectoryName(outputPath);
                string path = EditorUtility.SaveFilePanel("Save JSON File", defaultDir, defaultName, "json");
                if (!string.IsNullOrEmpty(path))
                {
                    outputPath = path;
                    SaveRecentOutputPath(path);
                }
            }
            EditorGUILayout.EndHorizontal();
            
            // Show recent paths suggestion
            if (!string.IsNullOrEmpty(outputPath) && GUI.GetNameOfFocusedControl() == "outputPath")
            {
                string[] recentPaths = GetRecentOutputPaths();
                if (recentPaths.Length > 0)
                {
                    string[] suggestions = recentPaths.Where(p => 
                        Path.GetFileName(p).IndexOf(outputPath, StringComparison.OrdinalIgnoreCase) >= 0 ||
                        p.IndexOf(outputPath, StringComparison.OrdinalIgnoreCase) >= 0)
                        .Take(3).ToArray();
                    if (suggestions.Length > 0)
                    {
                        EditorGUILayout.HelpBox($"Recent: {string.Join(", ", suggestions.Select(Path.GetFileName))}", MessageType.None);
                    }
                }
            }

            // Show file status
            if (!string.IsNullOrEmpty(outputPath))
            {
                bool fileExists = File.Exists(outputPath);
                if (fileExists)
                {
                    FileInfo fileInfo = new FileInfo(outputPath);
                    EditorGUILayout.HelpBox($"File exists ({fileInfo.Length / 1024} KB). New items will be merged with existing items (duplicates by ID will be skipped).", MessageType.Info);
                }
                else
                {
                    EditorGUILayout.HelpBox("New file will be created.", MessageType.None);
                }
            }

            EditorGUILayout.Space(5);

            // Auto Import Option
            autoImportAfterGeneration = EditorGUILayout.Toggle("Auto Import After Generation", autoImportAfterGeneration);
            EditorGUILayout.HelpBox("Automatically import generated items into Unity ScriptableObjects after generation completes.", MessageType.None);

            EditorGUILayout.Space(5);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
            
            // Additional User Prompt
            EditorGUILayout.LabelField("Additional Prompt (Optional)", EditorStyles.boldLabel);
            EditorGUILayout.HelpBox("Add custom instructions or details to the generation prompt. This will be appended to the base prompt.", MessageType.Info);
            additionalPrompt = EditorGUILayout.TextArea(additionalPrompt, GUILayout.Height(80));
            if (GUILayout.Button("Clear Additional Prompt", GUILayout.Height(22)))
            {
                additionalPrompt = "";
            }

            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Advanced Options
        showAdvancedOptions = EditorGUILayout.Foldout(showAdvancedOptions, "Advanced Options", false);
        if (showAdvancedOptions)
        {
            EditorGUI.indentLevel++;
            EditorGUILayout.HelpBox("Advanced options for power users. Most users can ignore these settings.", MessageType.Info);
            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        if (selectedItemType == ItemType.WeaponComponent)
        {
            DrawWeaponComponentEditingSection();
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        }
    }

    /// <summary>
    /// Draw Single Generation tab - Settings reference + Generate Items button + Log + Validation
    /// </summary>
    private void DrawSingleGenerationTab()
    {
        EditorGUILayout.HelpBox("Configure generation settings in the Settings tab, then generate items here.", MessageType.Info);
        
        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        EditorGUILayout.Space(5);

        // Show current settings summary
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.LabelField("Current Settings", EditorStyles.boldLabel);
        EditorGUILayout.LabelField($"Item Type: {selectedItemType}", EditorStyles.miniLabel);
        EditorGUILayout.LabelField($"Count: {itemCount}", EditorStyles.miniLabel);
        EditorGUILayout.LabelField($"Model: {GetModelName()}", EditorStyles.miniLabel);
        EditorGUILayout.LabelField($"Preset: {(useCustomPreset ? "Custom" : selectedPreset.ToString())}", EditorStyles.miniLabel);
        EditorGUILayout.LabelField($"Output: {Path.GetFileName(outputPath)}", EditorStyles.miniLabel);
        EditorGUILayout.EndVertical();

        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        EditorGUILayout.Space(5);

        // Validation
        bool canGenerate = !isGenerating && !string.IsNullOrEmpty(executablePath) && File.Exists(executablePath);
        List<string> validationErrors = new List<string>();
        
        if (canGenerate)
        {
            // Validate settings using InputValidator
            var countValidation = InputValidator.ValidateItemCount(itemCount);
            if (!countValidation.IsValid)
                validationErrors.Add(countValidation.ErrorMessage);
            
            // Validate output path - use ValidateOutputDirectory which auto-creates directories
            var outputDirValidation = InputValidator.ValidateOutputDirectory(outputPath);
            if (!outputDirValidation.IsValid)
            {
                validationErrors.Add(outputDirValidation.ErrorMessage);
            }
            else
            {
                // Also validate file path format
                var outputValidation = InputValidator.ValidateFilePath(outputPath, false, ".json");
                if (!outputValidation.IsValid && outputValidation.ErrorMessage != "Directory does not exist")
                {
                    validationErrors.Add(outputValidation.ErrorMessage);
                }
            }
            
            if (selectedModel == ModelType.Custom)
            {
                var modelValidation = InputValidator.ValidateModelName(customModelName);
                if (!modelValidation.IsValid)
                    validationErrors.Add(modelValidation.ErrorMessage);
            }
            
            if (useCustomPreset)
            {
                var presetValidation = InputValidator.ValidatePresetPath(customPresetPath);
                if (!presetValidation.IsValid)
                    validationErrors.Add(presetValidation.ErrorMessage);
            }
            
            if (validationErrors.Count > 0)
            {
                EditorGUILayout.HelpBox("Validation Errors:\n" + string.Join("\n", validationErrors.Select(e => "• " + e)), MessageType.Error);
            }
        }
        else if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            EditorGUILayout.HelpBox("Executable not found. Please configure it in the Settings tab.", MessageType.Warning);
        }
        
        // Generate Button
        EditorGUILayout.Space(5);
        EditorGUILayout.BeginHorizontal();
        GUI.enabled = canGenerate && validationErrors.Count == 0;
        if (GUILayout.Button(isGenerating && !isBatchGeneration ? "Generating..." : "Generate Items", GUILayout.Height(35)))
        {
            GenerateItems();
        }
        GUI.enabled = true;

        // Cancel Button
        if (isGenerating && !isBatchGeneration && currentProcess != null)
        {
            if (GUILayout.Button("Cancel", GUILayout.Height(35)))
            {
                CancelGeneration();
            }
        }
        EditorGUILayout.EndHorizontal();

        // Progress indicator
        if (isGenerating && !isBatchGeneration)
        {
            EditorGUILayout.Space(5);
            Rect progressRect = EditorGUILayout.GetControlRect(false, 25);
            
            float progress = 0.5f;
            string progressText = "Generating items... (This may take a while)";
            
            if (targetItemCount > 0)
            {
                progress = Mathf.Clamp01((float)generatedItemCount / targetItemCount);
                progressText = $"Generating items... {generatedItemCount}/{targetItemCount} ({progress * 100:F0}%)";
            }
            else
            {
                progress = Mathf.PingPong((float)EditorApplication.timeSinceStartup * 0.5f, 1f);
            }
            
            EditorGUI.ProgressBar(progressRect, progress, progressText);
            
            var oldColor = GUI.color;
            GUI.color = new Color(0.2f, 0.8f, 0.2f);
            EditorGUILayout.LabelField("● Processing...", EditorStyles.miniLabel);
            GUI.color = oldColor;
        }
        else if (!isGenerating && singleGenerationLogEntries.Any(e => e.type == LogType.Success))
        {
            var oldColor = GUI.color;
            GUI.color = new Color(0.2f, 0.8f, 0.2f);
            EditorGUILayout.LabelField("✓ Generation Complete", EditorStyles.boldLabel);
            if (generatedItemCount > 0)
            {
                EditorGUILayout.LabelField($"Generated {generatedItemCount} items", EditorStyles.miniLabel);
            }
            GUI.color = oldColor;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Log Area
        DrawLogArea(singleGenerationLogEntries, ref singleGenerationLogScrollPosition, "Single Generation Log");
    }

    /// <summary>
    /// Draw Batch Generation tab
    /// </summary>
    private void DrawBatchGenerationTab()
    {
        EditorGUILayout.HelpBox("Generate multiple item types in one operation. Each item type will be generated sequentially.", MessageType.Info);
        
        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        EditorGUILayout.Space(5);

        // Batch Generation Profile (shared across all batch items)
        EditorGUILayout.LabelField("Batch Generation Profile", EditorStyles.boldLabel);
        useProfile = EditorGUILayout.Toggle("Use Character Profile", useProfile);
        
        if (useProfile)
        {
            EditorGUI.indentLevel++;
            selectedProfile = (ItemGenerationProfile)EditorGUILayout.ObjectField(
                "Profile", 
                selectedProfile, 
                typeof(ItemGenerationProfile), 
                false);
            
            if (selectedProfile != null)
            {
                EditorGUILayout.HelpBox(selectedProfile.GetSummary(), MessageType.Info);
                
                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Create New Profile", GUILayout.Width(150)))
                {
                    CreateNewProfile();
                }
                if (GUILayout.Button("Edit Profile", GUILayout.Width(100)))
                {
                    Selection.activeObject = selectedProfile;
                    EditorGUIUtility.PingObject(selectedProfile);
                }
                EditorGUILayout.EndHorizontal();
            }
            else
            {
                EditorGUILayout.HelpBox("No profile selected. Click 'Create New Profile' to create one.", MessageType.Warning);
                if (GUILayout.Button("Create New Profile", GUILayout.Width(150)))
                {
                    CreateNewProfile();
                }
            }
            EditorGUI.indentLevel--;
        }
        
        EditorGUILayout.Space(5);
        
        if (batchItemsList == null)
        {
            SetupBatchItemsList();
        }

        if (batchItemsList != null)
        {
            batchItemsList.DoLayoutList();
        }

        EditorGUILayout.Space(5);
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Add Item", GUILayout.Height(25)))
        {
            var newItem = new BatchItem { itemType = ItemType.Food, count = 10, outputPath = "" };
            newItem.outputPath = GetDefaultBatchOutputPath(newItem.itemType);
            batchItems.Add(newItem);
            if (batchItemsList != null)
            {
                batchItemsList.list = batchItems;
            }
        }
        if (GUILayout.Button("Clear All", GUILayout.Height(25)))
        {
            batchItems.Clear();
            if (batchItemsList != null)
            {
                batchItemsList.list = batchItems;
            }
        }
        EditorGUILayout.EndHorizontal();

        // Validation
        bool canGenerateBatch = !isGenerating && batchItems.Count > 0 && !string.IsNullOrEmpty(executablePath) && File.Exists(executablePath);
        List<string> batchValidationErrors = new List<string>();
        
        if (canGenerateBatch)
        {
            foreach (var batchItem in batchItems)
            {
                if (batchItem.count < 1 || batchItem.count > 200)
                {
                    batchValidationErrors.Add($"{batchItem.itemType}: Count must be between 1 and 200");
                }
                if (string.IsNullOrEmpty(batchItem.outputPath))
                {
                    batchValidationErrors.Add($"{batchItem.itemType}: Output path is required");
                }
                else
                {
                    var outputValidation = InputValidator.ValidateOutputDirectory(batchItem.outputPath);
                    if (!outputValidation.IsValid)
                    {
                        batchValidationErrors.Add($"{batchItem.itemType}: {outputValidation.ErrorMessage}");
                    }
                }
            }
            
            if (batchValidationErrors.Count > 0)
            {
                EditorGUILayout.HelpBox("Validation Errors:\n" + string.Join("\n", batchValidationErrors.Select(e => "• " + e)), MessageType.Error);
            }
        }
        else if (batchItems.Count == 0)
        {
            EditorGUILayout.HelpBox("Add at least one item type to generate.", MessageType.Warning);
        }
        else if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            EditorGUILayout.HelpBox("Executable not found. Please configure it in the Settings tab.", MessageType.Warning);
        }

        EditorGUILayout.Space(5);
        GUI.enabled = canGenerateBatch && batchValidationErrors.Count == 0;
        if (GUILayout.Button(isGenerating && isBatchGeneration ? "Generating Batch..." : "Generate Batch", GUILayout.Height(35)))
        {
            GenerateBatch();
        }
        GUI.enabled = true;

        // Progress indicator
        if (isGenerating && isBatchGeneration)
        {
            EditorGUILayout.Space(5);
            Rect progressRect = EditorGUILayout.GetControlRect(false, 25);
            
            float progress = 0.5f;
            string progressText = "Generating batch...";
            
            if (batchProgress.Count > 0)
            {
                var progressParts = batchProgress.Select(kvp => $"{kvp.Key}: {kvp.Value}").ToArray();
                progressText = $"Batch: {string.Join(", ", progressParts)}";
                progress = Mathf.PingPong((float)EditorApplication.timeSinceStartup * 0.5f, 1f);
            }
            else
            {
                progress = Mathf.PingPong((float)EditorApplication.timeSinceStartup * 0.5f, 1f);
            }
            
            EditorGUI.ProgressBar(progressRect, progress, progressText);
            
            var oldColor = GUI.color;
            GUI.color = new Color(0.2f, 0.8f, 0.2f);
            EditorGUILayout.LabelField("● Processing...", EditorStyles.miniLabel);
            GUI.color = oldColor;
        }
        else if (!isGenerating && batchGenerationLogEntries.Any(e => e.type == LogType.Success))
        {
            var oldColor = GUI.color;
            GUI.color = new Color(0.2f, 0.8f, 0.2f);
            EditorGUILayout.LabelField("✓ Batch Generation Complete", EditorStyles.boldLabel);
            GUI.color = oldColor;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Log Area
        DrawLogArea(batchGenerationLogEntries, ref batchGenerationLogScrollPosition, "Batch Generation Log");
    }

    /// <summary>
    /// Draw Balance Report tab
    /// </summary>
    private void DrawBalanceReportTab()
    {
        EditorGUILayout.HelpBox("Generate a statistical balance report for an existing JSON file. This helps identify balance issues and distribution patterns.", MessageType.Info);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("JSON File:", GUILayout.Width(120));
        balanceReportJsonPath = EditorGUILayout.TextField(balanceReportJsonPath);
        if (GUILayout.Button("Browse", GUILayout.Width(60)))
        {
            string path = EditorUtility.OpenFilePanel("Select JSON File", Application.dataPath, "json");
            if (!string.IsNullOrEmpty(path))
            {
                balanceReportJsonPath = path;
            }
        }
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Item Type:", GUILayout.Width(120));
        int reportItemTypeIndex = Array.IndexOf(itemTypeValues, balanceReportItemType);
        if (reportItemTypeIndex < 0) reportItemTypeIndex = 0;
        int newReportItemTypeIndex = EditorGUILayout.Popup(reportItemTypeIndex, itemTypeNames);
        balanceReportItemType = itemTypeValues[newReportItemTypeIndex];
        EditorGUILayout.EndHorizontal();

        GUI.enabled = !string.IsNullOrEmpty(balanceReportJsonPath) && File.Exists(balanceReportJsonPath) && !isGenerating;
        if (GUILayout.Button("Generate Report", GUILayout.Height(25)))
        {
            GenerateBalanceReport();
        }
        GUI.enabled = true;

        if (!string.IsNullOrEmpty(balanceReportText))
        {
            EditorGUILayout.Space(5);
            EditorGUILayout.LabelField("Report Output:", EditorStyles.boldLabel);
            balanceReportScrollPosition = EditorGUILayout.BeginScrollView(balanceReportScrollPosition, GUILayout.Height(200));
            EditorGUILayout.TextArea(balanceReportText, GUILayout.ExpandHeight(true));
            EditorGUILayout.EndScrollView();
            
            if (GUILayout.Button("Clear Report", GUILayout.Height(25)))
            {
                balanceReportText = "";
                balanceReportScrollPosition = Vector2.zero;
            }
        }
    }

    /// <summary>
    /// Draw Preset Manager tab
    /// </summary>
    private void DrawPresetManagerTab()
    {
        EditorGUILayout.HelpBox("Create, edit, and manage custom presets for item generation.", MessageType.Info);
        
        presetManagerScrollPosition = EditorGUILayout.BeginScrollView(presetManagerScrollPosition, GUILayout.Height(200));
        
        // Preset List
        EditorGUILayout.LabelField("Custom Presets", EditorStyles.boldLabel);
        if (customPresets.Count == 0)
        {
            EditorGUILayout.HelpBox("No custom presets. Click 'Create New Preset' to add one.", MessageType.Info);
        }
        else
        {
            for (int i = 0; i < customPresets.Count; i++)
            {
                var preset = customPresets[i];
                EditorGUILayout.BeginVertical(EditorStyles.helpBox);
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField($"{preset.displayName} ({preset.id})", EditorStyles.boldLabel);
                if (GUILayout.Button("Edit", GUILayout.Width(50)))
                {
                    editingPreset = new CustomPresetData
                    {
                        id = preset.id,
                        displayName = preset.displayName,
                        description = preset.description,
                        flavorText = preset.flavorText,
                        author = preset.author,
                        version = preset.version,
                        tags = new List<string>(preset.tags)
                    };
                }
                if (GUILayout.Button("Delete", GUILayout.Width(60)))
                {
                    if (EditorUtility.DisplayDialog("Delete Preset", $"Are you sure you want to delete '{preset.displayName}'?", "Yes", "No"))
                    {
                        customPresets.RemoveAt(i);
                        i--;
                        continue;
                    }
                }
                EditorGUILayout.EndHorizontal();
                EditorGUILayout.LabelField($"Description: {preset.description}", EditorStyles.wordWrappedLabel);
                EditorGUILayout.EndVertical();
            }
        }
        
        EditorGUILayout.Space(5);
        if (GUILayout.Button("Create New Preset", GUILayout.Height(25)))
        {
            editingPreset = new CustomPresetData();
        }
        
        EditorGUILayout.EndScrollView();
        
        // Preset Editor
        if (editingPreset != null)
        {
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
            EditorGUILayout.LabelField(editingPreset.id == "" ? "New Preset" : $"Editing: {editingPreset.displayName}", EditorStyles.boldLabel);
            
            editingPreset.id = EditorGUILayout.TextField("ID (lowercase, underscore):", editingPreset.id);
            editingPreset.displayName = EditorGUILayout.TextField("Display Name:", editingPreset.displayName);
            editingPreset.description = EditorGUILayout.TextField("Description:", editingPreset.description);
            editingPreset.flavorText = EditorGUILayout.TextArea(editingPreset.flavorText, GUILayout.Height(100));
            EditorGUILayout.HelpBox("Flavor Text: World context description for LLM prompts. This text will be included at the start of generation prompts.", MessageType.Info);
            
            editingPreset.author = EditorGUILayout.TextField("Author:", editingPreset.author);
            editingPreset.version = EditorGUILayout.TextField("Version:", editingPreset.version);
            
            EditorGUILayout.LabelField("Tags:");
            EditorGUI.indentLevel++;
            for (int i = 0; i < editingPreset.tags.Count; i++)
            {
                EditorGUILayout.BeginHorizontal();
                editingPreset.tags[i] = EditorGUILayout.TextField(editingPreset.tags[i]);
                if (GUILayout.Button("Remove", GUILayout.Width(60)))
                {
                    editingPreset.tags.RemoveAt(i);
                    i--;
                }
                EditorGUILayout.EndHorizontal();
            }
            if (GUILayout.Button("Add Tag", GUILayout.Width(80)))
            {
                editingPreset.tags.Add("");
            }
            EditorGUI.indentLevel--;
            
            EditorGUILayout.Space(5);
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Save", GUILayout.Height(25)))
            {
                SaveCustomPreset(editingPreset);
            }
            if (GUILayout.Button("Save As...", GUILayout.Height(25)))
            {
                SaveCustomPresetAs(editingPreset);
            }
            if (GUILayout.Button("Cancel", GUILayout.Height(25)))
            {
                editingPreset = null;
            }
            EditorGUILayout.EndHorizontal();
        }
    }

    /// <summary>
    /// Draw log area for a specific log entry list
    /// </summary>
    private void DrawLogArea(List<LogEntry> logEntries, ref Vector2 scrollPosition, string logTitle)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.LabelField(logTitle, EditorStyles.boldLabel);
        
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Clear Log", GUILayout.Width(100), GUILayout.Height(22)))
        {
            logEntries.Clear();
            scrollPosition = Vector2.zero;
        }
        GUILayout.FlexibleSpace();
        if (logEntries.Count > 0)
        {
            EditorGUILayout.LabelField($"Entries: {logEntries.Count}", EditorStyles.miniLabel, GUILayout.Width(90));
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(3);

        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition, GUILayout.Height(200));
        
        if (logEntries.Count == 0)
        {
            EditorGUILayout.HelpBox("No log entries yet. Generation output will appear here.", MessageType.Info);
        }
        else
        {
            GUIStyle baseStyle = new GUIStyle(EditorStyles.label);
            baseStyle.fontSize = 10;
            baseStyle.wordWrap = true;
            baseStyle.richText = false;
            
            foreach (var entry in logEntries)
            {
                var oldColor = GUI.color;
                Color textColor = GetLogColor(entry.type);
                GUI.color = textColor;
                
                GUIStyle entryStyle = new GUIStyle(baseStyle);
                if (entry.type == LogType.Error)
                {
                    entryStyle.fontStyle = FontStyle.Bold;
                }
                
                EditorGUILayout.LabelField(entry.message, entryStyle);
                GUI.color = oldColor;
            }
        }
        
        EditorGUILayout.EndScrollView();
        EditorGUILayout.EndVertical();
    }


    private void GenerateItems()
    {
        if (isGenerating)
        {
            AddLog("[Error] Generation is already in progress.");
            ErrorHandler.ShowError("Generation In Progress", "Item generation is already running. Please wait for it to complete.");
            return;
        }

        // Validate inputs
        var executableValidation = InputValidator.ValidateExecutablePath(executablePath);
        if (!executableValidation.IsValid)
        {
            AddLog($"[Error] {executableValidation.ErrorMessage}");
            ErrorHandler.ShowError("Invalid Executable", executableValidation.ErrorMessage, executableValidation.Suggestion);
            return;
        }

        var countValidation = InputValidator.ValidateItemCount(itemCount);
        if (!countValidation.IsValid)
        {
            AddLog($"[Error] {countValidation.ErrorMessage}");
            ErrorHandler.ShowError("Invalid Item Count", countValidation.ErrorMessage, countValidation.Suggestion);
            return;
        }

        var outputValidation = InputValidator.ValidateOutputDirectory(outputPath);
        if (!outputValidation.IsValid)
        {
            AddLog($"[Error] {outputValidation.ErrorMessage}");
            ErrorHandler.ShowError("Invalid Output Path", outputValidation.ErrorMessage, outputValidation.Suggestion);
            return;
        }

        var modelValidation = InputValidator.ValidateModelName(GetModelName());
        if (!modelValidation.IsValid)
        {
            AddLog($"[Error] {modelValidation.ErrorMessage}");
            ErrorHandler.ShowError("Invalid Model Name", modelValidation.ErrorMessage, modelValidation.Suggestion);
            return;
        }

        if (useCustomPreset && !string.IsNullOrEmpty(customPresetPath))
        {
            var presetValidation = InputValidator.ValidatePresetPath(customPresetPath);
            if (!presetValidation.IsValid)
            {
                AddLog($"[Error] {presetValidation.ErrorMessage}");
                ErrorHandler.ShowError("Invalid Preset Path", presetValidation.ErrorMessage, presetValidation.Suggestion);
                return;
            }
        }

        string actualModelName = GetModelName();
        
        AddLog("=== Starting Item Generation ===");
        AddLog($"Model: {actualModelName}");
        if (useCustomPreset)
        {
            AddLog($"Preset: Custom ({customPresetPath})");
        }
        else
        {
            AddLog($"Preset: {selectedPreset}");
        }
        AddLog($"Item Type: {selectedItemType}");
        AddLog($"Count: {itemCount}");
        AddLog($"Output: {outputPath}");
        AddLog("");
        
        // Reset progress tracking
        generatedItemCount = 0;
        targetItemCount = itemCount;

        // Ensure ItemJson directory exists
        if (!string.IsNullOrEmpty(outputPath))
        {
            string directory = Path.GetDirectoryName(outputPath);
            if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
                AddLog($"[Info] Created directory: {directory}");
            }
        }

        isGenerating = true;
        isBatchGeneration = false;

        // Build command line arguments
        string presetName = useCustomPreset ? "" : selectedPreset.ToString().ToLower();
        string itemTypeName = selectedItemType.ToString().ToLower();
        // Handle WeaponComponent -> weaponcomponent conversion
        if (selectedItemType == ItemType.WeaponComponent)
        {
            itemTypeName = "weaponcomponent";
        }
        string args = $"--mode llm --itemType {itemTypeName} --model {actualModelName} --count {itemCount}";
        if (useTestMode)
        {
            args += " --test";
        }
        if (useCustomPreset && !string.IsNullOrEmpty(customPresetPath))
        {
            args += $" --customPreset \"{customPresetPath}\"";
        }
        else
        {
            args += $" --preset {presetName}";
        }
        args += $" --out \"{outputPath}\"";
        
        // Build combined prompt from profile and additional prompt
        string combinedPrompt = "";
        
        // Add profile context if using profile
        if (useProfile && selectedProfile != null)
        {
            combinedPrompt = selectedProfile.GenerateContextString(selectedItemType);
            if (!string.IsNullOrEmpty(additionalPrompt))
            {
                combinedPrompt += "\n\nAdditional Instructions:\n" + additionalPrompt;
            }
        }
        else if (!string.IsNullOrEmpty(additionalPrompt))
        {
            combinedPrompt = additionalPrompt;
        }
        
        // Add combined prompt if provided
        if (!string.IsNullOrEmpty(combinedPrompt))
        {
            // Escape quotes and newlines for command line
            string escapedPrompt = combinedPrompt.Replace("\"", "\\\"").Replace("\n", "\\n").Replace("\r", "");
            args += $" --additionalPrompt \"{escapedPrompt}\"";
            AddLog($"Using Profile: {(useProfile && selectedProfile != null ? selectedProfile.profileName : "None")}");
            AddLog($"Prompt Preview: {combinedPrompt.Substring(0, Math.Min(100, combinedPrompt.Length))}...");
        }

        AddLog($"Command: {executablePath} {args}");
        AddLog("");

        // Start process
        ProcessStartInfo startInfo = new ProcessStartInfo
        {
            FileName = executablePath,
            Arguments = args,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8,
            WorkingDirectory = Path.GetDirectoryName(executablePath)
        };

        try
        {
            currentProcess = new Process { StartInfo = startInfo };
            currentProcess.OutputDataReceived += OnOutputReceived;
            currentProcess.ErrorDataReceived += OnErrorReceived;
            currentProcess.Exited += OnProcessExited;
            currentProcess.EnableRaisingEvents = true;

            currentProcess.Start();
            currentProcess.BeginOutputReadLine();
            currentProcess.BeginErrorReadLine();

            AddLog("[Process] Started successfully.");
        }
        catch (Exception e)
        {
            AddLog($"[Error] Failed to start process: {e.Message}");
            isGenerating = false;
            currentProcess = null;
        }
    }

    private void DrawWeaponComponentEditingSection()
    {
        EditorGUILayout.LabelField("Weapon Component Load Plan Editor", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox("생성된 Magazine ScriptableObject를 바로 선택해서 혼합 장전 순서를 편집할 수 있습니다.", MessageType.None);

        WeaponComponentItemDataSO newAsset = (WeaponComponentItemDataSO)EditorGUILayout.ObjectField("Target Component", selectedWeaponComponentAsset, typeof(WeaponComponentItemDataSO), false);
        if (newAsset != selectedWeaponComponentAsset)
        {
            SetSelectedWeaponComponent(newAsset);
        }

        if (selectedWeaponComponentAsset == null)
        {
            EditorGUILayout.HelpBox("편집할 Weapon Component (Magazine)을 선택하세요.", MessageType.Info);
            return;
        }

        bool isMagazine = string.Equals(selectedWeaponComponentAsset.componentType, "Magazine", StringComparison.OrdinalIgnoreCase);
        if (!isMagazine)
        {
            EditorGUILayout.HelpBox("선택된 컴포넌트가 Magazine 타입이 아닙니다. Magazine만 로드 플랜을 가집니다.", MessageType.Warning);
            return;
        }

        if (weaponComponentSerializedObject == null || loadedRoundsList == null)
        {
            SetupWeaponComponentSerialization();
        }

        if (weaponComponentSerializedObject == null || loadedRoundsList == null)
        {
            EditorGUILayout.HelpBox("SerializedObject 초기화에 실패했습니다.", MessageType.Error);
            return;
        }

        weaponComponentSerializedObject.Update();

        EditorGUILayout.LabelField($"Capacity: {selectedWeaponComponentAsset.magazineCapacity}  |  Caliber: {selectedWeaponComponentAsset.caliber}");

        showLoadPlanEditor = EditorGUILayout.Foldout(showLoadPlanEditor, "Loaded Rounds", true);
        if (showLoadPlanEditor)
        {
            EditorGUILayout.HelpBox("orderIndex 0이 가장 먼저 발사됩니다. roundCount 합계가 용량을 넘지 않도록 하세요. ammoId는 Ammo ScriptableObject ID와 일치해야 합니다.", MessageType.None);
            loadedRoundsList.DoLayoutList();

            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Normalize Order"))
            {
                NormalizeSelectedLoadedRounds();
            }
            if (GUILayout.Button("Clear Plan"))
            {
                SerializedProperty prop = weaponComponentSerializedObject.FindProperty("loadedRounds");
                prop.ClearArray();
            }
            EditorGUILayout.EndHorizontal();
        }

        if (weaponComponentSerializedObject.ApplyModifiedProperties())
        {
            EditorUtility.SetDirty(selectedWeaponComponentAsset);
        }
    }

    private void SetSelectedWeaponComponent(WeaponComponentItemDataSO asset)
    {
        selectedWeaponComponentAsset = asset;
        SetupWeaponComponentSerialization();
    }

    private void SetupWeaponComponentSerialization()
    {
        if (selectedWeaponComponentAsset == null)
        {
            weaponComponentSerializedObject = null;
            loadedRoundsList = null;
            return;
        }

        weaponComponentSerializedObject = new SerializedObject(selectedWeaponComponentAsset);
        SerializedProperty loadedRoundsProp = weaponComponentSerializedObject.FindProperty("loadedRounds");
        if (loadedRoundsProp == null)
        {
            loadedRoundsList = null;
            return;
        }

        loadedRoundsList = new ReorderableList(weaponComponentSerializedObject, loadedRoundsProp, true, true, true, true);
        loadedRoundsList.drawHeaderCallback = rect =>
        {
            EditorGUI.LabelField(rect, "Load Plan (Top → Bottom)");
        };

        loadedRoundsList.elementHeightCallback = index =>
        {
            float line = EditorGUIUtility.singleLineHeight;
            return line * 3f + 12f;
        };

        loadedRoundsList.drawElementCallback = (rect, index, active, focused) =>
        {
            SerializedProperty element = loadedRoundsProp.GetArrayElementAtIndex(index);
            rect.y += 2;
            float line = EditorGUIUtility.singleLineHeight;
            float spacing = 4f;

            Rect orderRect = new Rect(rect.x, rect.y, 80f, line);
            EditorGUI.PropertyField(orderRect, element.FindPropertyRelative("orderIndex"), new GUIContent("Order"));

            Rect countRect = new Rect(orderRect.xMax + spacing, rect.y, 80f, line);
            EditorGUI.PropertyField(countRect, element.FindPropertyRelative("roundCount"), new GUIContent("Count"));

            Rect ammoRect = new Rect(countRect.xMax + spacing, rect.y, rect.width - (160f + spacing * 2f), line);
            EditorGUI.PropertyField(ammoRect, element.FindPropertyRelative("ammoId"), GUIContent.none);

            Rect displayRect = new Rect(rect.x, rect.y + line + spacing, rect.width, line);
            EditorGUI.PropertyField(displayRect, element.FindPropertyRelative("ammoDisplayName"), new GUIContent("Display"));

            Rect notesRect = new Rect(rect.x, displayRect.y + line + spacing, rect.width, line);
            EditorGUI.PropertyField(notesRect, element.FindPropertyRelative("ammoNotes"), new GUIContent("Notes"));
        };

        loadedRoundsList.onAddCallback = list =>
        {
            SerializedProperty array = loadedRoundsProp;
            int index = array.arraySize;
            array.InsertArrayElementAtIndex(index);
            SerializedProperty element = array.GetArrayElementAtIndex(index);
            element.FindPropertyRelative("orderIndex").intValue = index;
            element.FindPropertyRelative("roundCount").intValue = 1;
            element.FindPropertyRelative("ammoId").stringValue = string.Empty;
            element.FindPropertyRelative("ammoDisplayName").stringValue = string.Empty;
            element.FindPropertyRelative("ammoNotes").stringValue = string.Empty;
        };
    }

    private void NormalizeSelectedLoadedRounds()
    {
        if (weaponComponentSerializedObject == null)
            return;

        SerializedProperty loadedRoundsProp = weaponComponentSerializedObject.FindProperty("loadedRounds");
        if (loadedRoundsProp == null)
            return;

        for (int i = 0; i < loadedRoundsProp.arraySize; i++)
        {
            SerializedProperty element = loadedRoundsProp.GetArrayElementAtIndex(i);
            element.FindPropertyRelative("orderIndex").intValue = i;
        }
    }

    private void SetupBatchItemsList()
    {
        if (batchItems == null)
        {
            batchItems = new List<BatchItem>();
        }

        // Use a simple list-based ReorderableList
        batchItemsList = new ReorderableList(batchItems, typeof(BatchItem), true, true, true, true);
        batchItemsList.drawHeaderCallback = rect =>
        {
            EditorGUI.LabelField(rect, "Batch Items");
        };

        batchItemsList.elementHeightCallback = index =>
        {
            return EditorGUIUtility.singleLineHeight * 3 + 12;
        };

        batchItemsList.drawElementCallback = (rect, index, active, focused) =>
        {
            if (index < 0 || index >= batchItems.Count)
                return;

            var item = batchItems[index];
            rect.y += 2;
            float line = EditorGUIUtility.singleLineHeight;
            float spacing = 4f;

            // Item Type
            Rect typeRect = new Rect(rect.x, rect.y, rect.width * 0.35f, line);
            int typeIndex = Array.IndexOf(itemTypeValues, item.itemType);
            if (typeIndex < 0) typeIndex = 0;
            int newTypeIndex = EditorGUI.Popup(typeRect, typeIndex, itemTypeNames);
            ItemType oldType = item.itemType;
            item.itemType = itemTypeValues[newTypeIndex];
            
            // Auto-update output path if type changed and path is empty or uses old type name
            if (oldType != item.itemType && (string.IsNullOrEmpty(item.outputPath) || item.outputPath.Contains(oldType.ToString().ToLower())))
            {
                item.outputPath = GetDefaultBatchOutputPath(item.itemType);
            }

            // Count
            Rect countRect = new Rect(typeRect.xMax + spacing, rect.y, rect.width * 0.15f, line);
            EditorGUI.LabelField(new Rect(countRect.x, countRect.y, 40, line), "Count:");
            Rect countFieldRect = new Rect(countRect.x + 45, countRect.y, countRect.width - 45, line);
            item.count = EditorGUI.IntField(countFieldRect, item.count);
            item.count = Mathf.Clamp(item.count, 1, 200);

            // Output Path Label
            Rect pathLabelRect = new Rect(rect.x, rect.y + line + spacing, 80, line);
            EditorGUI.LabelField(pathLabelRect, "Output:");
            Rect pathFieldRect = new Rect(pathLabelRect.xMax + spacing, pathLabelRect.y, rect.width - pathLabelRect.width - spacing - 60, line);
            
            // Show default path hint if empty
            string displayPath = item.outputPath;
            bool isDefaultPath = string.IsNullOrEmpty(item.outputPath);
            if (isDefaultPath)
            {
                displayPath = GetDefaultBatchOutputPath(item.itemType);
                GUI.color = new Color(1f, 1f, 1f, 0.6f); // Gray out default path hint
            }
            
            string newPath = EditorGUI.TextField(pathFieldRect, displayPath);
            GUI.color = Color.white;
            
            // Update path if user actually changed it
            if (!isDefaultPath || (!string.IsNullOrEmpty(newPath) && newPath != displayPath))
            {
                item.outputPath = newPath;
            }
            
            Rect browseRect = new Rect(pathFieldRect.xMax + spacing, pathLabelRect.y, 60, line);
            if (GUI.Button(browseRect, "Browse"))
            {
                string defaultPath = GetDefaultBatchOutputPath(item.itemType);
                string directory = Path.GetDirectoryName(defaultPath);
                string fileName = Path.GetFileName(defaultPath);
                string path = EditorUtility.SaveFilePanel("Save JSON File", directory, fileName, "json");
                if (!string.IsNullOrEmpty(path))
                {
                    item.outputPath = path;
                }
            }
        };

        batchItemsList.onAddCallback = list =>
        {
            var newItem = new BatchItem { itemType = ItemType.Food, count = 10, outputPath = "" };
            // Auto-generate default output path
            newItem.outputPath = GetDefaultBatchOutputPath(newItem.itemType);
            batchItems.Add(newItem);
        };

        batchItemsList.onRemoveCallback = list =>
        {
            if (list.index >= 0 && list.index < batchItems.Count)
            {
                batchItems.RemoveAt(list.index);
            }
        };
    }

    private void GenerateBatch()
    {
        if (isGenerating)
        {
            AddLog("[Error] Generation is already in progress.");
            return;
        }

        if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            AddLog("[Error] Executable path is not valid.");
            return;
        }

        if (batchItems == null || batchItems.Count == 0)
        {
            AddLog("[Error] No batch items to generate.");
            return;
        }

        string actualModelName = GetModelName();
        
        AddLog("=== Starting Batch Generation ===");
        AddLog($"Model: {actualModelName}");
        AddLog($"Preset: {selectedPreset}");
        AddLog($"Batch Items: {batchItems.Count}");
        for (int i = 0; i < batchItems.Count; i++)
        {
            var item = batchItems[i];
            // Auto-generate path if empty
            if (string.IsNullOrEmpty(item.outputPath))
            {
                item.outputPath = GetDefaultBatchOutputPath(item.itemType);
            }
            AddLog($"  {i + 1}. {item.itemType} x{item.count} -> {item.outputPath}");
            
            // Ensure ItemJson directory exists for each output path
            if (!string.IsNullOrEmpty(item.outputPath))
            {
                string directory = Path.GetDirectoryName(item.outputPath);
                if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                    AddLog($"[Info] Created directory: {directory}");
                }
            }
        }
        AddLog("");

        isGenerating = true;
        isBatchGeneration = true;

        // Build batch string
        StringBuilder batchStr = new StringBuilder();
        for (int i = 0; i < batchItems.Count; i++)
        {
            var item = batchItems[i];
            if (i > 0) batchStr.Append(",");
            
            string itemTypeName = item.itemType.ToString().ToLower();
            if (item.itemType == ItemType.WeaponComponent)
            {
                itemTypeName = "weaponcomponent";
            }
            
            batchStr.Append($"{itemTypeName}:{item.count}");
            
            if (!string.IsNullOrEmpty(item.outputPath))
            {
                batchStr.Append($":{item.outputPath}");
            }
        }

        string presetName = useCustomPreset ? "" : selectedPreset.ToString().ToLower();
        string args = $"--mode batch --batch \"{batchStr}\" --model {actualModelName}";
        if (useTestMode)
        {
            args += " --test";
        }
        if (useCustomPreset && !string.IsNullOrEmpty(customPresetPath))
        {
            args += $" --customPreset \"{customPresetPath}\"";
        }
        else
        {
            args += $" --preset {presetName}";
        }
        
        // Build combined prompt from profile and additional prompt (applies to all items in batch)
        string combinedPrompt = "";
        
        // Add profile context if using profile
        if (useProfile && selectedProfile != null)
        {
            // For batch, we'll use a general context that applies to all types
            System.Text.StringBuilder profileContext = new System.Text.StringBuilder();
            profileContext.AppendLine($"Character Profile: {selectedProfile.profileName}");
            profileContext.AppendLine($"Description: {selectedProfile.description}");
            profileContext.AppendLine();
            profileContext.AppendLine("Character Stats:");
            profileContext.AppendLine($"- Max Hunger: {selectedProfile.maxHunger}");
            profileContext.AppendLine($"- Max Thirst: {selectedProfile.maxThirst}");
            profileContext.AppendLine($"- Max Health: {selectedProfile.maxHealth}");
            profileContext.AppendLine();
            profileContext.AppendLine("General Constraints:");
            profileContext.AppendLine($"- Max Weapon Damage: {selectedProfile.maxWeaponDamage}");
            profileContext.AppendLine($"- Max Armor Value: {selectedProfile.maxArmorValue}");
            profileContext.AppendLine($"- Max Clothing Warmth: {selectedProfile.maxClothingWarmth}");
            profileContext.AppendLine($"- Max Material Hardness: {selectedProfile.maxMaterialHardness}");
            profileContext.AppendLine($"- Max Ammo Damage: {selectedProfile.maxAmmoDamage}");
            profileContext.AppendLine($"- Max Stack Size: {selectedProfile.maxStackSize}");
            profileContext.AppendLine($"- Max Item Value: {selectedProfile.maxItemValue}");
            
            combinedPrompt = profileContext.ToString();
            if (!string.IsNullOrEmpty(additionalPrompt))
            {
                combinedPrompt += "\n\nAdditional Instructions:\n" + additionalPrompt;
            }
        }
        else if (!string.IsNullOrEmpty(additionalPrompt))
        {
            combinedPrompt = additionalPrompt;
        }
        
        // Add combined prompt if provided
        if (!string.IsNullOrEmpty(combinedPrompt))
        {
            string escapedPrompt = combinedPrompt.Replace("\"", "\\\"").Replace("\n", "\\n").Replace("\r", "");
            args += $" --additionalPrompt \"{escapedPrompt}\"";
            AddLog($"Using Profile: {(useProfile && selectedProfile != null ? selectedProfile.profileName : "None")}");
            AddLog($"Prompt Preview: {combinedPrompt.Substring(0, Math.Min(100, combinedPrompt.Length))}...");
        }

        AddLog($"Command: {executablePath} {args}");
        AddLog("");

        // Start process
        ProcessStartInfo startInfo = new ProcessStartInfo
        {
            FileName = executablePath,
            Arguments = args,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8,
            WorkingDirectory = Path.GetDirectoryName(executablePath)
        };

        try
        {
            currentProcess = new Process { StartInfo = startInfo };
            currentProcess.OutputDataReceived += OnOutputReceived;
            currentProcess.ErrorDataReceived += OnErrorReceived;
            currentProcess.Exited += OnProcessExited;
            currentProcess.EnableRaisingEvents = true;

            currentProcess.Start();
            currentProcess.BeginOutputReadLine();
            currentProcess.BeginErrorReadLine();

            AddLog("[Process] Started successfully.");
        }
        catch (Exception e)
        {
            AddLog($"[Error] Failed to start process: {e.Message}");
            isGenerating = false;
            currentProcess = null;
        }
    }

    private string FilterAnsiCodes(string text)
    {
        if (string.IsNullOrEmpty(text))
            return text;

        // Remove ANSI escape sequences (e.g., [1G, [?25h, [?2026h, etc.)
        System.Text.RegularExpressions.Regex ansiRegex = new System.Text.RegularExpressions.Regex(@"\x1B\[[0-9;]*[a-zA-Z]");
        string filtered = ansiRegex.Replace(text, "");

        // Remove common control characters that might appear
        filtered = filtered.Replace("\x1B", ""); // ESC character
        filtered = filtered.Replace("\u0007", ""); // Bell
        filtered = filtered.Replace("\u0008", ""); // Backspace
        filtered = filtered.Replace("\u000C", ""); // Form feed
        filtered = filtered.Replace("\u001B", ""); // Another ESC representation

        // Remove progress indicator characters (like boxes)
        filtered = filtered.Replace("□", "");
        filtered = filtered.Replace("■", "");
        filtered = filtered.Replace("▢", "");
        filtered = filtered.Replace("▣", "");

        return filtered.Trim();
    }

    private void OnOutputReceived(object sender, DataReceivedEventArgs e)
    {
        if (!string.IsNullOrEmpty(e.Data))
        {
            string filtered = FilterAnsiCodes(e.Data);
            if (!string.IsNullOrEmpty(filtered))
            {
                EditorApplication.delayCall += () => AddLog(filtered);
            }
        }
    }

    private void OnErrorReceived(object sender, DataReceivedEventArgs e)
    {
        if (!string.IsNullOrEmpty(e.Data))
        {
            string filtered = FilterAnsiCodes(e.Data);
            if (!string.IsNullOrEmpty(filtered))
            {
                EditorApplication.delayCall += () => AddLog($"[Error] {filtered}");
            }
        }
    }

    private void OnProcessExited(object sender, EventArgs e)
    {
        EditorApplication.delayCall += () =>
        {
            if (currentProcess != null)
            {
                int exitCode = currentProcess.ExitCode;
                AddLog("");
                AddLog($"=== Generation Complete (Exit Code: {exitCode}) ===");

                // Call completion callback if set
                if (generationCompleteCallback != null)
                {
                    bool success = exitCode == 0;
                    string message = success ? "Generation completed successfully" : $"Generation failed with exit code {exitCode}";
                    generationCompleteCallback(success, message);
                    generationCompleteCallback = null; // Clear callback after use
                }

                if (exitCode == 0)
                {
                    AddLog("[Success] Items generated successfully!", LogType.Success);

                    // Clean up raw files if enabled
                    if (autoCleanRawFiles)
                    {
                        CleanupRawFiles();
                    }

                    // Auto import if enabled
                    if (autoImportAfterGeneration)
                    {
                        if (isBatchGeneration)
                        {
                            AddLog("[Auto Import] Starting batch import...", LogType.Info);
                            ImportBatchItems();
                        }
                        else if (File.Exists(outputPath))
                        {
                            AddLog("[Auto Import] Starting import...", LogType.Info);
                            ImportGeneratedItems();
                        }
                    }
                }
                else
                {
                    AddLog($"[Error] Generation failed with exit code {exitCode}.", LogType.Error);
                }

                currentProcess.Dispose();
                currentProcess = null;
                isGenerating = false;
            }
        };
    }

    private void ImportGeneratedItems()
    {
        try
        {
            if (!File.Exists(outputPath))
            {
                AddLog($"[Auto Import Error] Output file not found: {outputPath}");
                return;
            }

            if (selectedItemType == ItemType.Food)
            {
                ItemImporter.ImportFoodFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Drink)
            {
                ItemImporter.ImportDrinkFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Medicine)
            {
                ItemImporter.ImportMedicineFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Material)
            {
                ItemImporter.ImportMaterialFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Weapon)
            {
                ItemImporter.ImportWeaponFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.WeaponComponent)
            {
                ItemImporter.ImportWeaponComponentFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Ammo)
            {
                ItemImporter.ImportAmmoFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Armor)
            {
                ItemImporter.ImportArmorFromJsonPath(outputPath);
            }
            else if (selectedItemType == ItemType.Clothing)
            {
                ItemImporter.ImportClothingFromJsonPath(outputPath);
            }

            AddLog("[Auto Import] Import completed successfully!");
        }
        catch (Exception e)
        {
            AddLog($"[Auto Import Error] {e.Message}");
        }
    }

    private void ImportBatchItems()
    {
        if (batchItems == null || batchItems.Count == 0)
        {
            AddLog("[Auto Import Error] No batch items to import.");
            return;
        }

        int successCount = 0;
        int failCount = 0;

        foreach (var batchItem in batchItems)
        {
            string filePath = batchItem.outputPath;
            if (string.IsNullOrEmpty(filePath))
            {
                // Generate default path
                string itemTypeName = batchItem.itemType.ToString().ToLower();
                if (batchItem.itemType == ItemType.WeaponComponent)
                {
                    itemTypeName = "weaponcomponent";
                }
                string outputDir = useTestMode ? "Test" : "ItemJson";
                filePath = Path.Combine(Application.dataPath.Replace("/Assets", ""), outputDir, $"items_{itemTypeName}.json");
            }

            if (!File.Exists(filePath))
            {
                AddLog($"[Auto Import] Skipping {batchItem.itemType} - file not found: {filePath}");
                failCount++;
                continue;
            }

            try
            {
                AddLog($"[Auto Import] Importing {batchItem.itemType} from {filePath}...");
                
                if (batchItem.itemType == ItemType.Food)
                {
                    ItemImporter.ImportFoodFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Drink)
                {
                    ItemImporter.ImportDrinkFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Medicine)
                {
                    ItemImporter.ImportMedicineFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Material)
                {
                    ItemImporter.ImportMaterialFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Weapon)
                {
                    ItemImporter.ImportWeaponFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.WeaponComponent)
                {
                    ItemImporter.ImportWeaponComponentFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Ammo)
                {
                    ItemImporter.ImportAmmoFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Armor)
                {
                    ItemImporter.ImportArmorFromJsonPath(filePath);
                }
                else if (batchItem.itemType == ItemType.Clothing)
                {
                    ItemImporter.ImportClothingFromJsonPath(filePath);
                }

                successCount++;
                AddLog($"[Auto Import] ✓ {batchItem.itemType} imported successfully.");
            }
            catch (Exception e)
            {
                failCount++;
                AddLog($"[Auto Import Error] Failed to import {batchItem.itemType}: {e.Message}");
            }
        }

        AddLog($"[Auto Import] Batch import completed: {successCount} succeeded, {failCount} failed.");
    }

    private void CancelGeneration()
    {
        if (currentProcess != null && !currentProcess.HasExited)
        {
            try
            {
                currentProcess.Kill();
                AddLog("[Cancelled] Generation cancelled by user.");
            }
            catch (Exception e)
            {
                AddLog($"[Error] Failed to cancel process: {e.Message}");
            }
        }
    }

    private void AddLog(string message, LogType type = LogType.Log)
    {
        // Auto-detect log type from message
        if (type == LogType.Log)
        {
            string lower = message.ToLower();
            if (lower.Contains("[error]") || lower.Contains("failed") || lower.Contains("exception"))
                type = LogType.Error;
            else if (lower.Contains("[warning]") || lower.Contains("warning:"))
                type = LogType.Warning;
            else if (lower.Contains("[success]") || lower.Contains("successfully"))
                type = LogType.Success;
            else if (lower.Contains("[info]") || lower.StartsWith("==="))
                type = LogType.Info;
        }
        
        // Add to appropriate log list based on generation mode
        List<LogEntry> targetLog = isBatchGeneration ? batchGenerationLogEntries : singleGenerationLogEntries;
        targetLog.Add(new LogEntry(message, type));
        
        // Also add to legacy logEntries for backward compatibility
        logEntries.Add(new LogEntry(message, type));
        
        // Keep log entries manageable (max 1000 entries)
        if (targetLog.Count > 1000)
        {
            targetLog.RemoveRange(0, targetLog.Count - 1000);
        }
        if (logEntries.Count > 1000)
        {
            logEntries.RemoveRange(0, logEntries.Count - 1000);
        }
        
        // Update legacy logText for compatibility
        if (string.IsNullOrEmpty(logText))
        {
            logText = message;
        }
        else
        {
            logText += "\n" + message;
        }
        
        // Parse item count from log
        ParseItemCountFromLog(message);
        
        // Auto-scroll to bottom
        if (isBatchGeneration)
        {
            batchGenerationLogScrollPosition.y = float.MaxValue;
        }
        else
        {
            singleGenerationLogScrollPosition.y = float.MaxValue;
        }
        logScrollPosition.y = float.MaxValue;

        Repaint();
    }
    
    private Color GetLogColor(LogType type)
    {
        switch (type)
        {
            case LogType.Error:
                return new Color(0.9f, 0.2f, 0.2f); // Red
            case LogType.Warning:
                return new Color(1f, 0.7f, 0.2f); // Orange
            case LogType.Success:
                return new Color(0.2f, 0.8f, 0.2f); // Green
            case LogType.Info:
                return new Color(0.4f, 0.6f, 1f); // Blue
            default:
                return EditorStyles.label.normal.textColor;
        }
    }
    
    private void ParseItemCountFromLog(string message)
    {
        // Parse patterns like "Successfully added 5 new Food items"
        var match = Regex.Match(message, @"Successfully added (\d+) new (\w+) items");
        if (match.Success)
        {
            int count = int.Parse(match.Groups[1].Value);
            string itemType = match.Groups[2].Value;
            generatedItemCount += count;
        }
        
        // Parse batch progress
        match = Regex.Match(message, @"\[Batch\] (\w+): (\d+)/(\d+)");
        if (match.Success)
        {
            string typeStr = match.Groups[1].Value;
            if (Enum.TryParse<ItemType>(typeStr, true, out ItemType itemType))
            {
                int current = int.Parse(match.Groups[2].Value);
                int total = int.Parse(match.Groups[3].Value);
                batchProgress[itemType] = current;
            }
        }
    }

    private string GetPresetDescription(PresetType preset)
    {
        switch (preset)
        {
            case PresetType.Forest:
                return "Forest environment: Natural resources, wild foods, organic materials.";
            case PresetType.Desert:
                return "Desert environment: Scarce resources, preserved foods, heat-resistant materials.";
            case PresetType.Coast:
                return "Coastal environment: Seafood, fresh water sources, marine materials.";
            case PresetType.City:
                return "Urban environment: Processed foods, manufactured items, modern weapons.";
            case PresetType.Default:
            default:
                return "Generic survival environment with moderate resources.";
        }
    }

    private void GenerateBalanceReport()
    {
        if (string.IsNullOrEmpty(balanceReportJsonPath) || !File.Exists(balanceReportJsonPath))
        {
            balanceReportText = "[Error] JSON file path is invalid or file does not exist.";
            return;
        }

        if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            balanceReportText = "[Error] Executable path is not valid.";
            return;
        }

        balanceReportText = "Generating balance report...\n";

        string itemTypeName = balanceReportItemType.ToString().ToLower();
        if (balanceReportItemType == ItemType.WeaponComponent)
        {
            itemTypeName = "weaponcomponent";
        }

        string args = $"--report \"{balanceReportJsonPath}\" --itemType {itemTypeName}";

        ProcessStartInfo startInfo = new ProcessStartInfo
        {
            FileName = executablePath,
            Arguments = args,
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
            StandardOutputEncoding = Encoding.UTF8,
            StandardErrorEncoding = Encoding.UTF8,
            WorkingDirectory = Path.GetDirectoryName(executablePath)
        };

        try
        {
            using (Process process = Process.Start(startInfo))
            {
                StringBuilder output = new StringBuilder();
                StringBuilder error = new StringBuilder();

                process.OutputDataReceived += (sender, e) =>
                {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        output.AppendLine(FilterAnsiCodes(e.Data));
                    }
                };

                process.ErrorDataReceived += (sender, e) =>
                {
                    if (!string.IsNullOrEmpty(e.Data))
                    {
                        error.AppendLine(FilterAnsiCodes(e.Data));
                    }
                };

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                process.WaitForExit();

                if (process.ExitCode == 0)
                {
                    balanceReportText = output.ToString();
                    if (!string.IsNullOrEmpty(error.ToString()))
                    {
                        balanceReportText += "\n[Warnings]\n" + error.ToString();
                    }
                }
                else
                {
                    balanceReportText = $"[Error] Report generation failed with exit code {process.ExitCode}.\n\nOutput:\n{output}\n\nErrors:\n{error}";
                }
            }
        }
        catch (Exception e)
        {
            balanceReportText = $"[Error] Failed to generate report: {e.Message}";
        }

        balanceReportScrollPosition.y = float.MaxValue;
        Repaint();
    }

    private void OnDestroy()
    {
        if (currentProcess != null && !currentProcess.HasExited)
        {
            try
            {
                currentProcess.Kill();
                currentProcess.Dispose();
            }
            catch { }
        }
    }

    private void SaveCustomPreset(CustomPresetData preset)
    {
        if (string.IsNullOrEmpty(preset.id) || string.IsNullOrEmpty(preset.displayName) || string.IsNullOrEmpty(preset.flavorText))
        {
            EditorUtility.DisplayDialog("Validation Error", "ID, Display Name, and Flavor Text are required fields.", "OK");
            return;
        }

        string defaultPath = Path.Combine(Application.dataPath, "Presets", $"{preset.id}.json");
        string path = EditorUtility.SaveFilePanel("Save Custom Preset", Path.GetDirectoryName(defaultPath), preset.id, "json");
        
        if (string.IsNullOrEmpty(path))
            return;

        SaveCustomPresetToFile(preset, path);
        
        // Update or add to list
        int index = customPresets.FindIndex(p => p.id == preset.id);
        if (index >= 0)
        {
            customPresets[index] = preset;
        }
        else
        {
            customPresets.Add(preset);
        }
        
        editingPreset = null;
        EditorUtility.DisplayDialog("Success", $"Preset '{preset.displayName}' saved successfully!", "OK");
    }

    private void SaveCustomPresetAs(CustomPresetData preset)
    {
        if (string.IsNullOrEmpty(preset.id) || string.IsNullOrEmpty(preset.displayName) || string.IsNullOrEmpty(preset.flavorText))
        {
            EditorUtility.DisplayDialog("Validation Error", "ID, Display Name, and Flavor Text are required fields.", "OK");
            return;
        }

        string defaultPath = Path.Combine(Application.dataPath, "Presets", $"{preset.id}.json");
        string path = EditorUtility.SaveFilePanel("Save Custom Preset As", Path.GetDirectoryName(defaultPath), preset.id, "json");
        
        if (string.IsNullOrEmpty(path))
            return;

        SaveCustomPresetToFile(preset, path);
        EditorUtility.DisplayDialog("Success", $"Preset saved to: {path}", "OK");
    }

    private void SaveCustomPresetToFile(CustomPresetData preset, string path)
    {
        try
        {
            // Ensure directory exists
            string directory = Path.GetDirectoryName(path);
            if (!Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }

            // Create JSON
            var json = new System.Text.StringBuilder();
            json.Append("{\n");
            json.Append($"  \"id\": \"{preset.id}\",\n");
            json.Append($"  \"displayName\": \"{preset.displayName}\",\n");
            json.Append($"  \"description\": \"{preset.description}\",\n");
            json.Append($"  \"flavorText\": \"{EscapeJsonString(preset.flavorText)}\",\n");
            if (!string.IsNullOrEmpty(preset.author))
            {
                json.Append($"  \"author\": \"{preset.author}\",\n");
            }
            if (!string.IsNullOrEmpty(preset.version))
            {
                json.Append($"  \"version\": \"{preset.version}\",\n");
            }
            if (preset.tags != null && preset.tags.Count > 0)
            {
                json.Append("  \"tags\": [\n");
                for (int i = 0; i < preset.tags.Count; i++)
                {
                    json.Append($"    \"{preset.tags[i]}\"");
                    if (i < preset.tags.Count - 1)
                        json.Append(",");
                    json.Append("\n");
                }
                json.Append("  ]\n");
            }
            else
            {
                json.Append("  \"tags\": []\n");
            }
            json.Append("}");

            File.WriteAllText(path, json.ToString());
        }
        catch (Exception e)
        {
            EditorUtility.DisplayDialog("Error", $"Failed to save preset: {e.Message}", "OK");
        }
    }

    private string EscapeJsonString(string str)
    {
        if (string.IsNullOrEmpty(str))
            return "";
        
        return str.Replace("\\", "\\\\")
                  .Replace("\"", "\\\"")
                  .Replace("\n", "\\n")
                  .Replace("\r", "\\r")
                  .Replace("\t", "\\t");
    }

    private void CheckOllamaStatus()
    {
        ollamaInstalled = false;
        ollamaVersion = "";
        ollamaCheckError = "";

        try
        {
            string programFilesPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "Ollama", "ollama.exe");
            string localAppPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Ollama", "ollama.exe");

            if (File.Exists(programFilesPath) || File.Exists(localAppPath))
            {
                ollamaInstalled = true;
            }

            ProcessStartInfo psi = new ProcessStartInfo
            {
                FileName = "ollama",
                Arguments = "--version",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
                StandardOutputEncoding = Encoding.UTF8,
                StandardErrorEncoding = Encoding.UTF8
            };

            using (Process proc = Process.Start(psi))
            {
                if (proc != null)
                {
                    bool exited = proc.WaitForExit(4000);
                    if (!exited)
                    {
                        try { proc.Kill(); } catch { }
                        ollamaCheckError = "ollama --version timed out (4s).";
                        return;
                    }

                    string output = proc.StandardOutput.ReadToEnd().Trim();
                    string err = proc.StandardError.ReadToEnd().Trim();

                    if (proc.ExitCode == 0)
                    {
                        ollamaInstalled = true;
                        if (!string.IsNullOrEmpty(output))
                        {
                            ollamaVersion = output;
                        }
                    }
                    else if (!string.IsNullOrEmpty(err))
                    {
                        ollamaCheckError = err;
                    }
                }
            }
        }
        catch (Exception ex)
        {
            ollamaCheckError = ex.Message;
        }

        if (ollamaInstalled && !string.IsNullOrEmpty(ollamaVersion))
            ollamaStatus = $"Ollama detected ({ollamaVersion})";
        else
            ollamaStatus = ollamaInstalled ? "Ollama detected" : "Ollama not detected";
    }

    private void RunOllamaInstaller()
    {
        if (IsOllamaInstalledInDefaultPaths(out var detectedPath))
        {
            AddLog($"[Setup] Ollama already installed at {detectedPath}. Skipping installer.");
            // Ollama already installed
            return;
        }

        if (string.IsNullOrEmpty(ollamaSetupScriptPath) || !File.Exists(ollamaSetupScriptPath))
        {
            AddLog("[Setup] Installer .bat not found. Please set the correct path.");
            return;
        }

        try
        {
            AddLog("[Setup] Running Ollama installer batch...");

            ProcessStartInfo psi = new ProcessStartInfo
            {
                FileName = "cmd.exe",
                Arguments = $"/c \"\"{ollamaSetupScriptPath}\"\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
                WorkingDirectory = Path.GetDirectoryName(ollamaSetupScriptPath),
                StandardOutputEncoding = Encoding.UTF8,
                StandardErrorEncoding = Encoding.UTF8
            };

            using (Process proc = Process.Start(psi))
            {
                if (proc != null)
                {
                    string output = proc.StandardOutput.ReadToEnd();
                    string err = proc.StandardError.ReadToEnd();
                    proc.WaitForExit();

                    AddLog($"[Setup] Installer exit code: {proc.ExitCode}");
                    if (!string.IsNullOrWhiteSpace(output))
                    {
                        AddLog("[Setup] " + output.Trim());
                    }
                    if (!string.IsNullOrWhiteSpace(err))
                    {
                        AddLog("[Setup][stderr] " + err.Trim());
                    }
                }
            }

            CheckOllamaStatus();
            AddLog("[Setup] Ollama check: " + ollamaStatus);
        }
        catch (Exception ex)
        {
            AddLog("[Setup] Failed to run installer: " + ex.Message);
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

    private void CleanupRawFiles()
    {
        try
        {
            if (isBatchGeneration)
            {
                foreach (var batchItem in batchItems)
                {
                    string outputDir = useTestMode ? "Test" : "ItemJson";
                    string filePath = string.IsNullOrEmpty(batchItem.outputPath) 
                        ? Path.Combine(Application.dataPath.Replace("/Assets", ""), outputDir, $"items_{batchItem.itemType.ToString().ToLower()}.json")
                        : batchItem.outputPath;
                    string rawPath = filePath + ".raw.json";
                    if (File.Exists(rawPath))
                    {
                        File.Delete(rawPath);
                        AddLog($"[Cleanup] Removed {rawPath}", LogType.Info);
                    }
                }
            }
            else if (!string.IsNullOrEmpty(outputPath))
            {
                string rawPath = outputPath + ".raw.json";
                if (File.Exists(rawPath))
                {
                    File.Delete(rawPath);
                    AddLog($"[Cleanup] Removed {rawPath}", LogType.Info);
                }
            }
        }
        catch (Exception e)
        {
            AddLog($"[Warning] Failed to clean raw files: {e.Message}", LogType.Warning);
        }
    }
    
    private void LoadSettings()
    {
        // Try to load from ScriptableObject settings first
        var settings = ItemFactorySettings.Instance;
        
        // Load from ScriptableObject if available, otherwise fall back to EditorPrefs
        autoCleanRawFiles = settings != null ? settings.autoCleanRawFiles : EditorPrefs.GetBool(PREFS_KEY_PREFIX + "autoCleanRawFiles", true);
        autoImportAfterGeneration = settings != null ? settings.autoImportAfterGeneration : EditorPrefs.GetBool(PREFS_KEY_PREFIX + "autoImportAfterGeneration", true);
        selectedModel = (ModelType)EditorPrefs.GetInt(PREFS_KEY_PREFIX + "selectedModel", (int)ModelType.Llama3);
        selectedPreset = settings != null ? settings.defaultPreset : (PresetType)EditorPrefs.GetInt(PREFS_KEY_PREFIX + "selectedPreset", (int)PresetType.Default);
        selectedItemType = (ItemType)EditorPrefs.GetInt(PREFS_KEY_PREFIX + "selectedItemType", (int)ItemType.Food);
        itemCount = settings != null ? settings.defaultItemCount : EditorPrefs.GetInt(PREFS_KEY_PREFIX + "itemCount", 10);
        maxHunger = settings != null ? settings.defaultMaxHunger : EditorPrefs.GetInt(PREFS_KEY_PREFIX + "maxHunger", 100);
        maxThirst = settings != null ? settings.defaultMaxThirst : EditorPrefs.GetInt(PREFS_KEY_PREFIX + "maxThirst", 100);
        customModelName = settings != null && !string.IsNullOrEmpty(settings.defaultModel) ? settings.defaultModel : EditorPrefs.GetString(PREFS_KEY_PREFIX + "customModelName", "");
        useCustomPreset = EditorPrefs.GetBool(PREFS_KEY_PREFIX + "useCustomPreset", false);
        customPresetPath = EditorPrefs.GetString(PREFS_KEY_PREFIX + "customPresetPath", "");
        useTestMode = settings != null ? settings.useTestMode : EditorPrefs.GetBool(PREFS_KEY_PREFIX + "useTestMode", false);
        
        // Load executable path from settings if available
        if (settings != null && !string.IsNullOrEmpty(settings.GetFullExecutablePath()) && File.Exists(settings.GetFullExecutablePath()))
        {
            executablePath = settings.GetFullExecutablePath();
        }
        
        // Update output path after loading test mode setting
        UpdateOutputPathForItemType();
    }
    
    private void SaveSettings()
    {
        // Save to ScriptableObject settings
        var settings = ItemFactorySettings.Instance;
        if (settings != null)
        {
            settings.autoCleanRawFiles = autoCleanRawFiles;
            settings.autoImportAfterGeneration = autoImportAfterGeneration;
            settings.defaultPreset = selectedPreset;
            settings.defaultItemCount = itemCount;
            settings.defaultMaxHunger = maxHunger;
            settings.defaultMaxThirst = maxThirst;
            if (selectedModel == ModelType.Custom && !string.IsNullOrEmpty(customModelName))
            {
                settings.defaultModel = customModelName;
            }
            settings.useTestMode = useTestMode;
            settings.Save();
        }
        
        // Also save to EditorPrefs for backward compatibility and user-specific preferences
        EditorPrefs.SetBool(PREFS_KEY_PREFIX + "autoCleanRawFiles", autoCleanRawFiles);
        EditorPrefs.SetBool(PREFS_KEY_PREFIX + "autoImportAfterGeneration", autoImportAfterGeneration);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "selectedModel", (int)selectedModel);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "selectedPreset", (int)selectedPreset);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "selectedItemType", (int)selectedItemType);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "itemCount", itemCount);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "maxHunger", maxHunger);
        EditorPrefs.SetInt(PREFS_KEY_PREFIX + "maxThirst", maxThirst);
        EditorPrefs.SetString(PREFS_KEY_PREFIX + "customModelName", customModelName);
        EditorPrefs.SetBool(PREFS_KEY_PREFIX + "useCustomPreset", useCustomPreset);
        EditorPrefs.SetString(PREFS_KEY_PREFIX + "customPresetPath", customPresetPath);
        EditorPrefs.SetBool(PREFS_KEY_PREFIX + "useTestMode", useTestMode);
    }
    
    private void HandleKeyboardShortcuts()
    {
        Event e = Event.current;
        if (e.type == EventType.KeyDown)
        {
            // Ctrl+G: Generate items
            if (e.control && e.keyCode == KeyCode.G)
            {
                if (!isGenerating && !string.IsNullOrEmpty(executablePath) && File.Exists(executablePath))
                {
                    GenerateItems();
                    e.Use();
                }
            }
            // Ctrl+R: Refresh
            else if (e.control && e.keyCode == KeyCode.R)
            {
                RefreshWindow();
                e.Use();
            }
            // Ctrl+L: Clear log
            else if (e.control && e.keyCode == KeyCode.L)
            {
                logText = "";
                logEntries.Clear();
                generatedItemCount = 0;
                targetItemCount = 0;
                batchProgress.Clear();
                logScrollPosition = Vector2.zero;
                e.Use();
            }
            // Escape: Cancel generation
            else if (e.keyCode == KeyCode.Escape && isGenerating)
            {
                CancelGeneration();
                e.Use();
            }
        }
    }
    
    private void RefreshWindow()
    {
        InitializeExecutablePath();
        CheckOllamaStatus();
        Repaint();
    }
    
    private string[] GetRecentOutputPaths()
    {
        string recentPathsKey = PREFS_KEY_PREFIX + "recentOutputPaths";
        string pathsStr = EditorPrefs.GetString(recentPathsKey, "");
        if (string.IsNullOrEmpty(pathsStr))
            return new string[0];
        
        return pathsStr.Split('|').Where(p => !string.IsNullOrEmpty(p) && File.Exists(p) || Directory.Exists(Path.GetDirectoryName(p)))
            .Take(10).ToArray();
    }
    
    private string[] GetRecentPresetPaths()
    {
        string recentPresetsKey = PREFS_KEY_PREFIX + "recentPresetPaths";
        string pathsStr = EditorPrefs.GetString(recentPresetsKey, "");
        if (string.IsNullOrEmpty(pathsStr))
            return new string[0];
        
        return pathsStr.Split('|').Where(p => !string.IsNullOrEmpty(p) && File.Exists(p))
            .Take(10).ToArray();
    }
    
    private void SaveRecentPresetPath(string path)
    {
        if (string.IsNullOrEmpty(path))
            return;
        
        string recentPresetsKey = PREFS_KEY_PREFIX + "recentPresetPaths";
        string pathsStr = EditorPrefs.GetString(recentPresetsKey, "");
        var paths = new List<string> { path };
        
        if (!string.IsNullOrEmpty(pathsStr))
        {
            paths.AddRange(pathsStr.Split('|').Where(p => p != path && !string.IsNullOrEmpty(p)));
        }
        
        EditorPrefs.SetString(recentPresetsKey, string.Join("|", paths.Take(10)));
    }
    
    private void SaveRecentOutputPath(string path)
    {
        if (string.IsNullOrEmpty(path))
            return;
        
        string recentPathsKey = PREFS_KEY_PREFIX + "recentOutputPaths";
        string pathsStr = EditorPrefs.GetString(recentPathsKey, "");
        var paths = new List<string> { path };
        
        if (!string.IsNullOrEmpty(pathsStr))
        {
            paths.AddRange(pathsStr.Split('|').Where(p => p != path && !string.IsNullOrEmpty(p)));
        }
        
        EditorPrefs.SetString(recentPathsKey, string.Join("|", paths.Take(10)));
    }
    
    private void CreateNewProfile()
    {
        string path = EditorUtility.SaveFilePanelInProject(
            "Create Generation Profile",
            "ItemGenerationProfile",
            "asset",
            "Create a new item generation profile");
        
        if (!string.IsNullOrEmpty(path))
        {
            ItemGenerationProfile profile = ScriptableObject.CreateInstance<ItemGenerationProfile>();
            profile.profileName = Path.GetFileNameWithoutExtension(path);
            AssetDatabase.CreateAsset(profile, path);
            AssetDatabase.SaveAssets();
            AssetDatabase.Refresh();
            selectedProfile = profile;
            useProfile = true;
            Selection.activeObject = profile;
            EditorGUIUtility.PingObject(profile);
        }
    }

    private void CheckOllamaInDefaultPaths()
    {
        try
        {
            // Checking default paths
            string programFilesPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "Ollama", "ollama.exe");
            string localAppPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Ollama", "ollama.exe");
            bool existsProgram = File.Exists(programFilesPath);
            bool existsLocal = File.Exists(localAppPath);
            if (existsProgram)
            {
                ollamaExistsInDefaultPath = true;
                ollamaDetectedPath = programFilesPath;
                ollamaDetectMessage = $"Ollama detected at {programFilesPath}";
                // Ollama found in Program Files
            }
            else if (existsLocal)
            {
                ollamaExistsInDefaultPath = true;
                ollamaDetectedPath = localAppPath;
                ollamaDetectMessage = $"Ollama detected at {localAppPath}";
                // Ollama found in Local AppData
            }
            else
            {
                ollamaExistsInDefaultPath = false;
                ollamaDetectedPath = "";
                ollamaDetectMessage = "Ollama not found in default locations.";
                // Ollama not found
            }
        }
        catch (Exception ex)
        {
            ollamaExistsInDefaultPath = false;
            ollamaDetectedPath = "";
            ollamaDetectMessage = "Check failed: " + ex.Message;
            // Check failed
        }
    }

}
