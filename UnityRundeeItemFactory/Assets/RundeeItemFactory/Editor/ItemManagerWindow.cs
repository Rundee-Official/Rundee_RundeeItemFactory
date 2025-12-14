using System;
using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

public class ItemManagerWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string searchText = "";
    private string rarityFilter = "All";
    private Vector2 scrollPosition;

    private readonly List<ItemRecord> _items = new();
    private readonly HashSet<string> _selectedPaths = new();

    private static readonly string[] RarityOptions = { "All", "Common", "Uncommon", "Rare", "Legendary" };

    private class ItemRecord
    {
        public string Id;
        public string DisplayName;
        public string Rarity;
        public string Description;
        public UnityEngine.Object Asset;
        public string AssetPath;
    }

    [MenuItem("Tools/Rundee/Item Factory/Item Manager")]
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
        using (new EditorGUILayout.HorizontalScope())
        {
            EditorGUI.BeginChangeCheck();
            selectedType = (ItemType)EditorGUILayout.EnumPopup("Item Type", selectedType);
            if (EditorGUI.EndChangeCheck())
            {
                RefreshItems();
            }

            if (GUILayout.Button("Refresh", GUILayout.Width(80)))
            {
                RefreshItems();
            }
        }

        using (new EditorGUILayout.HorizontalScope())
        {
            searchText = EditorGUILayout.TextField("Search", searchText);
            int rarityIndex = Array.IndexOf(RarityOptions, rarityFilter);
            if (rarityIndex < 0) rarityIndex = 0;
            int newIndex = EditorGUILayout.Popup("Rarity", rarityIndex, RarityOptions);
            rarityFilter = RarityOptions[Mathf.Clamp(newIndex, 0, RarityOptions.Length - 1)];
        }
    }

    private void DrawSummary()
    {
        EditorGUILayout.HelpBox(
            $"{_items.Count} assets found in {GetFolderForType(selectedType)}. " +
            $"{_selectedPaths.Count} selected.",
            MessageType.Info);
    }

    private void DrawItemList()
    {
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);

        foreach (var record in _items)
        {
            if (!PassesFilter(record))
                continue;

            using (new EditorGUILayout.VerticalScope("box"))
            {
                bool isSelected = _selectedPaths.Contains(record.AssetPath);
                bool newSelected = EditorGUILayout.ToggleLeft(
                    $"{record.DisplayName} ({record.Id})",
                    isSelected);

                if (newSelected != isSelected)
                {
                    if (newSelected)
                        _selectedPaths.Add(record.AssetPath);
                    else
                        _selectedPaths.Remove(record.AssetPath);
                }

                EditorGUILayout.LabelField("Rarity", record.Rarity);
                EditorGUILayout.LabelField("Description",
                    string.IsNullOrEmpty(record.Description)
                        ? "(no description)"
                        : record.Description, EditorStyles.wordWrappedLabel);
                EditorGUILayout.LabelField("Asset Path", record.AssetPath);

                using (new EditorGUILayout.HorizontalScope())
                {
                    GUILayout.FlexibleSpace();
                    if (GUILayout.Button("Select", GUILayout.Width(70)))
                    {
                        EditorGUIUtility.PingObject(record.Asset);
                        Selection.activeObject = record.Asset;
                    }
                    if (GUILayout.Button("Delete", GUILayout.Width(70)))
                    {
                        DeleteAsset(record);
                    }
                }
            }
        }

        EditorGUILayout.EndScrollView();
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

        _items.Sort((a, b) => string.Compare(a.DisplayName, b.DisplayName, StringComparison.OrdinalIgnoreCase));
    }

    private ItemRecord CreateRecordFromAsset(UnityEngine.Object asset, string path)
    {
        switch (selectedType)
        {
            case ItemType.Food:
                return CreateRecord(asset as FoodItemDataSO, path, so => so.rarity);
            case ItemType.Drink:
                return CreateRecord(asset as DrinkItemDataSO, path, so => so.rarity);
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
            ItemType.Material => typeof(MaterialItemDataSO),
            ItemType.Weapon => typeof(WeaponItemDataSO),
            ItemType.WeaponComponent => typeof(WeaponComponentItemDataSO),
            ItemType.Ammo => typeof(AmmoItemDataSO),
            _ => null
        };
    }
}

