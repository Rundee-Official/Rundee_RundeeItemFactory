using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Main unified Item Factory window for profile-based item generation
/// </summary>
public class ItemFactoryMainWindow : EditorWindow
{
    private string profilesPath;
    private List<ItemProfile> availableProfiles = new List<ItemProfile>();
    private ItemProfile selectedProfile = null;
    private int selectedProfileIndex = 0;
    private string[] profileNames = new string[0];
    
    private string executablePath = "";
    
    // Ollama/LLM Generation Parameters
    private string modelName = "llama3";
    private float temperature = 0.7f;
    private float topP = 0.9f;
    private int maxTokens = 4096;
    private int itemCount = 10;
    private string presetName = ""; // User-defined preset name (e.g., "Forest", "Desert", "My Custom World")
    
    // Player Settings
    private int maxHunger = 100;
    private int maxThirst = 100;
    private int maxHealth = 100;
    private int maxStamina = 100;
    private int maxWeight = 50000; // grams (50kg default)
    private int maxEnergy = 100;
    
    // Output Settings
    private string outputPath = "";
    private bool autoImport = true;
    
    private Vector2 scrollPosition;
    private bool isGenerating = false;
    private Process currentProcess = null;
    private string logText = "";
    private Vector2 logScrollPosition;
    
    // Download state
    private bool isDownloading = false;
    private float downloadProgress = 0f;
    private string downloadStatus = "";
    
    // Tab system
    private int selectedTab = 0;
    private string[] tabNames = new string[] { "Item Factory", "Player Profile Manager", "Item Profile Manager" };
    
    // Item Profile Manager state
    private Vector2 itemProfileListScrollPosition;
    private Vector2 itemProfileEditorScrollPosition;
    private List<ItemProfile> itemProfileManagerProfiles = new List<ItemProfile>();
    private ItemProfile itemProfileManagerSelectedProfile = null;
    private bool showItemProfileEditor = false;
    private string itemProfileSearchFilter = "";
    private string itemProfileSelectedItemType = "All";
    private string[] itemProfileAvailableItemTypes = new string[] { "All" };
    
    // Player Profile Manager state
    private Vector2 playerProfileListScrollPosition;
    private Vector2 playerProfileEditorScrollPosition;
    private List<PlayerProfile> playerProfileManagerProfiles = new List<PlayerProfile>();
    private PlayerProfile playerProfileManagerSelectedProfile = null;
    private bool showPlayerProfileEditor = false;
    private string playerProfileSearchFilter = "";
    private PlayerProfile selectedPlayerProfile = null; // Selected in Item Factory tab
    
    [MenuItem("Tools/Rundee/Item Factory/Item Factory", false, 1000)]
    public static void ShowWindow()
    {
        var window = GetWindow<ItemFactoryMainWindow>("Item Factory");
        window.minSize = new Vector2(1000, 700);
        // Set initial window size
        window.position = new Rect(window.position.x, window.position.y, 1200, 800);
        window.Initialize();
    }
    
    private void Initialize()
    {
        LoadProfiles();
        LoadItemProfileManagerProfiles();
        LoadPlayerProfileManagerProfiles();
        LoadSettings();
        
        // Load selected player profile
        string savedPlayerProfileId = EditorPrefs.GetString("ItemFactory_SelectedPlayerProfileId", "");
        if (!string.IsNullOrEmpty(savedPlayerProfileId))
        {
            selectedPlayerProfile = playerProfileManagerProfiles.FirstOrDefault(p => p.id == savedPlayerProfileId);
            if (selectedPlayerProfile != null)
            {
                LoadPlayerSettingsFromPlayerProfile();
            }
        }
        
        UpdateOutputPath();
    }
    
    private void OnEnable()
    {
        Initialize();
    }
    
    private void OnGUI()
    {
        // Tab selection
        selectedTab = GUILayout.Toolbar(selectedTab, tabNames);
        
        EditorGUILayout.Space(5);
        
        if (selectedTab == 0)
        {
            DrawItemFactoryTab();
        }
        else if (selectedTab == 1)
        {
            DrawPlayerProfileManagerTab();
        }
        else if (selectedTab == 2)
        {
            DrawItemProfileManagerTab();
        }
    }
    
    private void DrawItemFactoryTab()
    {
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition, GUILayout.ExpandHeight(true));
        
        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("Item Factory", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Generate game items using LLM with custom profiles. Select a profile and configure generation parameters.",
            MessageType.Info);
        
        EditorGUILayout.Space(10);
        
        // Executable Path (Auto-detected or Download)
        EditorGUILayout.LabelField("Configuration", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Executable Path:", EditorStyles.miniLabel);
        if (GUILayout.Button("Auto-Detect", GUILayout.Width(100)))
        {
            FindExecutablePath();
        }
        EditorGUILayout.EndHorizontal();
        
        // Check if executable exists at default download location
        string defaultDownloadPath = ExecutableDownloader.GetDownloadPath();
        bool existsAtDefault = File.Exists(defaultDownloadPath);
        
        if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            if (!existsAtDefault && !isDownloading)
            {
                EditorGUILayout.HelpBox(
                    "Executable not found. You can download it automatically or browse manually.", 
                    MessageType.Warning);
                
                EditorGUILayout.BeginHorizontal();
                if (GUILayout.Button("Download Executable", GUILayout.Height(30)))
                {
                    StartDownload();
                }
                if (GUILayout.Button("Browse", GUILayout.Width(70), GUILayout.Height(30)))
                {
                    string defaultDir = string.IsNullOrEmpty(executablePath) 
                        ? Application.dataPath 
                        : Path.GetDirectoryName(executablePath);
                    string path = EditorUtility.OpenFilePanel("Select RundeeItemFactory.exe", defaultDir, "exe");
                    if (!string.IsNullOrEmpty(path))
                    {
                        executablePath = path;
                        SaveSettings();
                    }
                }
                EditorGUILayout.EndHorizontal();
            }
            else if (existsAtDefault)
            {
                // Default location???�으�??�동?�로 ?�정
                executablePath = defaultDownloadPath;
                SaveSettings();
            }
            
            // Download progress
            if (isDownloading)
            {
                EditorGUILayout.Space(5);
                EditorGUILayout.LabelField($"Downloading: {downloadStatus}", EditorStyles.miniLabel);
                EditorGUI.ProgressBar(EditorGUILayout.GetControlRect(), downloadProgress, 
                    $"{(downloadProgress * 100):F1}%");
                Repaint(); // Update UI during download
            }
            
            if (!isDownloading && !existsAtDefault)
            {
                EditorGUILayout.BeginHorizontal();
                executablePath = EditorGUILayout.TextField(executablePath);
                EditorGUILayout.EndHorizontal();
            }
        }
        else
        {
            EditorGUILayout.HelpBox($"Executable found: {Path.GetFileName(executablePath)}", MessageType.Info);
            EditorGUILayout.LabelField(executablePath, EditorStyles.miniLabel);
            
            // Check for updates button
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Check for Updates", GUILayout.Width(120)))
            {
                CheckForUpdates();
            }
            if (GUILayout.Button("Re-download", GUILayout.Width(100)))
            {
                if (EditorUtility.DisplayDialog("Re-download Executable", 
                    "This will download the latest version. Continue?", "Yes", "No"))
                {
                    StartDownload();
                }
            }
            EditorGUILayout.EndHorizontal();
        }
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // LLM/Ollama Generation Parameters
        EditorGUILayout.LabelField("LLM Generation Parameters", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        modelName = EditorGUILayout.TextField("Model Name:", modelName);
        EditorGUILayout.HelpBox("Ollama model name (e.g., llama3, mistral, gemma)", MessageType.None);
        
        EditorGUILayout.Space(5);
        temperature = EditorGUILayout.Slider("Temperature:", temperature, 0.0f, 2.0f);
        EditorGUILayout.HelpBox("Controls randomness. Lower = more deterministic, Higher = more creative", MessageType.None);
        
        topP = EditorGUILayout.Slider("Top P:", topP, 0.0f, 1.0f);
        EditorGUILayout.HelpBox("Nucleus sampling. Controls diversity of token selection", MessageType.None);
        
        maxTokens = EditorGUILayout.IntField("Max Tokens:", maxTokens);
        EditorGUILayout.HelpBox("Maximum tokens in response (typically 2048-8192)", MessageType.None);
        
        itemCount = EditorGUILayout.IntField("Item Count:", itemCount);
        EditorGUILayout.HelpBox("Number of items to generate", MessageType.None);
        
        EditorGUILayout.Space(5);
        presetName = EditorGUILayout.TextField("Preset Name (Optional):", presetName);
        EditorGUILayout.HelpBox("User-defined preset name for world context (e.g., 'Forest', 'Desert', 'My Custom World'). Leave empty to use profile's customContext or default.", MessageType.None);
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Player Profile
        EditorGUILayout.LabelField("Player Profile", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (playerProfileManagerProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No player profiles found. Create a new profile in the Player Profile Manager tab.", 
                MessageType.Warning);
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadPlayerProfileManagerProfiles();
            }
            if (GUILayout.Button("Go to Player Profile Manager", GUILayout.Width(180)))
            {
                selectedTab = 1; // Switch to Player Profile Manager tab
            }
            EditorGUILayout.EndHorizontal();
        }
        else
        {
            EditorGUILayout.LabelField("Select Player Profile:", EditorStyles.miniLabel);
            string[] playerProfileNames = playerProfileManagerProfiles.Select(p => $"{p.displayName} ({p.id})").ToArray();
            int currentPlayerProfileIndex = selectedPlayerProfile != null 
                ? playerProfileManagerProfiles.IndexOf(selectedPlayerProfile) 
                : -1;
            if (currentPlayerProfileIndex < 0) currentPlayerProfileIndex = 0;
            
            int newPlayerProfileIndex = EditorGUILayout.Popup(currentPlayerProfileIndex >= 0 ? currentPlayerProfileIndex : 0, playerProfileNames);
            if (newPlayerProfileIndex >= 0 && newPlayerProfileIndex < playerProfileManagerProfiles.Count)
            {
                if (selectedPlayerProfile != playerProfileManagerProfiles[newPlayerProfileIndex])
                {
                    selectedPlayerProfile = playerProfileManagerProfiles[newPlayerProfileIndex];
                    LoadPlayerSettingsFromPlayerProfile();
                    SaveSettings();
                }
            }
            
            if (selectedPlayerProfile != null)
            {
                EditorGUILayout.Space(5);
                EditorGUILayout.LabelField("Profile Info:", EditorStyles.miniLabel);
                EditorGUILayout.LabelField($"Name: {selectedPlayerProfile.displayName}", EditorStyles.wordWrappedLabel);
                if (!string.IsNullOrEmpty(selectedPlayerProfile.description))
                {
                    EditorGUILayout.LabelField($"Description: {selectedPlayerProfile.description}", EditorStyles.wordWrappedLabel);
                }
            }
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadPlayerProfileManagerProfiles();
            }
            if (GUILayout.Button("Go to Player Profile Manager", GUILayout.Width(180)))
            {
                selectedTab = 1; // Switch to Player Profile Manager tab
            }
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Item Profile
        EditorGUILayout.LabelField("Item Profile", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (availableProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No profiles found. Create a new profile in the Item Profile Manager tab.", 
                MessageType.Warning);
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadProfiles();
            }
            if (GUILayout.Button("Go to Item Profile Manager", GUILayout.Width(180)))
            {
                selectedTab = 2; // Switch to Item Profile Manager tab
            }
            EditorGUILayout.EndHorizontal();
        }
        else
        {
            EditorGUILayout.LabelField("Select Profile:", EditorStyles.miniLabel);
            int newIndex = EditorGUILayout.Popup(selectedProfileIndex, profileNames);
            if (newIndex != selectedProfileIndex && newIndex >= 0 && newIndex < availableProfiles.Count)
            {
                selectedProfileIndex = newIndex;
                selectedProfile = availableProfiles[newIndex];
                LoadPlayerSettingsFromProfile();
                UpdateOutputPath();
                SaveSettings();
            }
            
            if (selectedProfile != null)
            {
                EditorGUILayout.Space(5);
                EditorGUILayout.LabelField("Profile Info:", EditorStyles.miniLabel);
                EditorGUILayout.LabelField($"Name: {selectedProfile.displayName}", EditorStyles.wordWrappedLabel);
                EditorGUILayout.LabelField($"Type: {selectedProfile.itemTypeName}", EditorStyles.wordWrappedLabel);
                if (!string.IsNullOrEmpty(selectedProfile.description))
                {
                    EditorGUILayout.LabelField($"Description: {selectedProfile.description}", EditorStyles.wordWrappedLabel);
                }
                
                if (selectedProfile.playerSettings != null)
                {
                    EditorGUILayout.Space(3);
                    EditorGUILayout.HelpBox($"Note: This Item Profile has its own Player Settings that will override the selected Player Profile.", MessageType.Info);
                }
            }
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadProfiles();
            }
            if (GUILayout.Button("Go to Item Profile Manager", GUILayout.Width(180)))
            {
                selectedTab = 2; // Switch to Item Profile Manager tab
            }
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Output Settings
        EditorGUILayout.LabelField("Output Settings", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (selectedProfile != null)
        {
            EditorGUILayout.HelpBox($"Output path is automatically set based on selected profile: {selectedProfile.id}", MessageType.Info);
        }
        
        EditorGUILayout.LabelField("Output Path:", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        outputPath = EditorGUILayout.TextField(outputPath);
        if (GUILayout.Button("Browse", GUILayout.Width(70)))
        {
            string defaultDir = string.IsNullOrEmpty(outputPath) 
                ? Application.dataPath 
                : Path.GetDirectoryName(outputPath);
            string path = EditorUtility.SaveFilePanel("Save Generated Items", 
                defaultDir, 
                Path.GetFileName(outputPath), "json");
            if (!string.IsNullOrEmpty(path))
            {
                outputPath = path;
                SaveSettings();
            }
        }
        if (selectedProfile != null && GUILayout.Button("Auto", GUILayout.Width(50)))
        {
            UpdateOutputPath();
            SaveSettings();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Options
        EditorGUILayout.LabelField("Options", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        autoImport = EditorGUILayout.Toggle("Auto-Import After Generation", autoImport);
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Generate Button
        EditorGUI.BeginDisabledGroup(isGenerating || selectedProfile == null || !File.Exists(executablePath));
        if (GUILayout.Button(isGenerating ? "Generating..." : "Generate Items", GUILayout.Height(40)))
        {
            GenerateItems();
        }
        EditorGUI.EndDisabledGroup();
        
        EditorGUILayout.Space(10);
        
        // Log
        EditorGUILayout.LabelField("Log", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox, GUILayout.Height(200));
        logScrollPosition = EditorGUILayout.BeginScrollView(logScrollPosition);
        EditorGUILayout.TextArea(logText, GUILayout.ExpandHeight(true));
        EditorGUILayout.EndScrollView();
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.EndScrollView();
        
        // Update during generation
        if (isGenerating && currentProcess != null && currentProcess.HasExited)
        {
            OnGenerationComplete();
        }
    }
    
    private void DrawItemProfileManagerTab()
    {
        // Ensure profiles are loaded when tab is shown
        if (itemProfileManagerProfiles.Count == 0 && itemProfileAvailableItemTypes.Length == 1)
        {
            LoadItemProfileManagerProfiles();
        }
        
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Item Profile Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Create and manage custom item profiles. Profiles define the structure of items " +
            "that will be generated. You can add custom fields with types and validation rules.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Toolbar
        EditorGUILayout.BeginHorizontal();
        
        // Item Type Filter (dynamically generated from loaded profiles)
        EditorGUILayout.LabelField("Item Type:", GUILayout.Width(70));
        int currentIndex = Array.IndexOf(itemProfileAvailableItemTypes, itemProfileSelectedItemType);
        if (currentIndex < 0) currentIndex = 0;
        int newIndex = EditorGUILayout.Popup(currentIndex, itemProfileAvailableItemTypes, GUILayout.Width(150));
        if (newIndex >= 0 && newIndex < itemProfileAvailableItemTypes.Length)
        {
            itemProfileSelectedItemType = itemProfileAvailableItemTypes[newIndex];
        }
        
        GUILayout.FlexibleSpace();
        
        // Search
        EditorGUILayout.LabelField("Search:", GUILayout.Width(50));
        itemProfileSearchFilter = EditorGUILayout.TextField(itemProfileSearchFilter, GUILayout.Width(200));
        
        // Refresh
        if (GUILayout.Button("Refresh", GUILayout.Width(80)))
        {
            LoadItemProfileManagerProfiles();
            LoadProfiles(); // Also refresh Item Factory tab profiles
        }
        
        // Create New Profile
        if (GUILayout.Button("New Profile", GUILayout.Width(100)))
        {
            CreateNewItemProfileManager();
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Split view: Profile list and editor
        EditorGUILayout.BeginHorizontal(GUILayout.ExpandHeight(true));
        
        // Left: Profile List
        EditorGUILayout.BeginVertical(GUILayout.Width(300), GUILayout.ExpandHeight(true));
        DrawItemProfileManagerList();
        EditorGUILayout.EndVertical();
        
        // Right: Profile Editor
        EditorGUILayout.BeginVertical(GUILayout.ExpandWidth(true), GUILayout.ExpandHeight(true));
        if (itemProfileManagerSelectedProfile != null)
        {
            DrawItemProfileManagerEditor();
        }
        else
        {
            EditorGUILayout.HelpBox("Select a profile to edit, or create a new one.", MessageType.Info);
        }
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.EndHorizontal();
    }
    
    private void LoadProfiles()
    {
        availableProfiles.Clear();
        
        // Determine profiles directory (Assets/RundeeItemFactory/ItemProfiles)
        profilesPath = Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles");
        if (!Directory.Exists(profilesPath))
        {
            Directory.CreateDirectory(profilesPath);
            return;
        }
        
        string[] files = Directory.GetFiles(profilesPath, "*.json");
        foreach (string file in files)
        {
            try
            {
                string json = File.ReadAllText(file);
                ItemProfile profile = JsonUtility.FromJson<ItemProfile>(json);
                if (profile != null)
                {
                    availableProfiles.Add(profile);
                }
            }
            catch (Exception e)
            {
                UnityEngine.Debug.LogWarning($"[ItemFactoryMainWindow] Failed to load profile {file}: {e.Message}");
            }
        }
        
        // Update profile names array
        profileNames = availableProfiles.Select(p => $"{p.displayName} ({p.id})").ToArray();
        
        // Restore selected profile
        if (availableProfiles.Count > 0)
        {
            if (selectedProfile == null || !availableProfiles.Contains(selectedProfile))
            {
                selectedProfileIndex = 0;
                selectedProfile = availableProfiles[0];
            }
            else
            {
                selectedProfileIndex = availableProfiles.IndexOf(selectedProfile);
            }
            
            // Load player settings and update output path when profile is loaded/selected
            LoadPlayerSettingsFromProfile();
            UpdateOutputPath();
        }
    }
    
    private void LoadSettings()
    {
        executablePath = EditorPrefs.GetString("ItemFactory_ExecutablePath", "");
        modelName = EditorPrefs.GetString("ItemFactory_ModelName", "llama3");
        temperature = EditorPrefs.GetFloat("ItemFactory_Temperature", 0.7f);
        topP = EditorPrefs.GetFloat("ItemFactory_TopP", 0.9f);
        maxTokens = EditorPrefs.GetInt("ItemFactory_MaxTokens", 4096);
        itemCount = EditorPrefs.GetInt("ItemFactory_ItemCount", 10);
        presetName = EditorPrefs.GetString("ItemFactory_PresetName", "");
        maxHunger = EditorPrefs.GetInt("ItemFactory_MaxHunger", 100);
        maxThirst = EditorPrefs.GetInt("ItemFactory_MaxThirst", 100);
        maxHealth = EditorPrefs.GetInt("ItemFactory_MaxHealth", 100);
        maxStamina = EditorPrefs.GetInt("ItemFactory_MaxStamina", 100);
        maxWeight = EditorPrefs.GetInt("ItemFactory_MaxWeight", 50000);
        maxEnergy = EditorPrefs.GetInt("ItemFactory_MaxEnergy", 100);
        autoImport = EditorPrefs.GetBool("ItemFactory_AutoImport", true);
        
        // Auto-detect executable path with more comprehensive search
        if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
        {
            FindExecutablePath();
            
            // If still not found, check default download location
            if (string.IsNullOrEmpty(executablePath) || !File.Exists(executablePath))
            {
                string defaultPath = ExecutableDownloader.GetDownloadPath();
                if (File.Exists(defaultPath))
                {
                    executablePath = defaultPath;
                    SaveSettings();
                }
            }
        }
    }
    
    private void FindExecutablePath()
    {
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        
        // First check default download location
        string defaultPath = ExecutableDownloader.GetDownloadPath();
        if (File.Exists(defaultPath))
        {
            executablePath = defaultPath;
            SaveSettings();
            return;
        }
        
        string[] possiblePaths = new string[]
        {
            // Standard build locations
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Release", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Debug", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Release", "ItemGenerator.exe"),
            Path.Combine(projectRoot, "RundeeItemFactory", "x64", "Debug", "ItemGenerator.exe"),
            // Alternative locations
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Release", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Debug", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Release", "ItemGenerator.exe"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64", "Debug", "ItemGenerator.exe"),
            // Deployment folder
            Path.Combine(projectRoot, "..", "Deployment", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "Deployment", "RundeeItemFactory.exe"),
            // Root level
            Path.Combine(projectRoot, "..", "RundeeItemFactory.exe"),
            Path.Combine(projectRoot, "RundeeItemFactory.exe"),
        };
        
        foreach (string path in possiblePaths)
        {
            string fullPath = Path.GetFullPath(path);
            if (File.Exists(fullPath))
            {
                executablePath = fullPath;
                SaveSettings();
                return;
            }
        }
        
        // If not found, try to find any .exe in common build directories
        string[] searchDirs = new string[]
        {
            Path.Combine(projectRoot, "RundeeItemFactory", "x64"),
            Path.Combine(projectRoot, "..", "RundeeItemFactory", "x64"),
        };
        
        foreach (string dir in searchDirs)
        {
            if (Directory.Exists(dir))
            {
                string[] exes = Directory.GetFiles(dir, "*.exe", SearchOption.AllDirectories);
                if (exes.Length > 0)
                {
                    executablePath = Path.GetFullPath(exes[0]);
                    SaveSettings();
                    return;
                }
            }
        }
    }
    
    private void StartDownload()
    {
        if (isDownloading)
        {
            EditorUtility.DisplayDialog("Download in Progress", 
                "A download is already in progress. Please wait.", "OK");
            return;
        }
        
        isDownloading = true;
        downloadProgress = 0f;
        downloadStatus = "Connecting...";
        
        AddLog("Starting executable download...");
        
        ExecutableDownloader.DownloadExecutable(
            onProgress: (progress) =>
            {
                downloadProgress = progress;
                downloadStatus = $"Downloading... {(progress * 100):F1}%";
                Repaint();
            },
            onComplete: (success, error) =>
            {
                isDownloading = false;
                
                if (success)
                {
                    downloadStatus = "Download complete!";
                    AddLog("Executable downloaded successfully");
                    
                    // Auto-set the downloaded path
                    string downloadedPath = ExecutableDownloader.GetDownloadPath();
                    if (File.Exists(downloadedPath))
                    {
                        executablePath = downloadedPath;
                        SaveSettings();
                        EditorUtility.DisplayDialog("Download Complete", 
                            $"Executable downloaded successfully to:\n{downloadedPath}", "OK");
                    }
                }
                else
                {
                    downloadStatus = "Download failed";
                    AddLog($"Download failed: {error}", true);
                    EditorUtility.DisplayDialog("Download Failed", 
                        $"Failed to download executable:\n{error}\n\nPlease try again or download manually.", "OK");
                }
                
                Repaint();
            }
        );
    }
    
    private void CheckForUpdates()
    {
        AddLog("Checking for updates...");
        
        ExecutableDownloader.CheckVersion((version) =>
        {
            if (!string.IsNullOrEmpty(version))
            {
                AddLog($"Latest version available: {version}");
                bool download = EditorUtility.DisplayDialog("Update Available", 
                    $"A new version is available: {version}\n\nWould you like to download it?", 
                    "Download", "Cancel");
                
                if (download)
                {
                    StartDownload();
                }
            }
            else
            {
                AddLog("Could not check for updates. You can try downloading manually.");
            }
        });
    }
    
    private void SaveSettings()
    {
        EditorPrefs.SetString("ItemFactory_ExecutablePath", executablePath);
        EditorPrefs.SetString("ItemFactory_ModelName", modelName);
        EditorPrefs.SetFloat("ItemFactory_Temperature", temperature);
        EditorPrefs.SetFloat("ItemFactory_TopP", topP);
        EditorPrefs.SetInt("ItemFactory_MaxTokens", maxTokens);
        EditorPrefs.SetInt("ItemFactory_ItemCount", itemCount);
        EditorPrefs.SetString("ItemFactory_PresetName", presetName);
        EditorPrefs.SetInt("ItemFactory_MaxHunger", maxHunger);
        EditorPrefs.SetInt("ItemFactory_MaxThirst", maxThirst);
        EditorPrefs.SetInt("ItemFactory_MaxHealth", maxHealth);
        EditorPrefs.SetInt("ItemFactory_MaxStamina", maxStamina);
        EditorPrefs.SetInt("ItemFactory_MaxWeight", maxWeight);
        EditorPrefs.SetInt("ItemFactory_MaxEnergy", maxEnergy);
        EditorPrefs.SetBool("ItemFactory_AutoImport", autoImport);
        
        // Save selected player profile ID
        if (selectedPlayerProfile != null)
        {
            EditorPrefs.SetString("ItemFactory_SelectedPlayerProfileId", selectedPlayerProfile.id);
        }
        else
        {
            EditorPrefs.DeleteKey("ItemFactory_SelectedPlayerProfileId");
        }
    }
    
    private void LoadPlayerSettingsFromProfile()
    {
        if (selectedProfile != null && selectedProfile.playerSettings != null)
        {
            maxHunger = selectedProfile.playerSettings.maxHunger;
            maxThirst = selectedProfile.playerSettings.maxThirst;
            maxHealth = selectedProfile.playerSettings.maxHealth;
            maxStamina = selectedProfile.playerSettings.maxStamina;
            maxWeight = selectedProfile.playerSettings.maxWeight;
            maxEnergy = selectedProfile.playerSettings.maxEnergy;
            UnityEngine.Debug.Log($"[ItemFactoryMainWindow] Loaded Player Settings from Item Profile '{selectedProfile.id}'");
        }
        else
        {
            // Fallback to Player Profile or EditorPrefs
            LoadPlayerSettingsFromPlayerProfile();
        }
    }
    
    private void LoadPlayerSettingsFromPlayerProfile()
    {
        if (selectedPlayerProfile != null)
        {
            // First try to load from statSections (new dynamic system)
            if (selectedPlayerProfile.statSections != null && selectedPlayerProfile.statSections.Count > 0)
            {
                foreach (var section in selectedPlayerProfile.statSections)
                {
                    if (section.fields != null)
                    {
                        foreach (var field in section.fields)
                        {
                            // Map field names to variables
                            switch (field.name.ToLower())
                            {
                                case "maxhealth":
                                case "max_health":
                                    maxHealth = field.value;
                                    break;
                                case "maxhunger":
                                case "max_hunger":
                                    maxHunger = field.value;
                                    break;
                                case "maxthirst":
                                case "max_thirst":
                                    maxThirst = field.value;
                                    break;
                                case "maxstamina":
                                case "max_stamina":
                                    maxStamina = field.value;
                                    break;
                                case "maxenergy":
                                case "max_energy":
                                    maxEnergy = field.value;
                                    break;
                                case "maxweight":
                                case "max_weight":
                                    maxWeight = field.value;
                                    break;
                            }
                        }
                    }
                }
                UnityEngine.Debug.Log($"[ItemFactoryMainWindow] Loaded Player Settings from Player Profile '{selectedPlayerProfile.id}' (statSections)");
            }
            // Fallback to legacy playerSettings
            else if (selectedPlayerProfile.playerSettings != null)
            {
                maxHunger = selectedPlayerProfile.playerSettings.maxHunger;
                maxThirst = selectedPlayerProfile.playerSettings.maxThirst;
                maxHealth = selectedPlayerProfile.playerSettings.maxHealth;
                maxStamina = selectedPlayerProfile.playerSettings.maxStamina;
                maxWeight = selectedPlayerProfile.playerSettings.maxWeight;
                maxEnergy = selectedPlayerProfile.playerSettings.maxEnergy;
                UnityEngine.Debug.Log($"[ItemFactoryMainWindow] Loaded Player Settings from Player Profile '{selectedPlayerProfile.id}' (legacy playerSettings)");
            }
        }
        else
        {
            // Fallback to EditorPrefs
            maxHunger = EditorPrefs.GetInt("ItemFactory_MaxHunger", 100);
            maxThirst = EditorPrefs.GetInt("ItemFactory_MaxThirst", 100);
            maxHealth = EditorPrefs.GetInt("ItemFactory_MaxHealth", 100);
            maxStamina = EditorPrefs.GetInt("ItemFactory_MaxStamina", 100);
            maxWeight = EditorPrefs.GetInt("ItemFactory_MaxWeight", 50000);
            maxEnergy = EditorPrefs.GetInt("ItemFactory_MaxEnergy", 100);
            UnityEngine.Debug.Log("[ItemFactoryMainWindow] No Player Profile selected, loaded from EditorPrefs.");
        }
    }
    
    private void UpdateOutputPath()
    {
        if (selectedProfile != null)
        {
            string projectRoot = Application.dataPath.Replace("/Assets", "");
            string fileName = $"items_{selectedProfile.id}.json";
            outputPath = Path.Combine(projectRoot, "ItemJson", fileName);
        }
    }
    
    private void GenerateItems()
    {
        if (selectedProfile == null)
        {
            AddLog("Error: No profile selected", true);
            return;
        }
        
        if (!File.Exists(executablePath))
        {
            AddLog("Error: Executable not found", true);
            return;
        }
        
        // Build command line arguments
        List<string> args = new List<string>();
        args.Add("--profile");
        args.Add(selectedProfile.id);
        args.Add("--model");
        args.Add(modelName);
        args.Add("--count");
        args.Add(itemCount.ToString());
        
        // Preset name (user-defined world context)
        if (!string.IsNullOrEmpty(presetName))
        {
            args.Add("--preset");
            args.Add(presetName);
        }
        
        // Player stats (used for item balance calculations)
        args.Add("--maxHunger");
        args.Add(maxHunger.ToString());
        args.Add("--maxThirst");
        args.Add(maxThirst.ToString());
        args.Add("--maxHealth");
        args.Add(maxHealth.ToString());
        args.Add("--maxStamina");
        args.Add(maxStamina.ToString());
        args.Add("--maxWeight");
        args.Add(maxWeight.ToString());
        args.Add("--maxEnergy");
        args.Add(maxEnergy.ToString());
        
        args.Add("--output");
        args.Add(outputPath);
        
        string argsString = string.Join(" ", args.Select(a => $"\"{a}\""));
        
        AddLog($"Starting generation with profile: {selectedProfile.displayName}");
        AddLog($"Command: {executablePath} {argsString}");
        
        try
        {
            ProcessStartInfo psi = new ProcessStartInfo
            {
                FileName = executablePath,
                Arguments = argsString,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            };
            
            currentProcess = Process.Start(psi);
            isGenerating = true;
            
            // Read output asynchronously
            currentProcess.OutputDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    AddLog(e.Data);
                }
            };
            currentProcess.ErrorDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    AddLog(e.Data, true);
                }
            };
            
            currentProcess.BeginOutputReadLine();
            currentProcess.BeginErrorReadLine();
            
            SaveSettings();
        }
        catch (Exception e)
        {
            AddLog($"Error starting process: {e.Message}", true);
            isGenerating = false;
        }
    }
    
    private void OnGenerationComplete()
    {
        isGenerating = false;
        int exitCode = currentProcess.ExitCode;
        
        if (exitCode == 0)
        {
            AddLog("Generation completed successfully!");
            
            if (autoImport && File.Exists(outputPath))
            {
                AddLog("Starting import...");
                ImportJson(outputPath);
            }
        }
        else
        {
            AddLog($"Generation failed with exit code: {exitCode}", true);
        }
        
        currentProcess = null;
    }
    
    private void ImportJson(string jsonPath)
    {
        if (selectedProfile == null)
        {
            AddLog("Error: Cannot import without profile", true);
            return;
        }
        
        try
        {
            int imported = DynamicItemImporter.ImportFromJson(jsonPath, selectedProfile.id, selectedProfile.itemTypeName);
            AddLog($"Successfully imported {imported} items");
        }
        catch (Exception e)
        {
            AddLog($"Import failed: {e.Message}", true);
        }
    }
    
    private void AddLog(string message, bool isError = false)
    {
        string timestamp = DateTime.Now.ToString("HH:mm:ss");
        string prefix = isError ? "[ERROR]" : "[INFO]";
        logText += $"[{timestamp}] {prefix} {message}\n";
        logScrollPosition.y = float.MaxValue; // Auto-scroll to bottom
        
        if (isError)
        {
            UnityEngine.Debug.LogError($"[ItemFactory] {message}");
        }
        else
        {
            UnityEngine.Debug.Log($"[ItemFactory] {message}");
        }
    }
    
    // Item Profile Manager Tab Methods
    private void LoadItemProfileManagerProfiles()
    {
        itemProfileManagerProfiles.Clear();
        
        // Determine profiles directory (project root/profiles)
        string pmProfilesPath = Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles");
        if (!Directory.Exists(pmProfilesPath))
        {
            Directory.CreateDirectory(pmProfilesPath);
            UpdateItemProfileManagerItemTypes();
            return;
        }
        
        string[] files = Directory.GetFiles(pmProfilesPath, "*.json");
        foreach (string file in files)
        {
            try
            {
                string json = File.ReadAllText(file);
                ItemProfile profile = JsonUtility.FromJson<ItemProfile>(json);
                
                // Ensure identity fields exist
                EnsureItemProfileManagerIdentityFields(profile);
                if (profile != null)
                {
                    itemProfileManagerProfiles.Add(profile);
                }
            }
            catch (Exception e)
            {
                UnityEngine.Debug.LogError($"Failed to load profile from {file}: {e.Message}");
            }
        }
        
        UpdateItemProfileManagerItemTypes();
    }
    
    private void UpdateItemProfileManagerItemTypes()
    {
        // Collect unique item types from loaded profiles
        var itemTypes = itemProfileManagerProfiles
            .Where(p => !string.IsNullOrEmpty(p.itemTypeName))
            .Select(p => p.itemTypeName)
            .Distinct()
            .OrderBy(t => t)
            .ToList();
        
        // Create array with "All" option first, then unique item types
        itemProfileAvailableItemTypes = new string[] { "All" }.Concat(itemTypes).ToArray();
        
        // If current selection is not in available types, reset to "All"
        if (!itemProfileAvailableItemTypes.Contains(itemProfileSelectedItemType))
        {
            itemProfileSelectedItemType = "All";
        }
    }
    
    private void DrawItemProfileManagerList()
    {
        string filterLabel = itemProfileSelectedItemType == "All" ? "All Profiles" : $"Profiles ({itemProfileSelectedItemType})";
        EditorGUILayout.LabelField(filterLabel, EditorStyles.boldLabel);
        
        itemProfileListScrollPosition = EditorGUILayout.BeginScrollView(itemProfileListScrollPosition, EditorStyles.helpBox);
        
        var filteredProfiles = itemProfileManagerProfiles.Where(p => 
            (itemProfileSelectedItemType == "All" || p.itemTypeName == itemProfileSelectedItemType) &&
            (string.IsNullOrEmpty(itemProfileSearchFilter) || 
             p.displayName.IndexOf(itemProfileSearchFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
             p.id.IndexOf(itemProfileSearchFilter, StringComparison.OrdinalIgnoreCase) >= 0))
            .OrderBy(p => p.isDefault ? 0 : 1)
            .ThenBy(p => p.itemTypeName)
            .ThenBy(p => p.displayName)
            .ToList();
        
        if (filteredProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No profiles found. Create a new profile to get started.", MessageType.Info);
        }
        else
        {
            foreach (var profile in filteredProfiles)
            {
                DrawItemProfileManagerListItem(profile);
            }
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawItemProfileManagerListItem(ItemProfile profile)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        bool isSelected = itemProfileManagerSelectedProfile == profile;
        bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
        
        if (newSelected != isSelected)
        {
            itemProfileManagerSelectedProfile = newSelected ? profile : null;
            if (itemProfileManagerSelectedProfile != null)
            {
                EnsureItemProfileManagerIdentityFields(itemProfileManagerSelectedProfile);
            }
            showItemProfileEditor = newSelected;
            GUI.FocusControl(null);
        }
        
        EditorGUILayout.BeginVertical();
        EditorGUILayout.LabelField(profile.displayName, EditorStyles.boldLabel);
        if (!string.IsNullOrEmpty(profile.description))
        {
            EditorGUILayout.LabelField(profile.description, EditorStyles.miniLabel);
        }
        EditorGUILayout.LabelField("ID: " + profile.id + (profile.isDefault ? " (Default)" : ""), EditorStyles.miniLabel);
        int customFieldsCount = profile.fields != null ? profile.fields.Count(f => !IsItemProfileManagerIdentityField(f.name)) : 0;
        EditorGUILayout.LabelField("Custom Fields: " + customFieldsCount, EditorStyles.miniLabel);
        EditorGUILayout.EndVertical();
        
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Delete", GUILayout.Width(60)))
        {
            if (EditorUtility.DisplayDialog("Delete Profile", 
                "Are you sure you want to delete profile '" + profile.displayName + "'?", 
                "Delete", "Cancel"))
            {
                DeleteItemProfileManager(profile);
            }
        }
        
        EditorGUILayout.EndHorizontal();
        EditorGUILayout.EndVertical();
    }
    
    private void DrawItemProfileManagerEditor()
    {
        if (itemProfileManagerSelectedProfile == null) return;
        
        EditorGUILayout.LabelField("Edit Profile", EditorStyles.boldLabel);
        
        itemProfileEditorScrollPosition = EditorGUILayout.BeginScrollView(itemProfileEditorScrollPosition);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        // Basic Info
        EditorGUILayout.LabelField("Basic Information", EditorStyles.boldLabel);
        
        EditorGUILayout.LabelField("Display Name:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("User-friendly name shown in the profile list (e.g., 'Food Items - Forest Setting')", MessageType.None);
        itemProfileManagerSelectedProfile.displayName = EditorGUILayout.TextField(itemProfileManagerSelectedProfile.displayName);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("ID:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Unique identifier for this profile (e.g., 'food_forest_profile'). Used internally and in file names.", MessageType.None);
        itemProfileManagerSelectedProfile.id = EditorGUILayout.TextField(itemProfileManagerSelectedProfile.id);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("Item Type:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Type of items this profile generates. You can use any custom type name (e.g., 'Food', 'Weapon', 'CustomItemType')", MessageType.None);
        itemProfileManagerSelectedProfile.itemTypeName = EditorGUILayout.TextField(itemProfileManagerSelectedProfile.itemTypeName);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("Description:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Optional description of this profile's purpose and usage", MessageType.None);
        itemProfileManagerSelectedProfile.description = EditorGUILayout.TextArea(itemProfileManagerSelectedProfile.description, GUILayout.Height(60));
        
        EditorGUILayout.Space(5);
        
        itemProfileManagerSelectedProfile.isDefault = EditorGUILayout.Toggle("Is Default Profile", itemProfileManagerSelectedProfile.isDefault);
        
        EditorGUILayout.Space();
        
        // Custom Context (World Background)
        EditorGUILayout.LabelField("World Context / Background", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Describe the world setting and background. This text will be used in prompts to guide item generation. " +
            "You can describe the environment, theme, style, etc. Leave empty to use preset context.",
            MessageType.Info);
        itemProfileManagerSelectedProfile.customContext = EditorGUILayout.TextArea(itemProfileManagerSelectedProfile.customContext, GUILayout.Height(100));
        
        EditorGUILayout.Space();
        
        // Player Settings
        EditorGUILayout.LabelField("Player Settings", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Player statistics used for item generation. These values define the maximum values for player attributes " +
            "and are used to balance generated items appropriately. These settings will be used when generating items with this profile.",
            MessageType.Info);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        EditorGUILayout.LabelField("Vital Stats", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Health:", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxHealth = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxHealth);
        EditorGUILayout.HelpBox("Maximum health points. Used to balance healthRestore values in items", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Hunger:", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxHunger = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxHunger);
        EditorGUILayout.HelpBox("Maximum hunger value. Used to balance hungerRestore values in food items", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Thirst:", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxThirst = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxThirst);
        EditorGUILayout.HelpBox("Maximum thirst value. Used to balance thirstRestore values in drink items", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("Physical Stats", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Stamina:", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxStamina = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxStamina);
        EditorGUILayout.HelpBox("Maximum stamina points. Used for stamina-related item effects", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Energy:", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxEnergy = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxEnergy);
        EditorGUILayout.HelpBox("Maximum energy points. Used for energy-related item effects", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(5);
        EditorGUILayout.LabelField("Inventory", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Weight (grams):", GUILayout.Width(120));
        itemProfileManagerSelectedProfile.playerSettings.maxWeight = EditorGUILayout.IntField(itemProfileManagerSelectedProfile.playerSettings.maxWeight);
        EditorGUILayout.HelpBox("Maximum carry weight in grams. Used to balance item weight values (e.g., 50000 = 50kg)", MessageType.None);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Identity Fields (Fixed)
        EditorGUILayout.LabelField("Identity Fields (Required)", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "These fields are always included in every item. They define the basic identity and classification of items.",
            MessageType.Info);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        DrawItemProfileManagerIdentityField("id", "ID", "Unique item identifier (e.g., 'food_canned_beans')", true);
        DrawItemProfileManagerIdentityField("displayName", "Display Name", "User-facing name (e.g., 'Canned Beans')", true);
        DrawItemProfileManagerIdentityField("category", "Category", "Item category (e.g., 'Food', 'Weapon')", false);
        DrawItemProfileManagerIdentityField("rarity", "Rarity", "Item rarity: Common, Uncommon, or Rare", false);
        DrawItemProfileManagerIdentityField("description", "Description", "Item description text", false);
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Custom Fields
        EditorGUILayout.LabelField("Custom Fields (" + GetItemProfileManagerCustomFieldsCount() + ")", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Add custom fields specific to your item type. These fields will be included in generated items.",
            MessageType.Info);
        
        itemProfileEditorScrollPosition = EditorGUILayout.BeginScrollView(itemProfileEditorScrollPosition, GUILayout.Height(300));
        
        for (int i = 0; i < itemProfileManagerSelectedProfile.fields.Count; i++)
        {
            // Skip identity fields
            if (IsItemProfileManagerIdentityField(itemProfileManagerSelectedProfile.fields[i].name))
                continue;
                
            DrawItemProfileManagerFieldEditor(itemProfileManagerSelectedProfile.fields[i], i);
        }
        
        EditorGUILayout.EndScrollView();
        
        // Add Field Button
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Add Custom Field", GUILayout.Height(30)))
        {
            var newField = new ProfileField
            {
                name = "newField",
                type = FieldType.String,
                displayName = "New Field",
                category = "General",
                displayOrder = GetItemProfileManagerCustomFieldsCount()
            };
            itemProfileManagerSelectedProfile.fields.Add(newField);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Save Button
        EditorGUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();
        if (GUILayout.Button("Save Profile", GUILayout.Width(120), GUILayout.Height(30)))
        {
            SaveItemProfileManager(itemProfileManagerSelectedProfile);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndScrollView();
    }
    
    // Profile Manager Helper Methods
    private static readonly string[] ItemProfileManagerFieldCategories = new string[]
    {
        "Identity",        // id, displayName
        "Classification",  // category, rarity
        "Stats",           // hungerRestore, healthRestore, damage
        "Properties",      // spoils, materialType, weaponType
        "Combat",          // accuracy, recoil, damage
        "Inventory",       // maxStack, weight
        "Effects",         // restore values, modifiers
        "General"          // 기�?
    };
    
    private static readonly string[] ItemProfileManagerIdentityFieldNames = new string[]
    {
        "id",
        "displayName",
        "category",
        "rarity",
        "description"
    };
    
    private bool IsItemProfileManagerIdentityField(string fieldName)
    {
        return Array.IndexOf(ItemProfileManagerIdentityFieldNames, fieldName) >= 0;
    }
    
    private int GetItemProfileManagerCustomFieldsCount()
    {
        if (itemProfileManagerSelectedProfile == null) return 0;
        return itemProfileManagerSelectedProfile.fields.Count(f => !IsItemProfileManagerIdentityField(f.name));
    }
    
    private void EnsureItemProfileManagerIdentityFields(ItemProfile profile)
    {
        if (profile == null || profile.fields == null) return;
        
        // Check and add missing identity fields
        foreach (string fieldName in ItemProfileManagerIdentityFieldNames)
        {
            if (!profile.fields.Any(f => f.name == fieldName))
            {
                var field = CreateItemProfileManagerIdentityField(fieldName);
                profile.fields.Add(field);
            }
        }
        
        // Sort fields: identity fields first, then custom fields
        profile.fields = profile.fields.OrderBy(f => 
        {
            int identityIndex = Array.IndexOf(ItemProfileManagerIdentityFieldNames, f.name);
            return identityIndex >= 0 ? identityIndex : 1000 + f.displayOrder;
        }).ToList();
    }
    
    private ProfileField CreateItemProfileManagerIdentityField(string fieldName)
    {
        string displayName = "";
        string description = "";
        bool isRequired = false;
        
        switch (fieldName)
        {
            case "id":
                displayName = "ID";
                description = "Unique item identifier (e.g., 'food_canned_beans')";
                isRequired = true;
                break;
            case "displayName":
                displayName = "Display Name";
                description = "User-facing name (e.g., 'Canned Beans')";
                isRequired = true;
                break;
            case "category":
                displayName = "Category";
                description = "Item category (e.g., 'Food', 'Weapon')";
                isRequired = false;
                break;
            case "rarity":
                displayName = "Rarity";
                description = "Item rarity: Common, Uncommon, or Rare";
                isRequired = false;
                break;
            case "description":
                displayName = "Description";
                description = "Item description text";
                isRequired = false;
                break;
        }
        
        var field = new ProfileField
        {
            name = fieldName,
            displayName = displayName,
            description = description,
            type = FieldType.String,
            category = "Identity",
            isRequired = isRequired,
            displayOrder = Array.IndexOf(ItemProfileManagerIdentityFieldNames, fieldName)
        };
        
        // Set default validation for specific fields
        if (fieldName == "id")
        {
            field.minLength = 1;
            field.maxLength = 100;
        }
        else if (fieldName == "displayName")
        {
            field.minLength = 1;
            field.maxLength = 100;
        }
        else if (fieldName == "rarity")
        {
            field.allowedValues = new List<string> { "Common", "Uncommon", "Rare" };
        }
        else if (fieldName == "description")
        {
            field.minLength = 0;
            field.maxLength = 500;
        }
        
        return field;
    }
    
    private void DrawItemProfileManagerIdentityField(string fieldName, string displayName, string description, bool isRequired)
    {
        // Find or create identity field
        ProfileField field = itemProfileManagerSelectedProfile.fields.FirstOrDefault(f => f.name == fieldName);
        if (field == null)
        {
            field = CreateItemProfileManagerIdentityField(fieldName);
            itemProfileManagerSelectedProfile.fields.Add(field);
        }
        
        // Display as read-only info (identity fields are fixed)
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField(displayName + ":", GUILayout.Width(120));
        EditorGUILayout.LabelField(field.type.ToString() + (isRequired ? " (Required)" : " (Optional)"), 
            EditorStyles.miniLabel);
        GUILayout.FlexibleSpace();
        EditorGUILayout.EndHorizontal();
        
        if (!string.IsNullOrEmpty(description))
        {
            EditorGUILayout.HelpBox(description, MessageType.None);
        }
    }
    
    private void DrawItemProfileManagerFieldEditor(ProfileField field, int index)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        EditorGUILayout.LabelField("Field " + (index + 1), EditorStyles.boldLabel, GUILayout.Width(80));
        
        GUILayout.FlexibleSpace();
        
        // Prevent removal of identity fields
        bool isIdentity = IsItemProfileManagerIdentityField(field.name);
        EditorGUI.BeginDisabledGroup(isIdentity);
        if (GUILayout.Button("Remove", GUILayout.Width(60)))
        {
            itemProfileManagerSelectedProfile.fields.RemoveAt(index);
            return;
        }
        EditorGUI.EndDisabledGroup();
        
        if (isIdentity)
        {
            EditorGUILayout.HelpBox("Identity fields cannot be removed.", MessageType.Info);
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(5);
        
        // Basic Info
        field.name = EditorGUILayout.TextField("Name:", field.name);
        field.displayName = EditorGUILayout.TextField("Display Name:", field.displayName);
        field.description = EditorGUILayout.TextArea(field.description, GUILayout.Height(40));
        
        EditorGUILayout.Space(5);
        
        // Category (Dropdown)
        EditorGUILayout.LabelField("Category:", EditorStyles.miniLabel);
        int categoryIndex = Array.IndexOf(ItemProfileManagerFieldCategories, field.category);
        if (categoryIndex < 0) categoryIndex = ItemProfileManagerFieldCategories.Length - 1; // Default to "General"
        int newCategoryIndex = EditorGUILayout.Popup(categoryIndex, ItemProfileManagerFieldCategories);
        field.category = ItemProfileManagerFieldCategories[newCategoryIndex];
        
        EditorGUILayout.Space(5);
        
        // Type and Settings
        field.type = (FieldType)EditorGUILayout.EnumPopup("Type:", field.type);
        field.displayOrder = EditorGUILayout.IntField("Display Order:", field.displayOrder);
        field.isRequired = EditorGUILayout.Toggle("Required:", field.isRequired);
        field.defaultValue = EditorGUILayout.TextField("Default Value:", field.defaultValue);
        
        EditorGUILayout.Space(5);
        
        // Type-specific Validation
        DrawItemProfileManagerTypeSpecificValidation(field);
        
        EditorGUILayout.EndVertical();
        EditorGUILayout.Space();
    }
    
    private void DrawItemProfileManagerTypeSpecificValidation(ProfileField field)
    {
        EditorGUILayout.LabelField("Validation", EditorStyles.boldLabel);
        
        switch (field.type)
        {
            case FieldType.String:
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Min Length:", GUILayout.Width(80));
                field.minLength = EditorGUILayout.IntField(field.minLength, GUILayout.Width(100));
                EditorGUILayout.LabelField("Max Length:", GUILayout.Width(80));
                field.maxLength = EditorGUILayout.IntField(field.maxLength, GUILayout.Width(100));
                EditorGUILayout.EndHorizontal();
                
                // Allowed Values (for String with enum-like behavior)
                EditorGUILayout.Space(3);
                EditorGUILayout.LabelField("Allowed Values (comma-separated, optional):", EditorStyles.miniLabel);
                string allowedValuesStr = string.Join(", ", field.allowedValues);
                allowedValuesStr = EditorGUILayout.TextField(allowedValuesStr);
                if (allowedValuesStr != string.Join(", ", field.allowedValues))
                {
                    field.allowedValues = allowedValuesStr.Split(',')
                        .Select(v => v.Trim())
                        .Where(v => !string.IsNullOrEmpty(v))
                        .ToList();
                }
                break;
                
            case FieldType.Integer:
            case FieldType.Float:
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Min Value:", GUILayout.Width(80));
                field.minValue = EditorGUILayout.DoubleField(field.minValue, GUILayout.Width(100));
                EditorGUILayout.LabelField("Max Value:", GUILayout.Width(80));
                field.maxValue = EditorGUILayout.DoubleField(field.maxValue, GUILayout.Width(100));
                EditorGUILayout.EndHorizontal();
                break;
                
            case FieldType.Boolean:
                EditorGUILayout.HelpBox("Boolean fields don't require validation.", MessageType.None);
                break;
                
            case FieldType.Array:
            case FieldType.Object:
                EditorGUILayout.HelpBox("Complex types (Array/Object) validation is handled by the parser.", MessageType.Info);
                break;
        }
    }
    
    private void SaveItemProfileManager(ItemProfile profile, bool showDialog = true)
    {
        if (string.IsNullOrEmpty(profile.id))
        {
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Error", "Profile ID cannot be empty.", "OK");
            }
            return;
        }
        
        string filePath = Path.Combine(Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles"), profile.id + ".json");
        try
        {
            string json = JsonUtility.ToJson(profile, true);
            File.WriteAllText(filePath, json);
            AssetDatabase.Refresh();
            
            LoadItemProfileManagerProfiles();
            LoadProfiles(); // Also refresh Item Factory tab profiles
            
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Success", "Profile saved successfully.", "OK");
            }
        }
        catch (Exception e)
        {
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Error", "Failed to save profile: " + e.Message, "OK");
            }
        }
    }
    
    private void DeleteItemProfileManager(ItemProfile profile)
    {
        string filePath = Path.Combine(Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles"), profile.id + ".json");
        
        // Remove from memory lists first
        itemProfileManagerProfiles.Remove(profile);
        if (itemProfileManagerSelectedProfile == profile)
        {
            itemProfileManagerSelectedProfile = null;
            showItemProfileEditor = false;
        }
        
        // Also remove from Item Factory tab if selected
        if (selectedProfile == profile)
        {
            selectedProfile = null;
            selectedProfileIndex = 0;
        }
        availableProfiles.Remove(profile);
        
        // Delete file if it exists
        if (File.Exists(filePath))
        {
            File.Delete(filePath);
            AssetDatabase.Refresh();
        }
        
        // Reload profiles to ensure consistency
        LoadItemProfileManagerProfiles();
        LoadProfiles(); // Refresh Item Factory tab profiles
        
        // Update profile names array
        profileNames = availableProfiles.Select(p => $"{p.displayName} ({p.id})").ToArray();
        
        // Clear selection if no profiles left
        if (availableProfiles.Count == 0)
        {
            selectedProfile = null;
            selectedProfileIndex = 0;
        }
    }
    
    private void CreateNewItemProfileManager()
    {
        var newProfile = new ItemProfile
        {
            id = "profile_" + System.Guid.NewGuid().ToString().Substring(0, 8),
            displayName = "New Profile",
            description = "",
            itemTypeName = "", // User will set this manually
            version = 1,
            isDefault = false,
            fields = new List<ProfileField>(),
            playerSettings = new PlayerSettings()
        };
        
        // Ensure identity fields are added
        EnsureItemProfileManagerIdentityFields(newProfile);
        
        itemProfileManagerProfiles.Add(newProfile);
        itemProfileManagerSelectedProfile = newProfile;
        showItemProfileEditor = true;
        
        // Auto-save the new profile immediately (without showing dialog)
        SaveItemProfileManager(newProfile, showDialog: false);
    }
    
    // Player Profile Manager Tab Methods
    private void DrawPlayerProfileManagerTab()
    {
        // Ensure profiles are loaded when tab is shown
        if (playerProfileManagerProfiles.Count == 0)
        {
            LoadPlayerProfileManagerProfiles();
        }
        
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Player Profile Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Create and manage player profiles. Player profiles define the maximum stats for players " +
            "and are used to balance generated items appropriately.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Toolbar
        EditorGUILayout.BeginHorizontal();
        
        // Search
        EditorGUILayout.LabelField("Search:", GUILayout.Width(50));
        playerProfileSearchFilter = EditorGUILayout.TextField(playerProfileSearchFilter, GUILayout.Width(200));
        
        GUILayout.FlexibleSpace();
        
        // Refresh
        if (GUILayout.Button("Refresh", GUILayout.Width(80)))
        {
            LoadPlayerProfileManagerProfiles();
        }
        
        // Create New Profile
        if (GUILayout.Button("New Profile", GUILayout.Width(100)))
        {
            CreateNewPlayerProfileManager();
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Split view: Profile list and editor
        EditorGUILayout.BeginHorizontal(GUILayout.ExpandHeight(true));
        
        // Left: Profile List
        EditorGUILayout.BeginVertical(GUILayout.Width(300), GUILayout.ExpandHeight(true));
        DrawPlayerProfileManagerList();
        EditorGUILayout.EndVertical();
        
        // Right: Profile Editor
        EditorGUILayout.BeginVertical(GUILayout.ExpandWidth(true), GUILayout.ExpandHeight(true));
        if (playerProfileManagerSelectedProfile != null)
        {
            DrawPlayerProfileManagerEditor();
        }
        else
        {
            EditorGUILayout.HelpBox("Select a profile to edit, or create a new one.", MessageType.Info);
        }
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.EndHorizontal();
    }
    
    private void LoadPlayerProfileManagerProfiles()
    {
        playerProfileManagerProfiles.Clear();
        
        // Determine player profiles directory (project root/player_profiles)
        string playerProfilesPath = Path.Combine(Application.dataPath, "RundeeItemFactory", "PlayerProfiles");
        if (!Directory.Exists(playerProfilesPath))
        {
            Directory.CreateDirectory(playerProfilesPath);
            return;
        }
        
        string[] files = Directory.GetFiles(playerProfilesPath, "*.json");
        foreach (string file in files)
        {
            try
            {
                string json = File.ReadAllText(file);
                PlayerProfile profile = JsonUtility.FromJson<PlayerProfile>(json);
                if (profile != null)
                {
                    playerProfileManagerProfiles.Add(profile);
                }
            }
            catch (Exception e)
            {
                UnityEngine.Debug.LogError($"Failed to load player profile from {file}: {e.Message}");
            }
        }
    }
    
    private void DrawPlayerProfileManagerList()
    {
        EditorGUILayout.LabelField("Player Profiles", EditorStyles.boldLabel);
        
        playerProfileListScrollPosition = EditorGUILayout.BeginScrollView(playerProfileListScrollPosition, EditorStyles.helpBox);
        
        var filteredProfiles = playerProfileManagerProfiles.Where(p => 
            string.IsNullOrEmpty(playerProfileSearchFilter) || 
            p.displayName.IndexOf(playerProfileSearchFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
            p.id.IndexOf(playerProfileSearchFilter, StringComparison.OrdinalIgnoreCase) >= 0)
            .OrderBy(p => p.isDefault ? 0 : 1)
            .ThenBy(p => p.displayName)
            .ToList();
        
        if (filteredProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No player profiles found. Create a new profile to get started.", MessageType.Info);
        }
        else
        {
            foreach (var profile in filteredProfiles)
            {
                DrawPlayerProfileManagerListItem(profile);
            }
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawPlayerProfileManagerListItem(PlayerProfile profile)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        bool isSelected = playerProfileManagerSelectedProfile == profile;
        bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
        
        if (newSelected != isSelected)
        {
            playerProfileManagerSelectedProfile = newSelected ? profile : null;
            showPlayerProfileEditor = newSelected;
            GUI.FocusControl(null);
        }
        
        EditorGUILayout.BeginVertical();
        EditorGUILayout.LabelField(profile.displayName, EditorStyles.boldLabel);
        if (!string.IsNullOrEmpty(profile.description))
        {
            EditorGUILayout.LabelField(profile.description, EditorStyles.miniLabel);
        }
        EditorGUILayout.LabelField("ID: " + profile.id + (profile.isDefault ? " (Default)" : ""), EditorStyles.miniLabel);
        EditorGUILayout.EndVertical();
        
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Delete", GUILayout.Width(60)))
        {
            if (EditorUtility.DisplayDialog("Delete Profile", 
                "Are you sure you want to delete player profile '" + profile.displayName + "'?", 
                "Delete", "Cancel"))
            {
                DeletePlayerProfileManager(profile);
            }
        }
        
        EditorGUILayout.EndHorizontal();
        EditorGUILayout.EndVertical();
    }
    
    private void DrawPlayerProfileManagerEditor()
    {
        if (playerProfileManagerSelectedProfile == null) return;
        
        EditorGUILayout.LabelField("Edit Player Profile", EditorStyles.boldLabel);
        
        playerProfileEditorScrollPosition = EditorGUILayout.BeginScrollView(playerProfileEditorScrollPosition);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        // Basic Info
        EditorGUILayout.LabelField("Basic Information", EditorStyles.boldLabel);
        
        EditorGUILayout.LabelField("Display Name:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("User-friendly name shown in the profile list (e.g., 'Standard Player', 'Hardcore Player')", MessageType.None);
        playerProfileManagerSelectedProfile.displayName = EditorGUILayout.TextField(playerProfileManagerSelectedProfile.displayName);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("ID:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Unique identifier for this profile (e.g., 'player_standard'). Used internally and in file names.", MessageType.None);
        playerProfileManagerSelectedProfile.id = EditorGUILayout.TextField(playerProfileManagerSelectedProfile.id);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("Description:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Optional description of this profile's purpose and usage", MessageType.None);
        playerProfileManagerSelectedProfile.description = EditorGUILayout.TextArea(playerProfileManagerSelectedProfile.description, GUILayout.Height(60));
        
        EditorGUILayout.Space(5);
        
        playerProfileManagerSelectedProfile.isDefault = EditorGUILayout.Toggle("Is Default Profile", playerProfileManagerSelectedProfile.isDefault);
        
        EditorGUILayout.Space();
        
        // Player Stats (Dynamic Sections)
        EditorGUILayout.LabelField("Player Stats", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Player statistics used for item generation. Organize stats into sections (e.g., 'Vital Stats', 'Physical Stats', 'Inventory'). " +
            "You can add custom sections and fields as needed.",
            MessageType.Info);
        
        // Ensure default sections exist
        EnsureDefaultStatSections(playerProfileManagerSelectedProfile);
        
        // Draw stat sections
        for (int sectionIndex = 0; sectionIndex < playerProfileManagerSelectedProfile.statSections.Count; sectionIndex++)
        {
            var section = playerProfileManagerSelectedProfile.statSections[sectionIndex];
            DrawPlayerStatSection(section, sectionIndex);
        }
        
        // Add Section Button
        EditorGUILayout.Space(5);
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Add Stat Section", GUILayout.Height(25)))
        {
            var newSection = new PlayerStatSection
            {
                name = "New Section",
                displayOrder = playerProfileManagerSelectedProfile.statSections.Count,
                fields = new List<PlayerStatField>()
            };
            playerProfileManagerSelectedProfile.statSections.Add(newSection);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Save Button
        EditorGUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();
        if (GUILayout.Button("Save Profile", GUILayout.Width(120), GUILayout.Height(30)))
        {
            SavePlayerProfileManager(playerProfileManagerSelectedProfile);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndScrollView();
    }
    
    private void SavePlayerProfileManager(PlayerProfile profile, bool showDialog = true)
    {
        if (string.IsNullOrEmpty(profile.id))
        {
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Error", "Profile ID cannot be empty.", "OK");
            }
            return;
        }
        
        string filePath = Path.Combine(Path.Combine(Application.dataPath, "RundeeItemFactory", "PlayerProfiles"), profile.id + ".json");
        try
        {
            string json = JsonUtility.ToJson(profile, true);
            File.WriteAllText(filePath, json);
            AssetDatabase.Refresh();
            
            LoadPlayerProfileManagerProfiles();
            
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Success", "Player profile saved successfully.", "OK");
            }
        }
        catch (Exception e)
        {
            if (showDialog)
            {
                EditorUtility.DisplayDialog("Error", "Failed to save player profile: " + e.Message, "OK");
            }
        }
    }
    
    private void DeletePlayerProfileManager(PlayerProfile profile)
    {
        string filePath = Path.Combine(Path.Combine(Application.dataPath, "RundeeItemFactory", "PlayerProfiles"), profile.id + ".json");
        
        // Remove from memory lists first
        playerProfileManagerProfiles.Remove(profile);
        if (playerProfileManagerSelectedProfile == profile)
        {
            playerProfileManagerSelectedProfile = null;
            showPlayerProfileEditor = false;
        }
        
        // Also remove from Item Factory tab if selected
        if (selectedPlayerProfile == profile)
        {
            selectedPlayerProfile = null;
        }
        
        // Delete file if it exists
        if (File.Exists(filePath))
        {
            File.Delete(filePath);
            AssetDatabase.Refresh();
        }
        
        // Reload profiles to ensure consistency
        LoadPlayerProfileManagerProfiles();
    }
    
    private void CreateNewPlayerProfileManager()
    {
        var newProfile = new PlayerProfile
        {
            id = "player_" + System.Guid.NewGuid().ToString().Substring(0, 8),
            displayName = "New Player Profile",
            description = "",
            version = 1,
            isDefault = false,
            playerSettings = new PlayerSettings()
        };
        
        // Ensure default stat sections exist
        EnsureDefaultStatSections(newProfile);
        
        playerProfileManagerProfiles.Add(newProfile);
        playerProfileManagerSelectedProfile = newProfile;
        showPlayerProfileEditor = true;
        
        // Auto-save the new profile immediately (without showing dialog)
        SavePlayerProfileManager(newProfile, showDialog: false);
    }
    
    private void EnsureDefaultStatSections(PlayerProfile profile)
    {
        if (profile.statSections == null)
        {
            profile.statSections = new List<PlayerStatSection>();
        }
        
        // Create default sections if they don't exist
        var sectionNames = new[] { "Vital Stats", "Physical Stats", "Inventory" };
        foreach (var sectionName in sectionNames)
        {
            if (!profile.statSections.Any(s => s.name == sectionName))
            {
                var section = new PlayerStatSection
                {
                    name = sectionName,
                    displayOrder = profile.statSections.Count,
                    fields = new List<PlayerStatField>()
                };
                
                // Add default fields based on section name
                if (sectionName == "Vital Stats")
                {
                    section.fields.Add(new PlayerStatField { name = "maxHealth", displayName = "Max Health", description = "Maximum health points", value = 100, displayOrder = 0 });
                    section.fields.Add(new PlayerStatField { name = "maxHunger", displayName = "Max Hunger", description = "Maximum hunger value", value = 100, displayOrder = 1 });
                    section.fields.Add(new PlayerStatField { name = "maxThirst", displayName = "Max Thirst", description = "Maximum thirst value", value = 100, displayOrder = 2 });
                }
                else if (sectionName == "Physical Stats")
                {
                    section.fields.Add(new PlayerStatField { name = "maxStamina", displayName = "Max Stamina", description = "Maximum stamina points", value = 100, displayOrder = 0 });
                    section.fields.Add(new PlayerStatField { name = "maxEnergy", displayName = "Max Energy", description = "Maximum energy points", value = 100, displayOrder = 1 });
                }
                else if (sectionName == "Inventory")
                {
                    section.fields.Add(new PlayerStatField { name = "maxWeight", displayName = "Max Weight (grams)", description = "Maximum carry weight in grams", value = 50000, displayOrder = 0 });
                }
                
                profile.statSections.Add(section);
            }
        }
        
        // Sort sections by display order
        profile.statSections = profile.statSections.OrderBy(s => s.displayOrder).ToList();
    }
    
    private void DrawPlayerStatSection(PlayerStatSection section, int sectionIndex)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField(section.name, EditorStyles.boldLabel);
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Add Field", GUILayout.Width(80), GUILayout.Height(20)))
        {
            var newField = new PlayerStatField
            {
                name = "newField_" + System.Guid.NewGuid().ToString().Substring(0, 6),
                displayName = "New Field",
                description = "",
                value = 0,
                displayOrder = section.fields.Count
            };
            section.fields.Add(newField);
        }
        
        if (GUILayout.Button("Remove Section", GUILayout.Width(100), GUILayout.Height(20)))
        {
            if (EditorUtility.DisplayDialog("Remove Section", 
                $"Are you sure you want to remove the '{section.name}' section?", 
                "Remove", "Cancel"))
            {
                playerProfileManagerSelectedProfile.statSections.RemoveAt(sectionIndex);
                return;
            }
        }
        EditorGUILayout.EndHorizontal();
        
        // Section name editor
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Section Name:", GUILayout.Width(100));
        section.name = EditorGUILayout.TextField(section.name);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(3);
        
        // Draw fields in this section
        if (section.fields == null)
        {
            section.fields = new List<PlayerStatField>();
        }
        
        for (int fieldIndex = 0; fieldIndex < section.fields.Count; fieldIndex++)
        {
            var field = section.fields[fieldIndex];
            DrawPlayerStatField(field, section, fieldIndex);
        }
        
        EditorGUILayout.EndVertical();
        EditorGUILayout.Space(5);
    }
    
    private void DrawPlayerStatField(PlayerStatField field, PlayerStatSection section, int fieldIndex)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        EditorGUILayout.LabelField("Field Name:", GUILayout.Width(80));
        field.name = EditorGUILayout.TextField(field.name, GUILayout.Width(150));
        
        EditorGUILayout.LabelField("Display Name:", GUILayout.Width(90));
        field.displayName = EditorGUILayout.TextField(field.displayName, GUILayout.Width(150));
        
        EditorGUILayout.LabelField("Value:", GUILayout.Width(50));
        field.value = EditorGUILayout.IntField(field.value, GUILayout.Width(100));
        
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Remove", GUILayout.Width(60), GUILayout.Height(20)))
        {
            section.fields.RemoveAt(fieldIndex);
            return;
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.LabelField("Description:", EditorStyles.miniLabel);
        field.description = EditorGUILayout.TextField(field.description);
        
        EditorGUILayout.EndVertical();
    }
}

