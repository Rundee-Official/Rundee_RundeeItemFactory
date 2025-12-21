using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Unity Editor window for managing ID registry files
/// </summary>
/// <remarks>
/// Provides functionality to:
/// - View registry contents by item type
/// - Search and filter IDs in registries
/// - Delete specific IDs from registries
/// - Clear entire registries
/// - View registry statistics
/// 
/// This window can be extended for Unreal Engine support by implementing
/// a similar UI using Slate or Editor Utility Widgets.
/// </remarks>
public class RegistryManagerWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string searchText = "";
    private Vector2 scrollPosition;
    private Vector2 statsScrollPosition;
    private bool showStats = true;
    private bool showRegistryContent = true;
    
    private Dictionary<ItemType, RegistryData> registries = new Dictionary<ItemType, RegistryData>();
    private HashSet<string> selectedIds = new HashSet<string>();
    
    private class RegistryData
    {
        public List<string> ids = new List<string>();
        public DateTime lastModified;
        public string filePath;
        public bool isLoaded = false;
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Management/Registry Manager", false, 2010)]
    public static void ShowWindow()
    {
        var window = GetWindow<RegistryManagerWindow>("Registry Manager");
        window.minSize = new Vector2(600, 400);
        window.Show();
    }
    
    private void OnEnable()
    {
        LoadAllRegistries();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        
        // Header
        EditorGUILayout.LabelField("ID Registry Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Manage item ID registries to prevent duplicate item generation. " +
            "Registries track all generated item IDs across multiple runs.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Item Type Selection
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Item Type:", GUILayout.Width(100));
        selectedType = (ItemType)EditorGUILayout.EnumPopup(selectedType);
        
        if (GUILayout.Button("Refresh", GUILayout.Width(80)))
        {
            LoadRegistry(selectedType);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Statistics Section
        showStats = EditorGUILayout.Foldout(showStats, "Statistics", true);
        if (showStats)
        {
            DrawStatistics();
        }
        
        EditorGUILayout.Space();
        
        // Registry Content Section
        showRegistryContent = EditorGUILayout.Foldout(showRegistryContent, "Registry Content", true);
        if (showRegistryContent)
        {
            DrawRegistryContent();
        }
        
        EditorGUILayout.Space();
        
        // Actions
        DrawActions();
    }
    
    private void DrawStatistics()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded)
        {
            EditorGUILayout.HelpBox("Registry not loaded. Click Refresh to load.", MessageType.Warning);
            EditorGUILayout.EndVertical();
            return;
        }
        
        var registry = registries[selectedType];
        var filteredIds = GetFilteredIds();
        
        EditorGUILayout.LabelField("Total IDs:", registry.ids.Count.ToString());
        EditorGUILayout.LabelField("Filtered IDs:", filteredIds.Count.ToString());
        EditorGUILayout.LabelField("Selected IDs:", selectedIds.Count.ToString());
        
        if (File.Exists(registry.filePath))
        {
            var fileInfo = new FileInfo(registry.filePath);
            EditorGUILayout.LabelField("Last Modified:", fileInfo.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss"));
            EditorGUILayout.LabelField("File Size:", FormatFileSize(fileInfo.Length));
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawRegistryContent()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded)
        {
            EditorGUILayout.HelpBox("Registry not loaded.", MessageType.Warning);
            EditorGUILayout.EndVertical();
            return;
        }
        
        // Search Box
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Search:", GUILayout.Width(60));
        searchText = EditorGUILayout.TextField(searchText);
        if (GUILayout.Button("Clear", GUILayout.Width(60)))
        {
            searchText = "";
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Select All / Deselect All
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Select All", GUILayout.Width(100)))
        {
            var filtered = GetFilteredIds();
            selectedIds.Clear();
            foreach (var id in filtered)
            {
                selectedIds.Add(id);
            }
        }
        if (GUILayout.Button("Deselect All", GUILayout.Width(100)))
        {
            selectedIds.Clear();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // ID List
        var filteredIds = GetFilteredIds();
        if (filteredIds.Count == 0)
        {
            EditorGUILayout.HelpBox(
                string.IsNullOrEmpty(searchText) 
                    ? "No IDs in registry." 
                    : "No IDs match the search filter.",
                MessageType.Info);
        }
        else
        {
            scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition, GUILayout.Height(300));
            
            foreach (var id in filteredIds)
            {
                EditorGUILayout.BeginHorizontal();
                
                bool isSelected = selectedIds.Contains(id);
                bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
                
                if (newSelected != isSelected)
                {
                    if (newSelected)
                        selectedIds.Add(id);
                    else
                        selectedIds.Remove(id);
                }
                
                EditorGUILayout.LabelField(id);
                
                EditorGUILayout.EndHorizontal();
            }
            
            EditorGUILayout.EndScrollView();
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawActions()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.LabelField("Actions", EditorStyles.boldLabel);
        
        EditorGUILayout.BeginHorizontal();
        
        // Delete Selected IDs
        EditorGUI.BeginDisabledGroup(selectedIds.Count == 0);
        if (GUILayout.Button($"Delete Selected ({selectedIds.Count})", GUILayout.Height(30)))
        {
            if (EditorUtility.DisplayDialog(
                "Delete IDs",
                $"Are you sure you want to delete {selectedIds.Count} ID(s) from the registry?",
                "Delete",
                "Cancel"))
            {
                DeleteSelectedIds();
            }
        }
        EditorGUI.EndDisabledGroup();
        
        // Clear Entire Registry
        EditorGUI.BeginDisabledGroup(!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded);
        if (GUILayout.Button("Clear Entire Registry", GUILayout.Height(30)))
        {
            if (EditorUtility.DisplayDialog(
                "Clear Registry",
                $"Are you sure you want to clear the entire {selectedType} registry? This cannot be undone.",
                "Clear",
                "Cancel"))
            {
                ClearRegistry(selectedType);
            }
        }
        EditorGUI.EndDisabledGroup();
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Export / Import
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Export Registry", GUILayout.Height(25)))
        {
            ExportRegistry();
        }
        if (GUILayout.Button("Import Registry", GUILayout.Height(25)))
        {
            ImportRegistry();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
    }
    
    private List<string> GetFilteredIds()
    {
        if (!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded)
            return new List<string>();
        
        var registry = registries[selectedType];
        
        if (string.IsNullOrEmpty(searchText))
            return registry.ids;
        
        return registry.ids.Where(id => 
            id.IndexOf(searchText, StringComparison.OrdinalIgnoreCase) >= 0).ToList();
    }
    
    private void LoadAllRegistries()
    {
        foreach (ItemType type in Enum.GetValues(typeof(ItemType)))
        {
            LoadRegistry(type);
        }
    }
    
    private void LoadRegistry(ItemType itemType)
    {
        string typeName = GetTypeName(itemType);
        string registryPath = GetRegistryPath(typeName);
        
        if (!registries.ContainsKey(itemType))
        {
            registries[itemType] = new RegistryData();
        }
        
        var registry = registries[itemType];
        registry.filePath = registryPath;
        registry.ids.Clear();
        registry.isLoaded = false;
        
        if (!File.Exists(registryPath))
        {
            registry.isLoaded = true;
            return;
        }
        
        try
        {
            string jsonContent = File.ReadAllText(registryPath);
            var jsonData = JsonUtility.FromJson<RegistryJson>(jsonContent);
            
            if (jsonData != null && jsonData.ids != null)
            {
                registry.ids = jsonData.ids.ToList();
                registry.lastModified = File.GetLastWriteTime(registryPath);
            }
            
            registry.isLoaded = true;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[RegistryManager] Failed to load registry: {ex.Message}");
            registry.isLoaded = false;
        }
    }
    
    private void DeleteSelectedIds()
    {
        if (!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded)
            return;
        
        var registry = registries[selectedType];
        int removedCount = 0;
        
        foreach (var id in selectedIds)
        {
            if (registry.ids.Remove(id))
                removedCount++;
        }
        
        SaveRegistry(selectedType);
        selectedIds.Clear();
        
        Debug.Log($"[RegistryManager] Removed {removedCount} ID(s) from {selectedType} registry.");
    }
    
    private void ClearRegistry(ItemType itemType)
    {
        if (!registries.ContainsKey(itemType))
            return;
        
        var registry = registries[itemType];
        registry.ids.Clear();
        SaveRegistry(itemType);
        selectedIds.Clear();
        
        Debug.Log($"[RegistryManager] Cleared {itemType} registry.");
    }
    
    private void SaveRegistry(ItemType itemType)
    {
        if (!registries.ContainsKey(itemType))
            return;
        
        var registry = registries[itemType];
        string directory = Path.GetDirectoryName(registry.filePath);
        
        if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
        {
            Directory.CreateDirectory(directory);
        }
        
        try
        {
            var jsonData = new RegistryJson { ids = registry.ids.ToArray() };
            string jsonContent = JsonUtility.ToJson(jsonData, true);
            File.WriteAllText(registry.filePath, jsonContent);
            
            registry.lastModified = DateTime.Now;
            Debug.Log($"[RegistryManager] Saved registry: {registry.filePath}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[RegistryManager] Failed to save registry: {ex.Message}");
        }
    }
    
    private void ExportRegistry()
    {
        if (!registries.ContainsKey(selectedType) || !registries[selectedType].isLoaded)
            return;
        
        string path = EditorUtility.SaveFilePanel(
            "Export Registry",
            "",
            $"registry_{GetTypeName(selectedType)}.json",
            "json");
        
        if (string.IsNullOrEmpty(path))
            return;
        
        try
        {
            var registry = registries[selectedType];
            var jsonData = new RegistryJson { ids = registry.ids.ToArray() };
            string jsonContent = JsonUtility.ToJson(jsonData, true);
            File.WriteAllText(path, jsonContent);
            
            Debug.Log($"[RegistryManager] Exported registry to: {path}");
            EditorUtility.DisplayDialog("Export", "Registry exported successfully!", "OK");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[RegistryManager] Export failed: {ex.Message}");
            EditorUtility.DisplayDialog("Export Failed", ex.Message, "OK");
        }
    }
    
    private void ImportRegistry()
    {
        string path = EditorUtility.OpenFilePanel(
            "Import Registry",
            "",
            "json");
        
        if (string.IsNullOrEmpty(path))
            return;
        
        try
        {
            string jsonContent = File.ReadAllText(path);
            var jsonData = JsonUtility.FromJson<RegistryJson>(jsonContent);
            
            if (jsonData == null || jsonData.ids == null)
            {
                EditorUtility.DisplayDialog("Import Failed", "Invalid registry file format.", "OK");
                return;
            }
            
            if (EditorUtility.DisplayDialog(
                "Import Registry",
                $"This will replace the current {selectedType} registry with {jsonData.ids.Length} ID(s). Continue?",
                "Import",
                "Cancel"))
            {
                if (!registries.ContainsKey(selectedType))
                {
                    registries[selectedType] = new RegistryData();
                }
                
                var registry = registries[selectedType];
                registry.ids = jsonData.ids.ToList();
                SaveRegistry(selectedType);
                selectedIds.Clear();
                
                Debug.Log($"[RegistryManager] Imported {jsonData.ids.Length} ID(s) to {selectedType} registry.");
                EditorUtility.DisplayDialog("Import", "Registry imported successfully!", "OK");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"[RegistryManager] Import failed: {ex.Message}");
            EditorUtility.DisplayDialog("Import Failed", ex.Message, "OK");
        }
    }
    
    private string GetRegistryPath(string typeName)
    {
        // Try to find the executable path to locate the Registry folder
        string[] possiblePaths = {
            Path.Combine(Application.dataPath, "..", "Deployment", "Registry", $"id_registry_{typeName}.json"),
            Path.Combine(Application.dataPath, "..", "RundeeItemFactory", "RundeeItemFactory", "x64", "Debug", "Registry", $"id_registry_{typeName}.json"),
            Path.Combine(Application.dataPath, "..", "RundeeItemFactory", "RundeeItemFactory", "x64", "Release", "Registry", $"id_registry_{typeName}.json"),
        };
        
        foreach (var path in possiblePaths)
        {
            if (File.Exists(path))
                return path;
        }
        
        // Return default path (will be created if needed)
        return Path.Combine(Application.dataPath, "..", "Deployment", "Registry", $"id_registry_{typeName}.json");
    }
    
    private string GetTypeName(ItemType itemType)
    {
        switch (itemType)
        {
            case ItemType.Food: return "food";
            case ItemType.Drink: return "drink";
            case ItemType.Medicine: return "medicine";
            case ItemType.Material: return "material";
            case ItemType.Weapon: return "weapon";
            case ItemType.WeaponComponent: return "weaponcomponent";
            case ItemType.Ammo: return "ammo";
            case ItemType.Armor: return "armor";
            case ItemType.Clothing: return "clothing";
            default: return "unknown";
        }
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
    
    [Serializable]
    private class RegistryJson
    {
        public string[] ids;
    }
}

