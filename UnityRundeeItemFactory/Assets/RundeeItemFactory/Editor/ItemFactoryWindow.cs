using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
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

public class ItemFactoryWindow : EditorWindow
{
    // #region agent log helper
    private static void AgentLog(string hypothesisId, string location, string message)
    {
        var payload = $"{{\"sessionId\":\"debug-session\",\"runId\":\"run4\",\"hypothesisId\":\"{hypothesisId}\",\"location\":\"{location}\",\"message\":\"{message}\",\"timestamp\":{DateTimeOffset.UtcNow.ToUnixTimeMilliseconds()}}}\n";
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
            // ignore logging errors
        }
    }
    // #endregion

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

    // UI State
    private Vector2 logScrollPosition;
    private string logText = "";
    private bool isGenerating = false;
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

    // UI Foldouts
    private bool showConfigSection = true;
    private bool showGenerationParams = true;
    private bool showBatchGeneration = false;
    private bool showPresetManager = false;
    private bool showAdvancedOptions = false;
    private bool showBalanceReport = false;

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
    private readonly string[] itemTypeNames = { "Food", "Drink", "Material", "Weapon", "Weapon Component", "Ammo" };
    private readonly ItemType[] itemTypeValues = { ItemType.Food, ItemType.Drink, ItemType.Material, ItemType.Weapon, ItemType.WeaponComponent, ItemType.Ammo };

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

    [MenuItem("Tools/Rundee/Item Factory/Item Factory Window")]
    public static void ShowWindow()
    {
        ItemFactoryWindow window = GetWindow<ItemFactoryWindow>("Item Factory");
        window.minSize = new Vector2(500, 600);
        window.InitializeExecutablePath();
        window.InitializeOutputPath();
        window.InitializeSetupPaths();
        window.CheckOllamaStatus();
    }

    private void OnEnable()
    {
        InitializeExecutablePath();
        InitializeOutputPath();
        InitializeSetupPaths();
        CheckOllamaStatus();
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
        // Default to ItemJson folder in project root with a descriptive filename
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        string defaultFileName = $"items_{selectedItemType.ToString().ToLower()}.json";
        outputPath = Path.Combine(projectRoot, "ItemJson", defaultFileName);
    }

    private void InitializeExecutablePath()
    {
        // Try to find the executable relative to Unity project
        AgentLog("H10", "ItemFactoryWindow.InitializeExecutablePath:start", "auto-scan");
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
                AgentLog("H10", "ItemFactoryWindow.InitializeExecutablePath:found", path);
                break;
            }
        }

        if (string.IsNullOrEmpty(executablePath))
        {
            AddLog("[Warning] Executable not found in standard locations.");
            AgentLog("H10", "ItemFactoryWindow.InitializeExecutablePath:not_found", "none");
        }
    }

    private void OnGUI()
    {
        EditorGUILayout.Space(10);

        // Title
        GUIStyle titleStyle = new GUIStyle(GUI.skin.label);
        titleStyle.fontSize = 18;
        titleStyle.fontStyle = FontStyle.Bold;
        EditorGUILayout.LabelField("Rundee Item Factory", titleStyle);
        EditorGUILayout.Space(5);

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
                customPresetPath = EditorGUILayout.TextField(customPresetPath);
                if (GUILayout.Button("Browse", GUILayout.Width(60)))
                {
                    string path = EditorUtility.OpenFilePanel("Select Custom Preset", Application.dataPath, "json");
                    if (!string.IsNullOrEmpty(path))
                    {
                        customPresetPath = path;
                    }
                }
                EditorGUILayout.EndHorizontal();
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

            // Count
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Count:", GUILayout.Width(120));
            itemCount = EditorGUILayout.IntField(itemCount);
            itemCount = Mathf.Clamp(itemCount, 1, 200);
            EditorGUILayout.EndHorizontal();
            EditorGUILayout.HelpBox($"Will generate {itemCount} items. Recommended: 10-20 for testing, 50-100 for production.", MessageType.None);

            // Max Hunger (only for Food/Drink)
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

            // Output Path
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Output File:", GUILayout.Width(120));
            outputPath = EditorGUILayout.TextField(outputPath);
            if (GUILayout.Button("Browse", GUILayout.Width(60)))
            {
                string defaultName = $"items_{selectedItemType.ToString().ToLower()}";
                string path = EditorUtility.SaveFilePanel("Save JSON File", Application.dataPath, defaultName, "json");
                if (!string.IsNullOrEmpty(path))
                {
                    outputPath = path;
                }
            }
            EditorGUILayout.EndHorizontal();

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

            EditorGUI.indentLevel--;
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Preset Manager Section
        showPresetManager = EditorGUILayout.Foldout(showPresetManager, "Preset Manager", false);
        if (showPresetManager)
        {
            EditorGUI.indentLevel++;
            EditorGUILayout.HelpBox("Create, edit, and manage custom presets for item generation.", MessageType.Info);
            
            presetManagerScrollPosition = EditorGUILayout.BeginScrollView(presetManagerScrollPosition);
            
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
            
            EditorGUI.indentLevel--;
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        }

        // Batch Generation Section
        showBatchGeneration = EditorGUILayout.Foldout(showBatchGeneration, "Batch Generation", false);
        if (showBatchGeneration)
        {
            EditorGUI.indentLevel++;
            EditorGUILayout.HelpBox("Generate multiple item types in one operation. Each item type will be generated sequentially.", MessageType.Info);
            
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
                batchItems.Add(new BatchItem { itemType = ItemType.Food, count = 10, outputPath = "" });
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

            EditorGUILayout.Space(5);
            GUI.enabled = !isGenerating && batchItems.Count > 0 && !string.IsNullOrEmpty(executablePath) && File.Exists(executablePath);
            if (GUILayout.Button("Generate Batch", GUILayout.Height(30)))
            {
                GenerateBatch();
            }
            GUI.enabled = true;

            EditorGUI.indentLevel--;
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        }

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

        // Balance Report Section
        showBalanceReport = EditorGUILayout.Foldout(showBalanceReport, "Balance Report", false);
        if (showBalanceReport)
        {
            EditorGUI.indentLevel++;
            EditorGUILayout.HelpBox("Generate a statistical balance report for an existing JSON file. This helps identify balance issues and distribution patterns.", MessageType.Info);
            
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

            EditorGUI.indentLevel--;
            EditorGUILayout.Space(10);
            EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);
        }

        // Generate Button
        EditorGUILayout.BeginHorizontal();
        GUI.enabled = !isGenerating && !string.IsNullOrEmpty(executablePath) && File.Exists(executablePath);
        if (GUILayout.Button(isGenerating ? "Generating..." : "Generate Items", GUILayout.Height(35)))
        {
            GenerateItems();
        }
        GUI.enabled = true;

        // Cancel Button
        if (isGenerating && currentProcess != null)
        {
            if (GUILayout.Button("Cancel", GUILayout.Height(35)))
            {
                CancelGeneration();
            }
        }
        EditorGUILayout.EndHorizontal();

        // Progress indicator
        if (isGenerating)
        {
            EditorGUILayout.Space(5);
            Rect progressRect = EditorGUILayout.GetControlRect(false, 20);
            EditorGUI.ProgressBar(progressRect, 0.5f, "Generating items... (This may take a while)");
        }

        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("", GUI.skin.horizontalSlider);

        // Log Area
        EditorGUILayout.LabelField("Generation Log", EditorStyles.boldLabel);
        EditorGUILayout.Space(5);

        logScrollPosition = EditorGUILayout.BeginScrollView(logScrollPosition, GUILayout.Height(200));
        EditorGUILayout.TextArea(logText, GUILayout.ExpandHeight(true));
        EditorGUILayout.EndScrollView();

        // Clear Log Button
        if (GUILayout.Button("Clear Log", GUILayout.Height(25)))
        {
            logText = "";
            logScrollPosition = Vector2.zero;
        }
    }

    private void GenerateItems()
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
        if (useCustomPreset && !string.IsNullOrEmpty(customPresetPath))
        {
            args += $" --customPreset \"{customPresetPath}\"";
        }
        else
        {
            args += $" --preset {presetName}";
        }
        args += $" --out \"{outputPath}\"";

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
            item.itemType = itemTypeValues[newTypeIndex];

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
            item.outputPath = EditorGUI.TextField(pathFieldRect, item.outputPath);
            
            Rect browseRect = new Rect(pathFieldRect.xMax + spacing, pathLabelRect.y, 60, line);
            if (GUI.Button(browseRect, "Browse"))
            {
                string defaultName = $"items_{item.itemType.ToString().ToLower()}";
                string path = EditorUtility.SaveFilePanel("Save JSON File", Application.dataPath, defaultName, "json");
                if (!string.IsNullOrEmpty(path))
                {
                    item.outputPath = path;
                }
            }
        };

        batchItemsList.onAddCallback = list =>
        {
            batchItems.Add(new BatchItem { itemType = ItemType.Food, count = 10, outputPath = "" });
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
            string output = string.IsNullOrEmpty(item.outputPath) ? $"ItemJson/items_{item.itemType.ToString().ToLower()}.json" : item.outputPath;
            AddLog($"  {i + 1}. {item.itemType} x{item.count} -> {output}");
            
            // Ensure ItemJson directory exists for each output path
            if (!string.IsNullOrEmpty(output))
            {
                string directory = Path.GetDirectoryName(output);
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
        if (useCustomPreset && !string.IsNullOrEmpty(customPresetPath))
        {
            args += $" --customPreset \"{customPresetPath}\"";
        }
        else
        {
            args += $" --preset {presetName}";
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

                if (exitCode == 0)
                {
                    AddLog("[Success] Items generated successfully!");

                    // Auto import if enabled
                    if (autoImportAfterGeneration)
                    {
                        if (isBatchGeneration)
                        {
                            AddLog("[Auto Import] Starting batch import...");
                            ImportBatchItems();
                        }
                        else if (File.Exists(outputPath))
                        {
                            AddLog("[Auto Import] Starting import...");
                            ImportGeneratedItems();
                        }
                    }
                }
                else
                {
                    AddLog($"[Error] Generation failed with exit code {exitCode}.");
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
                filePath = Path.Combine(Application.dataPath.Replace("/Assets", ""), "ItemJson", $"items_{itemTypeName}.json");
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

    private void AddLog(string message)
    {
        if (string.IsNullOrEmpty(logText))
        {
            logText = message;
        }
        else
        {
            logText += "\n" + message;
        }

        // Auto-scroll to bottom
        logScrollPosition.y = float.MaxValue;

        Repaint();
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
            AgentLog("H9", "ItemFactoryWindow.RunOllamaInstaller:skip_installed", detectedPath ?? "unknown");
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

    private void CheckOllamaInDefaultPaths()
    {
        try
        {
            AgentLog("H7", "ItemFactoryWindow.CheckOllamaInDefaultPaths:start", "checking default paths");
            string programFilesPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "Ollama", "ollama.exe");
            string localAppPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Programs", "Ollama", "ollama.exe");
            bool existsProgram = File.Exists(programFilesPath);
            bool existsLocal = File.Exists(localAppPath);
            if (existsProgram)
            {
                ollamaExistsInDefaultPath = true;
                ollamaDetectedPath = programFilesPath;
                ollamaDetectMessage = $"Ollama detected at {programFilesPath}";
                AgentLog("H7", "ItemFactoryWindow.CheckOllamaInDefaultPaths:found", programFilesPath);
            }
            else if (existsLocal)
            {
                ollamaExistsInDefaultPath = true;
                ollamaDetectedPath = localAppPath;
                ollamaDetectMessage = $"Ollama detected at {localAppPath}";
                AgentLog("H7", "ItemFactoryWindow.CheckOllamaInDefaultPaths:found", localAppPath);
            }
            else
            {
                ollamaExistsInDefaultPath = false;
                ollamaDetectedPath = "";
                ollamaDetectMessage = "Ollama not found in default locations.";
                AgentLog("H7", "ItemFactoryWindow.CheckOllamaInDefaultPaths:not_found", "none");
            }
        }
        catch (Exception ex)
        {
            ollamaExistsInDefaultPath = false;
            ollamaDetectedPath = "";
            ollamaDetectMessage = "Check failed: " + ex.Message;
            AgentLog("H7", "ItemFactoryWindow.CheckOllamaInDefaultPaths:exception", ex.Message);
        }
    }

}
