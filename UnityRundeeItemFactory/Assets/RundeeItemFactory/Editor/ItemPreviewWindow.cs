using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Unity Editor window for previewing and editing generated items before import
/// </summary>
/// <remarks>
/// Provides functionality to:
/// - Preview items from JSON files
/// - Edit item properties
/// - Filter and search items
/// - Export edited items back to JSON
/// 
/// This can be extended for Unreal Engine using Editor Utility Widgets.
/// </remarks>
public class ItemPreviewWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string jsonFilePath = "";
    private string searchText = "";
    private string rarityFilter = "All";
    private Vector2 scrollPosition;
    private Vector2 itemScrollPosition;
    
    private List<ItemPreviewData> items = new List<ItemPreviewData>();
    private ItemPreviewData selectedItem = null;
    private bool showItemDetails = false;
    
    private class ItemPreviewData
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
        public Dictionary<string, object> properties = new Dictionary<string, object>();
        public int originalIndex;
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Tools/Item Preview", false, 3000)]
    public static void ShowWindow()
    {
        var window = GetWindow<ItemPreviewWindow>("Item Preview");
        window.minSize = new Vector2(800, 600);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        
        // Header
        EditorGUILayout.LabelField("Item Preview & Editor", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Preview and edit items from JSON files before importing into Unity.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // File Selection
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("JSON File:", GUILayout.Width(80));
        EditorGUILayout.TextField(jsonFilePath, GUILayout.ExpandWidth(true));
        if (GUILayout.Button("Browse", GUILayout.Width(80)))
        {
            string path = EditorUtility.OpenFilePanel("Select JSON File", "", "json");
            if (!string.IsNullOrEmpty(path))
            {
                jsonFilePath = path;
                LoadItems();
            }
        }
        if (GUILayout.Button("Reload", GUILayout.Width(80)))
        {
            LoadItems();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        if (items.Count == 0)
        {
            EditorGUILayout.HelpBox("No items loaded. Select a JSON file to preview items.", MessageType.Info);
            return;
        }
        
        // Filters
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Item Type:", GUILayout.Width(80));
        selectedType = (ItemType)EditorGUILayout.EnumPopup(selectedType);
        
        EditorGUILayout.LabelField("Rarity:", GUILayout.Width(50));
        string[] rarityOptions = { "All", "Common", "Uncommon", "Rare" };
        int rarityIndex = Array.IndexOf(rarityOptions, rarityFilter);
        rarityIndex = EditorGUILayout.Popup(rarityIndex, rarityOptions, GUILayout.Width(100));
        rarityFilter = rarityOptions[rarityIndex];
        
        EditorGUILayout.LabelField("Search:", GUILayout.Width(50));
        searchText = EditorGUILayout.TextField(searchText, GUILayout.Width(200));
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Split View
        EditorGUILayout.BeginHorizontal();
        
        // Item List (Left Panel)
        EditorGUILayout.BeginVertical(EditorStyles.helpBox, GUILayout.Width(300));
        EditorGUILayout.LabelField($"Items ({GetFilteredItems().Count})", EditorStyles.boldLabel);
        
        itemScrollPosition = EditorGUILayout.BeginScrollView(itemScrollPosition);
        
        foreach (var item in GetFilteredItems())
        {
            bool isSelected = selectedItem == item;
            bool newSelected = EditorGUILayout.ToggleLeft(
                $"{item.displayName} ({item.rarity})",
                isSelected);
            
            if (newSelected != isSelected)
            {
                selectedItem = newSelected ? item : null;
                showItemDetails = newSelected;
            }
        }
        
        EditorGUILayout.EndScrollView();
        EditorGUILayout.EndVertical();
        
        // Item Details (Right Panel)
        EditorGUILayout.BeginVertical(EditorStyles.helpBox, GUILayout.ExpandWidth(true));
        
        if (selectedItem != null && showItemDetails)
        {
            DrawItemDetails(selectedItem);
        }
        else
        {
            EditorGUILayout.HelpBox("Select an item to view details", MessageType.Info);
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Actions
        EditorGUILayout.BeginHorizontal();
        EditorGUI.BeginDisabledGroup(items.Count == 0);
        if (GUILayout.Button("Export to JSON", GUILayout.Height(30)))
        {
            ExportToJSON();
        }
        if (GUILayout.Button("Import to Unity", GUILayout.Height(30)))
        {
            ImportToUnity();
        }
        EditorGUI.EndDisabledGroup();
        EditorGUILayout.EndHorizontal();
    }
    
    private void DrawItemDetails(ItemPreviewData item)
    {
        EditorGUILayout.LabelField("Item Details", EditorStyles.boldLabel);
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("ID:", item.id);
        EditorGUILayout.LabelField("Display Name:", item.displayName);
        EditorGUILayout.LabelField("Category:", item.category);
        EditorGUILayout.LabelField("Rarity:", item.rarity);
        
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Description:", EditorStyles.boldLabel);
        EditorGUILayout.TextArea(item.description, GUILayout.Height(60));
        
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Properties:", EditorStyles.boldLabel);
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        foreach (var prop in item.properties)
        {
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField(prop.Key + ":", GUILayout.Width(150));
            EditorGUILayout.LabelField(prop.Value?.ToString() ?? "null");
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private List<ItemPreviewData> GetFilteredItems()
    {
        var filtered = items.Where(item => 
            (string.IsNullOrEmpty(searchText) || 
             item.id.IndexOf(searchText, StringComparison.OrdinalIgnoreCase) >= 0 ||
             item.displayName.IndexOf(searchText, StringComparison.OrdinalIgnoreCase) >= 0) &&
            (rarityFilter == "All" || item.rarity == rarityFilter)
        ).ToList();
        
        return filtered;
    }
    
    private void LoadItems()
    {
        items.Clear();
        selectedItem = null;
        
        if (string.IsNullOrEmpty(jsonFilePath) || !File.Exists(jsonFilePath))
        {
            Debug.LogWarning("[ItemPreview] JSON file not found or not selected.");
            return;
        }
        
        try
        {
            string jsonContent = File.ReadAllText(jsonFilePath);
            
            // Unity's JsonUtility cannot parse arrays directly, so we need to wrap it
            // Check if content starts with '[' (array) or '{' (object)
            string jsonToParse = jsonContent.Trim();
            if (jsonToParse.StartsWith("["))
            {
                // It's an array, wrap it in an object
                jsonToParse = "{ \"items\": " + jsonContent + " }";
            }
            else if (!jsonToParse.StartsWith("{"))
            {
                Debug.LogError("[ItemPreview] Invalid JSON format. Expected array or object.");
                return;
            }
            
            var jsonData = JsonUtility.FromJson<ItemListWrapper>(jsonToParse);
            
            if (jsonData != null && jsonData.items != null)
            {
                for (int i = 0; i < jsonData.items.Length; i++)
                {
                    var itemDto = jsonData.items[i];
                    var item = new ItemPreviewData
                    {
                        id = itemDto.id ?? "",
                        displayName = itemDto.displayName ?? "",
                        category = itemDto.category ?? "",
                        rarity = itemDto.rarity ?? "",
                        description = itemDto.description ?? "",
                        originalIndex = i
                    };
                    
                    // Add all properties
                    item.properties["maxStack"] = itemDto.maxStack;
                    
                    // Type-specific properties would be added here
                    // For now, just basic properties
                    
                    items.Add(item);
                }
                
                Debug.Log($"[ItemPreview] Loaded {items.Count} items from {jsonFilePath}");
            }
            else
            {
                Debug.LogError("[ItemPreview] Failed to parse JSON. jsonData is null or items is null.");
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"[ItemPreview] Failed to load items: {ex.Message}\n{ex.StackTrace}");
        }
    }
    
    private void ExportToJSON()
    {
        string path = EditorUtility.SaveFilePanel(
            "Export Items",
            Path.GetDirectoryName(jsonFilePath),
            Path.GetFileNameWithoutExtension(jsonFilePath) + "_edited.json",
            "json");
        
        if (string.IsNullOrEmpty(path))
            return;
        
        try
        {
            // Reconstruct JSON from items
            var wrapper = new ItemListWrapper { items = new ItemDTO[items.Count] };
            for (int i = 0; i < items.Count; i++)
            {
                wrapper.items[i] = new ItemDTO
                {
                    id = items[i].id,
                    displayName = items[i].displayName,
                    category = items[i].category,
                    rarity = items[i].rarity,
                    description = items[i].description
                };
            }
            
            string jsonContent = JsonUtility.ToJson(wrapper, true);
            File.WriteAllText(path, jsonContent);
            
            Debug.Log($"[ItemPreview] Exported {items.Count} items to {path}");
            EditorUtility.DisplayDialog("Export", "Items exported successfully!", "OK");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[ItemPreview] Export failed: {ex.Message}");
            EditorUtility.DisplayDialog("Export Failed", ex.Message, "OK");
        }
    }
    
    private void ImportToUnity()
    {
        if (string.IsNullOrEmpty(jsonFilePath))
        {
            EditorUtility.DisplayDialog("Import Failed", "No JSON file selected.", "OK");
            return;
        }
        
        // Use existing ItemImporter
        switch (selectedType)
        {
            case ItemType.Food:
                ItemImporter.ImportFoodFromJsonPath(jsonFilePath);
                break;
            case ItemType.Drink:
                ItemImporter.ImportDrinkFromJsonPath(jsonFilePath);
                break;
            case ItemType.Medicine:
                ItemImporter.ImportMedicineFromJsonPath(jsonFilePath);
                break;
            case ItemType.Material:
                ItemImporter.ImportMaterialFromJsonPath(jsonFilePath);
                break;
            case ItemType.Weapon:
                ItemImporter.ImportWeaponFromJsonPath(jsonFilePath);
                break;
            case ItemType.WeaponComponent:
                ItemImporter.ImportWeaponComponentFromJsonPath(jsonFilePath);
                break;
            case ItemType.Ammo:
                ItemImporter.ImportAmmoFromJsonPath(jsonFilePath);
                break;
            default:
                EditorUtility.DisplayDialog("Import Failed", $"Import not supported for {selectedType}", "OK");
                return;
        }
        
        EditorUtility.DisplayDialog("Import", "Items imported successfully!", "OK");
    }
    
    [Serializable]
    private class ItemListWrapper
    {
        public ItemDTO[] items;
    }
    
    [Serializable]
    private class ItemDTO
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
        public int maxStack;
        // Add other properties as needed
    }
}

