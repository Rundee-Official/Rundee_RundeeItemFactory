using System;
using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Unity Editor window for managing imported item ScriptableObjects
/// </summary>
/// <remarks>
/// Provides functionality to:
/// - Browse items by type (Food, Drink, Material, Weapon, WeaponComponent, Ammo)
/// - Filter items by rarity (Common, Uncommon, Rare)
/// - Search items by name or ID
/// - Sort items by name, rarity, or ID
/// - View item statistics and rarity distribution
/// - Select and delete items
/// </remarks>
public class ItemManagerWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string searchText = "";
    private string rarityFilter = "All";
    private Vector2 scrollPosition;
    private SortMode sortMode = SortMode.Name;
    private bool sortAscending = true;

    private readonly List<ItemRecord> _items = new();
    private readonly HashSet<string> _selectedPaths = new();

    private static readonly string[] RarityOptions = { "All", "Common", "Uncommon", "Rare" };

    private enum SortMode
    {
        Name,
        Rarity,
        ID
    }

    private class ItemRecord
    {
        public string Id;
        public string DisplayName;
        public string Rarity;
        public string Description;
        public UnityEngine.Object Asset;
        public string AssetPath;
    }

    [MenuItem("Tools/Rundee/Item Factory/Management/Item Manager", false, 2000)]
    public static void ShowWindow()
    {
        var window = GetWindow<ItemManagerWindow>("Item Manager");
        window.minSize = new Vector2(500, 400);
        window.RefreshItems();
    }

    private void OnEnable()
    {
        RefreshItems();
    }

    private void OnGUI()
    {
        DrawToolbar();
        DrawSummary();
        DrawItemList();
        DrawFooter();
    }

    private void DrawToolbar()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        using (new EditorGUILayout.HorizontalScope())
        {
            EditorGUI.BeginChangeCheck();
            EditorGUILayout.LabelField("Item Type:", GUILayout.Width(80));
            selectedType = (ItemType)EditorGUILayout.EnumPopup(selectedType, GUILayout.Width(150));
            if (EditorGUI.EndChangeCheck())
            {
                RefreshItems();
            }

            GUILayout.FlexibleSpace();
            
            if (GUILayout.Button("Refresh", GUILayout.Width(80)))
            {
                RefreshItems();
            }
        }

        EditorGUILayout.Space(3);

        using (new EditorGUILayout.HorizontalScope())
        {
            EditorGUILayout.LabelField("Search:", GUILayout.Width(80));
            searchText = EditorGUILayout.TextField(searchText);
            
            EditorGUILayout.LabelField("Rarity:", GUILayout.Width(60));
            int rarityIndex = Array.IndexOf(RarityOptions, rarityFilter);
            if (rarityIndex < 0) rarityIndex = 0;
            int newIndex = EditorGUILayout.Popup(rarityIndex, RarityOptions, GUILayout.Width(100));
            rarityFilter = RarityOptions[Mathf.Clamp(newIndex, 0, RarityOptions.Length - 1)];
        }

        EditorGUILayout.Space(3);

        using (new EditorGUILayout.HorizontalScope())
        {
            EditorGUILayout.LabelField("Sort By:", GUILayout.Width(80));
            sortMode = (SortMode)EditorGUILayout.EnumPopup(sortMode, GUILayout.Width(100));
            sortAscending = EditorGUILayout.Toggle("Ascending", sortAscending, GUILayout.Width(100));
        }

        EditorGUILayout.EndVertical();
    }

    private void DrawSummary()
    {
        var filteredItems = _items.Where(PassesFilter).ToList();
        var rarityStats = filteredItems.GroupBy(r => r.Rarity)
            .ToDictionary(g => g.Key, g => g.Count());

        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        // Title
        GUIStyle titleStyle = new GUIStyle(EditorStyles.boldLabel);
        titleStyle.fontSize = 14;
        EditorGUILayout.LabelField($"Item Manager - {selectedType}", titleStyle);
        EditorGUILayout.Space(5);

        // Statistics
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField($"Total Items:", GUILayout.Width(100));
        EditorGUILayout.LabelField($"{_items.Count}", EditorStyles.boldLabel);
        EditorGUILayout.LabelField($"Filtered:", GUILayout.Width(80));
        EditorGUILayout.LabelField($"{filteredItems.Count}", EditorStyles.boldLabel);
        EditorGUILayout.LabelField($"Selected:", GUILayout.Width(80));
        EditorGUILayout.LabelField($"{_selectedPaths.Count}", EditorStyles.boldLabel);
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.Space(3);

        // Rarity Distribution
        EditorGUILayout.LabelField("Rarity Distribution:", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        foreach (var rarity in new[] { "Common", "Uncommon", "Rare" })
        {
            int count = rarityStats.ContainsKey(rarity) ? rarityStats[rarity] : 0;
            float percentage = filteredItems.Count > 0 ? (count * 100f / filteredItems.Count) : 0f;
            Color rarityColor = GetRarityColor(rarity);
            
            var oldColor = GUI.color;
            GUI.color = rarityColor;
            EditorGUILayout.LabelField($"{rarity}: {count} ({percentage:F1}%)", EditorStyles.miniLabel, GUILayout.Width(140));
            GUI.color = oldColor;
        }
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.EndVertical();
    }

    private Color GetRarityColor(string rarity)
    {
        switch (rarity)
        {
            case "Common":
                return new Color(0.7f, 0.7f, 0.7f); // Gray
            case "Uncommon":
                return new Color(0.2f, 0.8f, 0.2f); // Green
            case "Rare":
                return new Color(1f, 0.84f, 0f); // Gold
            default:
                return Color.white;
        }
    }

    private void DrawItemList()
    {
        var filteredItems = _items.Where(PassesFilter).ToList();
        SortItems(filteredItems);

        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);

        if (filteredItems.Count == 0)
        {
            EditorGUILayout.HelpBox("No items found matching the current filters.", MessageType.Info);
            EditorGUILayout.EndScrollView();
            return;
        }

        foreach (var record in filteredItems)
        {
            DrawItemCard(record);
        }

        EditorGUILayout.EndScrollView();
    }

    private void DrawItemCard(ItemRecord record)
    {
        bool isSelected = _selectedPaths.Contains(record.AssetPath);
        Color rarityColor = GetRarityColor(record.Rarity);

        // Card background
        var oldBgColor = GUI.backgroundColor;
        if (isSelected)
        {
            GUI.backgroundColor = new Color(0.3f, 0.5f, 0.8f, 0.3f);
        }

        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        GUI.backgroundColor = oldBgColor;

        // Header row with checkbox, name, and rarity badge
        EditorGUILayout.BeginHorizontal();
        
        bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
        if (newSelected != isSelected)
        {
            if (newSelected)
                _selectedPaths.Add(record.AssetPath);
            else
                _selectedPaths.Remove(record.AssetPath);
        }

        // Item name
        GUIStyle nameStyle = new GUIStyle(EditorStyles.boldLabel);
        nameStyle.fontSize = 12;
        EditorGUILayout.LabelField(record.DisplayName, nameStyle, GUILayout.ExpandWidth(true));

        // Rarity badge
        var oldColor = GUI.color;
        GUI.color = rarityColor;
        GUIStyle badgeStyle = new GUIStyle(EditorStyles.miniButton);
        badgeStyle.normal.textColor = Color.white;
        badgeStyle.fontStyle = FontStyle.Bold;
        EditorGUILayout.LabelField(record.Rarity, badgeStyle, GUILayout.Width(80));
        GUI.color = oldColor;

        EditorGUILayout.EndHorizontal();

        // ID
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("ID:", GUILayout.Width(30));
        EditorGUILayout.LabelField(record.Id, EditorStyles.miniLabel);
        EditorGUILayout.EndHorizontal();

        // Description
        if (!string.IsNullOrEmpty(record.Description))
        {
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Description:", GUILayout.Width(80));
            EditorGUILayout.LabelField(record.Description, EditorStyles.wordWrappedMiniLabel);
            EditorGUILayout.EndHorizontal();
        }

        // Action buttons
        EditorGUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Select", GUILayout.Width(70)))
        {
            EditorGUIUtility.PingObject(record.Asset);
            Selection.activeObject = record.Asset;
        }
        
        var oldButtonColor = GUI.backgroundColor;
        GUI.backgroundColor = new Color(0.8f, 0.3f, 0.3f);
        if (GUILayout.Button("Delete", GUILayout.Width(70)))
        {
            DeleteAsset(record);
        }
        GUI.backgroundColor = oldButtonColor;
        
        EditorGUILayout.EndHorizontal();

        EditorGUILayout.EndVertical();
    }

    private void SortItems(List<ItemRecord> items)
    {
        switch (sortMode)
        {
            case SortMode.Name:
                items.Sort((a, b) => sortAscending 
                    ? string.Compare(a.DisplayName, b.DisplayName, StringComparison.OrdinalIgnoreCase)
                    : string.Compare(b.DisplayName, a.DisplayName, StringComparison.OrdinalIgnoreCase));
                break;
            case SortMode.Rarity:
                var rarityOrder = new Dictionary<string, int> { { "Common", 1 }, { "Uncommon", 2 }, { "Rare", 3 } };
                items.Sort((a, b) =>
                {
                    int aOrder = rarityOrder.ContainsKey(a.Rarity) ? rarityOrder[a.Rarity] : 0;
                    int bOrder = rarityOrder.ContainsKey(b.Rarity) ? rarityOrder[b.Rarity] : 0;
                    return sortAscending ? aOrder.CompareTo(bOrder) : bOrder.CompareTo(aOrder);
                });
                break;
            case SortMode.ID:
                items.Sort((a, b) => sortAscending
                    ? string.Compare(a.Id, b.Id, StringComparison.OrdinalIgnoreCase)
                    : string.Compare(b.Id, a.Id, StringComparison.OrdinalIgnoreCase));
                break;
        }
    }

    private void DrawFooter()
    {
        using (new EditorGUILayout.HorizontalScope())
        {
            GUILayout.FlexibleSpace();
            EditorGUI.BeginDisabledGroup(_selectedPaths.Count == 0);
            if (GUILayout.Button($"Delete Selected ({_selectedPaths.Count})", GUILayout.Width(180)))
            {
                DeleteSelectedAssets();
            }
            EditorGUI.EndDisabledGroup();
        }
    }

    private bool PassesFilter(ItemRecord record)
    {
        if (!string.IsNullOrEmpty(searchText))
        {
            if (!(record.DisplayName?.IndexOf(searchText, StringComparison.OrdinalIgnoreCase) >= 0
                  || record.Id?.IndexOf(searchText, StringComparison.OrdinalIgnoreCase) >= 0))
            {
                return false;
            }
        }

        if (rarityFilter != "All" &&
            !string.Equals(record.Rarity, rarityFilter, StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        return true;
    }

    private void RefreshItems()
    {
        _items.Clear();
        _selectedPaths.Clear();

        string folder = GetFolderForType(selectedType);
        if (!AssetDatabase.IsValidFolder(folder))
        {
            Debug.LogWarning($"Folder not found: {folder}");
            return;
        }

        Type assetType = GetAssetType(selectedType);
        if (assetType == null)
        {
            Debug.LogError($"Unsupported item type: {selectedType}");
            return;
        }

        string[] guids = AssetDatabase.FindAssets($"t:{assetType.Name}", new[] { folder });
        foreach (var guid in guids)
        {
            string path = AssetDatabase.GUIDToAssetPath(guid);
            var asset = AssetDatabase.LoadAssetAtPath<UnityEngine.Object>(path);
            if (asset == null)
                continue;

            var record = CreateRecordFromAsset(asset, path);
            if (record != null)
            {
                _items.Add(record);
            }
        }

        // Sorting will be done in DrawItemList based on sortMode
    }

    private ItemRecord CreateRecordFromAsset(UnityEngine.Object asset, string path)
    {
        switch (selectedType)
        {
            case ItemType.Food:
                return CreateRecord(asset as FoodItemDataSO, path, so => so.rarity);
            case ItemType.Drink:
                return CreateRecord(asset as DrinkItemDataSO, path, so => so.rarity);
            case ItemType.Medicine:
                return CreateRecord(asset as MedicineItemDataSO, path, so => so.rarity);
            case ItemType.Material:
                return CreateRecord(asset as MaterialItemDataSO, path, so => so.rarity);
            case ItemType.Weapon:
                return CreateRecord(asset as WeaponItemDataSO, path, so => so.rarity);
            case ItemType.WeaponComponent:
                return CreateRecord(asset as WeaponComponentItemDataSO, path, so => so.rarity);
            case ItemType.Ammo:
                return CreateRecord(asset as AmmoItemDataSO, path, so => so.rarity);
            default:
                return null;
        }
    }

    private ItemRecord CreateRecord<T>(T asset, string path, Func<T, string> raritySelector) where T : ScriptableObject
    {
        if (asset == null)
            return null;

        string id = asset.GetType().GetField("id")?.GetValue(asset) as string ?? "(no id)";
        string displayName = asset.GetType().GetField("displayName")?.GetValue(asset) as string ?? asset.name;
        string rarity = raritySelector(asset) ?? "Unknown";
        string description = asset.GetType().GetField("description")?.GetValue(asset) as string ?? "";

        return new ItemRecord
        {
            Id = id,
            DisplayName = displayName,
            Rarity = rarity,
            Description = description,
            Asset = asset,
            AssetPath = path
        };
    }

    private void DeleteAsset(ItemRecord record)
    {
        if (!EditorUtility.DisplayDialog("Delete Item",
                $"Delete asset '{record.DisplayName}'?\n{record.AssetPath}", "Delete", "Cancel"))
        {
            return;
        }

        AssetDatabase.DeleteAsset(record.AssetPath);
        AssetDatabase.SaveAssets();
        RefreshItems();
    }

    private void DeleteSelectedAssets()
    {
        if (!EditorUtility.DisplayDialog("Delete Selected Items",
                $"Delete {_selectedPaths.Count} selected assets?", "Delete", "Cancel"))
        {
            return;
        }

        foreach (var path in _selectedPaths.ToList())
        {
            AssetDatabase.DeleteAsset(path);
        }

        AssetDatabase.SaveAssets();
        RefreshItems();
    }

    private static string GetFolderForType(ItemType type)
    {
        return type switch
        {
            ItemType.Food => "Assets/Resources/RundeeItemFactory/FoodItems",
            ItemType.Drink => "Assets/Resources/RundeeItemFactory/DrinkItems",
            ItemType.Medicine => "Assets/Resources/RundeeItemFactory/MedicineItems",
            ItemType.Material => "Assets/Resources/RundeeItemFactory/MaterialItems",
            ItemType.Weapon => "Assets/Resources/RundeeItemFactory/WeaponItems",
            ItemType.WeaponComponent => "Assets/Resources/RundeeItemFactory/WeaponComponentItems",
            ItemType.Ammo => "Assets/Resources/RundeeItemFactory/AmmoItems",
            _ => "Assets/Resources/RundeeItemFactory"
        };
    }

    private static Type GetAssetType(ItemType type)
    {
        return type switch
        {
            ItemType.Food => typeof(FoodItemDataSO),
            ItemType.Drink => typeof(DrinkItemDataSO),
            ItemType.Medicine => typeof(MedicineItemDataSO),
            ItemType.Material => typeof(MaterialItemDataSO),
            ItemType.Weapon => typeof(WeaponItemDataSO),
            ItemType.WeaponComponent => typeof(WeaponComponentItemDataSO),
            ItemType.Ammo => typeof(AmmoItemDataSO),
            _ => null
        };
    }
}

