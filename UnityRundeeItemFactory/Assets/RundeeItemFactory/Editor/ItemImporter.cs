// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemImporter.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Imports JSON item data files and converts them to Unity ScriptableObjects.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Standard Library Includes
using System;
using System.IO;

// Unity Includes
using UnityEditor;
using UnityEngine;

// ============================================================================
// SECTION 1: Data Transfer Objects (DTOs) for JSON Deserialization
// ============================================================================

[Serializable]
public class FoodDrinkItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;

    public int hungerRestore;
    public int thirstRestore;
    public int healthRestore;

    public bool spoils;
    public int spoilTimeMinutes;

    public string description;
}

[Serializable]
public class MaterialItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;

    public string materialType;
    public int hardness;
    public int flammability;
    public int value;

    public string description;
}

[Serializable]
public class FoodDrinkItemListWrapper
{
    public FoodDrinkItemDataDTO[] items;
}

[Serializable]
public class MaterialItemListWrapper
{
    public MaterialItemDataDTO[] items;
}

[Serializable]
public class WeaponItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;
    public string weaponType;
    public string caliber;
    public int minDamage;
    public int maxDamage;
    public int fireRate;
    public int accuracy;
    public int recoil;
    public int ergonomics;
    public int weight;
    public int durability;
    public int muzzleVelocity;
    public int effectiveRange;
    public int penetrationPower;
    public int moddingSlots;
    public WeaponAttachmentSlotDTO[] attachmentSlots;
    public string description;
}

[Serializable]
public class WeaponAttachmentSlotDTO
{
    public string slotType;
    public int slotIndex;
    public bool isRequired;
}

[Serializable]
public class WeaponComponentItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;
    public string componentType;
    public int magazineCapacity;
    public string caliber;
    public string magazineType;
    public LoadedRoundSegmentDTO[] loadedRounds;
    public string[] compatibleSlots;
    public ComponentSubSlotDTO[] subSlots;
    public int damageModifier;
    public int recoilModifier;
    public int ergonomicsModifier;
    public int accuracyModifier;
    public int weightModifier;
    public int muzzleVelocityModifier;
    public int effectiveRangeModifier;
    public int penetrationModifier;
    public bool hasBuiltInRail;
    public string railType;
    public string description;
}

[Serializable]
public class ComponentSubSlotDTO
{
    public string slotType;
    public int slotIndex;
    public bool hasBuiltInRail;
}

[Serializable]
public class LoadedRoundSegmentDTO
{
    public int orderIndex;
    public int roundCount;
    public string ammoId;
    public string ammoDisplayName;
    public string ammoNotes;
}

[Serializable]
public class AmmoItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;
    public string caliber;
    public int damageBonus;
    public int penetration;
    public int accuracyBonus;
    public int recoilModifier;
    public bool armorPiercing;
    public bool hollowPoint;
    public bool tracer;
    public bool incendiary;
    public int value;
    public string description;
}

[Serializable]
public class WeaponItemListWrapper
{
    public WeaponItemDataDTO[] items;
}

[Serializable]
public class WeaponComponentItemListWrapper
{
    public WeaponComponentItemDataDTO[] items;
}

[Serializable]
public class AmmoItemListWrapper
{
    public AmmoItemDataDTO[] items;
}

[Serializable]
public class ArmorItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;
    public string armorType;
    public int armorClass;
    public int durability;
    public int material;
    public string protectionZones;
    public int movementSpeedPenalty;
    public int ergonomicsPenalty;
    public int turnSpeedPenalty;
    public int weight;
    public int capacity;
    public bool blocksHeadset;
    public bool blocksFaceCover;
    public string description;
}

[Serializable]
public class ClothingItemDataDTO
{
    public string id;
    public string displayName;
    public string category;
    public string rarity;
    public int maxStack;
    public string clothingType;
    public int coldResistance;
    public int heatResistance;
    public int waterResistance;
    public int windResistance;
    public int comfort;
    public int mobilityBonus;
    public int staminaBonus;
    public int durability;
    public int material;
    public int weight;
    public bool isInsulated;
    public bool isWaterproof;
    public bool isWindproof;
    public string description;
}

[Serializable]
public class ArmorItemListWrapper
{
    public ArmorItemDataDTO[] items;
}

[Serializable]
public class ClothingItemListWrapper
{
    public ClothingItemDataDTO[] items;
}

public enum ItemType
{
    Food,
    Drink,
    Material,
    Weapon,
    WeaponComponent,
    Ammo,
    Armor,
    Clothing
}

// ============================================================================
// SECTION 2: ItemImporter Class
// ============================================================================

public static class ItemImporter
{
    // Direct import methods for use by ItemFactoryWindow
    public static void ImportFoodFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportFoodItems(jsonText);
    }

    public static void ImportDrinkFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportDrinkItems(jsonText);
    }

    public static void ImportMaterialFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportMaterialItems(jsonText);
    }

    public static void ImportWeaponFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportWeaponItems(jsonText);
    }

    public static void ImportWeaponComponentFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportWeaponComponentItems(jsonText);
    }

    public static void ImportAmmoFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportAmmoItems(jsonText);
    }

    public static void ImportArmorFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportArmorItems(jsonText);
    }

    public static void ImportClothingFromJsonPath(string jsonPath)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[ItemImporter] File not found: {jsonPath}");
            return;
        }
        string jsonText = File.ReadAllText(jsonPath);
        ImportClothingItems(jsonText);
    }

    private static void ImportFoodItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        FoodDrinkItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<FoodDrinkItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/FoodItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            FoodItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<FoodItemDataSO>(assetPath);
            
            // If asset already exists with same ID, skip it (keep existing data)
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing food item: {dto.id} (already exists)");
                continue;
            }

            // Create new asset only if it doesn't exist
            FoodItemDataSO asset = ScriptableObject.CreateInstance<FoodItemDataSO>();

            // Copy fields
            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.hungerRestore = dto.hungerRestore;
            asset.thirstRestore = dto.thirstRestore;
            asset.healthRestore = dto.healthRestore;
            asset.spoils = dto.spoils;
            asset.spoilTimeMinutes = dto.spoilTimeMinutes;
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Food items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportDrinkItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        FoodDrinkItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<FoodDrinkItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/DrinkItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            DrinkItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<DrinkItemDataSO>(assetPath);
            
            // If asset already exists with same ID, skip it (keep existing data)
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing drink item: {dto.id} (already exists)");
                continue;
            }

            // Create new asset only if it doesn't exist
            DrinkItemDataSO asset = ScriptableObject.CreateInstance<DrinkItemDataSO>();

            // Copy fields
            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.hungerRestore = dto.hungerRestore;
            asset.thirstRestore = dto.thirstRestore;
            asset.healthRestore = dto.healthRestore;
            asset.spoils = dto.spoils;
            asset.spoilTimeMinutes = dto.spoilTimeMinutes;
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Drink items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportMaterialItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        MaterialItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<MaterialItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/MaterialItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            MaterialItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<MaterialItemDataSO>(assetPath);
            
            // If asset already exists with same ID, skip it (keep existing data)
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing material item: {dto.id} (already exists)");
                continue;
            }

            // Create new asset only if it doesn't exist
            MaterialItemDataSO asset = ScriptableObject.CreateInstance<MaterialItemDataSO>();

            // Copy fields
            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.materialType = dto.materialType;
            asset.hardness = Mathf.Clamp(dto.hardness, 0, 100);
            asset.flammability = Mathf.Clamp(dto.flammability, 0, 100);
            asset.value = Mathf.Clamp(dto.value, 0, 100);
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Material items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportWeaponItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        WeaponItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<WeaponItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/WeaponItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            WeaponItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<WeaponItemDataSO>(assetPath);
            
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing weapon item: {dto.id} (already exists)");
                continue;
            }

            WeaponItemDataSO asset = ScriptableObject.CreateInstance<WeaponItemDataSO>();

            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.weaponType = dto.weaponType;
            asset.caliber = dto.caliber;
            asset.minDamage = Mathf.Clamp(dto.minDamage, 0, 100);
            asset.maxDamage = Mathf.Clamp(dto.maxDamage, 0, 100);
            asset.fireRate = Mathf.Clamp(dto.fireRate, 0, 1200);
            asset.accuracy = Mathf.Clamp(dto.accuracy, 0, 100);
            asset.recoil = Mathf.Clamp(dto.recoil, 0, 100);
            asset.ergonomics = Mathf.Clamp(dto.ergonomics, 0, 100);
            asset.weight = dto.weight;
            asset.durability = Mathf.Clamp(dto.durability, 0, 100);
            asset.muzzleVelocity = Mathf.Clamp(dto.muzzleVelocity, 0, 1500);
            asset.effectiveRange = Mathf.Clamp(dto.effectiveRange, 0, 1000);
            asset.penetrationPower = Mathf.Clamp(dto.penetrationPower, 0, 100);
            asset.moddingSlots = Mathf.Clamp(dto.moddingSlots, 0, 10);
            asset.description = dto.description;

            // Convert attachment slots
            if (dto.attachmentSlots != null && dto.attachmentSlots.Length > 0)
            {
                asset.attachmentSlots = new WeaponItemDataSO.AttachmentSlot[dto.attachmentSlots.Length];
                for (int i = 0; i < dto.attachmentSlots.Length; i++)
                {
                    asset.attachmentSlots[i] = new WeaponItemDataSO.AttachmentSlot
                    {
                        slotType = dto.attachmentSlots[i].slotType,
                        slotIndex = dto.attachmentSlots[i].slotIndex,
                        isRequired = dto.attachmentSlots[i].isRequired
                    };
                }
            }

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Weapon items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportWeaponComponentItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        WeaponComponentItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<WeaponComponentItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/WeaponComponentItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            WeaponComponentItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<WeaponComponentItemDataSO>(assetPath);
            
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing weapon component item: {dto.id} (already exists)");
                continue;
            }

            WeaponComponentItemDataSO asset = ScriptableObject.CreateInstance<WeaponComponentItemDataSO>();

            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.componentType = dto.componentType;
            asset.compatibleSlots = dto.compatibleSlots;
            
            // Magazine-specific fields
            asset.magazineCapacity = dto.magazineCapacity;
            asset.caliber = dto.caliber;
            asset.magazineType = dto.magazineType;
            
            asset.damageModifier = Mathf.Clamp(dto.damageModifier, -50, 50);
            asset.recoilModifier = Mathf.Clamp(dto.recoilModifier, -50, 50);
            asset.ergonomicsModifier = Mathf.Clamp(dto.ergonomicsModifier, -50, 50);
            asset.accuracyModifier = Mathf.Clamp(dto.accuracyModifier, -50, 50);
            asset.weightModifier = Mathf.Clamp(dto.weightModifier, -2000, 2000);
            asset.muzzleVelocityModifier = Mathf.Clamp(dto.muzzleVelocityModifier, -200, 200);
            asset.effectiveRangeModifier = Mathf.Clamp(dto.effectiveRangeModifier, -200, 200);
            asset.penetrationModifier = Mathf.Clamp(dto.penetrationModifier, -50, 50);
            asset.hasBuiltInRail = dto.hasBuiltInRail;
            asset.railType = dto.railType;
            asset.description = dto.description;

            // Convert sub slots
            if (dto.subSlots != null && dto.subSlots.Length > 0)
            {
                asset.subSlots = new WeaponComponentItemDataSO.ComponentSubSlot[dto.subSlots.Length];
                for (int i = 0; i < dto.subSlots.Length; i++)
                {
                    asset.subSlots[i] = new WeaponComponentItemDataSO.ComponentSubSlot
                    {
                        slotType = dto.subSlots[i].slotType,
                        slotIndex = dto.subSlots[i].slotIndex,
                        hasBuiltInRail = dto.subSlots[i].hasBuiltInRail
                    };
                }
            }
            else
            {
                asset.subSlots = Array.Empty<WeaponComponentItemDataSO.ComponentSubSlot>();
            }

            if (dto.loadedRounds != null && dto.loadedRounds.Length > 0)
            {
                asset.loadedRounds = new WeaponComponentItemDataSO.LoadedRoundSegment[dto.loadedRounds.Length];
                for (int i = 0; i < dto.loadedRounds.Length; i++)
                {
                    asset.loadedRounds[i] = new WeaponComponentItemDataSO.LoadedRoundSegment
                    {
                        orderIndex = dto.loadedRounds[i].orderIndex,
                        roundCount = dto.loadedRounds[i].roundCount,
                        ammoId = dto.loadedRounds[i].ammoId,
                        ammoDisplayName = dto.loadedRounds[i].ammoDisplayName,
                        ammoNotes = dto.loadedRounds[i].ammoNotes
                    };
                }
            }
            else
            {
                asset.loadedRounds = Array.Empty<WeaponComponentItemDataSO.LoadedRoundSegment>();
            }

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Weapon component items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportAmmoItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        AmmoItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<AmmoItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/AmmoItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            AmmoItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<AmmoItemDataSO>(assetPath);
            
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing ammo item: {dto.id} (already exists)");
                continue;
            }

            AmmoItemDataSO asset = ScriptableObject.CreateInstance<AmmoItemDataSO>();

            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.caliber = dto.caliber;
            asset.damageBonus = Mathf.Clamp(dto.damageBonus, -50, 50);
            asset.penetration = Mathf.Clamp(dto.penetration, 0, 100);
            asset.accuracyBonus = Mathf.Clamp(dto.accuracyBonus, -50, 50);
            asset.recoilModifier = Mathf.Clamp(dto.recoilModifier, -50, 50);
            asset.armorPiercing = dto.armorPiercing;
            asset.hollowPoint = dto.hollowPoint;
            asset.tracer = dto.tracer;
            asset.incendiary = dto.incendiary;
            asset.value = Mathf.Clamp(dto.value, 0, 100);
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Ammo items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportArmorItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        ArmorItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<ArmorItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/ArmorItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            ArmorItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<ArmorItemDataSO>(assetPath);
            
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing armor item: {dto.id} (already exists)");
                continue;
            }

            ArmorItemDataSO asset = ScriptableObject.CreateInstance<ArmorItemDataSO>();

            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.armorType = dto.armorType;
            asset.armorClass = Mathf.Clamp(dto.armorClass, 0, 6);
            asset.durability = Mathf.Clamp(dto.durability, 0, 100);
            asset.material = Mathf.Clamp(dto.material, 0, 100);
            asset.protectionZones = dto.protectionZones;
            asset.movementSpeedPenalty = Mathf.Clamp(dto.movementSpeedPenalty, 0, 100);
            asset.ergonomicsPenalty = Mathf.Clamp(dto.ergonomicsPenalty, 0, 100);
            asset.turnSpeedPenalty = Mathf.Clamp(dto.turnSpeedPenalty, 0, 100);
            asset.weight = dto.weight;
            asset.capacity = dto.capacity;
            asset.blocksHeadset = dto.blocksHeadset;
            asset.blocksFaceCover = dto.blocksFaceCover;
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Armor items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void ImportClothingItems(string jsonText)
    {
        string wrapped = "{ \"items\": " + jsonText + " }";

        ClothingItemListWrapper wrapper;
        try
        {
            wrapper = JsonUtility.FromJson<ClothingItemListWrapper>(wrapped);
        }
        catch (Exception e)
        {
            Debug.LogError($"[ItemImporter] JSON parse error: {e.Message}");
            return;
        }

        if (wrapper == null || wrapper.items == null || wrapper.items.Length == 0)
        {
            Debug.LogWarning("[ItemImporter] No items found in JSON.");
            return;
        }

        string assetsRoot = "Assets/Resources/RundeeItemFactory/ClothingItems";
        EnsureFolderStructure(assetsRoot);

        int created = 0;
        int skipped = 0;

        foreach (var dto in wrapper.items)
        {
            if (string.IsNullOrEmpty(dto.id))
            {
                Debug.LogWarning("[ItemImporter] Skipping item with empty id.");
                continue;
            }

            string safeId = MakeSafeFileName(dto.id);
            string assetPath = assetsRoot + "/" + safeId + ".asset";

            ClothingItemDataSO existingAsset = AssetDatabase.LoadAssetAtPath<ClothingItemDataSO>(assetPath);
            
            if (existingAsset != null)
            {
                skipped++;
                Debug.Log($"[ItemImporter] Skipping existing clothing item: {dto.id} (already exists)");
                continue;
            }

            ClothingItemDataSO asset = ScriptableObject.CreateInstance<ClothingItemDataSO>();

            asset.id = dto.id;
            asset.displayName = dto.displayName;
            asset.category = dto.category;
            asset.rarity = dto.rarity;
            asset.maxStack = dto.maxStack;
            asset.clothingType = dto.clothingType;
            asset.coldResistance = Mathf.Clamp(dto.coldResistance, 0, 100);
            asset.heatResistance = Mathf.Clamp(dto.heatResistance, 0, 100);
            asset.waterResistance = Mathf.Clamp(dto.waterResistance, 0, 100);
            asset.windResistance = Mathf.Clamp(dto.windResistance, 0, 100);
            asset.comfort = Mathf.Clamp(dto.comfort, 0, 100);
            asset.mobilityBonus = Mathf.Clamp(dto.mobilityBonus, -50, 50);
            asset.staminaBonus = Mathf.Clamp(dto.staminaBonus, -50, 50);
            asset.durability = Mathf.Clamp(dto.durability, 0, 100);
            asset.material = Mathf.Clamp(dto.material, 0, 100);
            asset.weight = dto.weight;
            asset.isInsulated = dto.isInsulated;
            asset.isWaterproof = dto.isWaterproof;
            asset.isWindproof = dto.isWindproof;
            asset.description = dto.description;

            AssetDatabase.CreateAsset(asset, assetPath);
            created++;
        }

        AssetDatabase.SaveAssets();
        AssetDatabase.Refresh();

        Debug.Log($"[ItemImporter] Clothing items imported. Created: {created}, Skipped (already exist): {skipped}");
    }

    private static void EnsureFolderStructure(string assetsRoot)
    {
        if (!AssetDatabase.IsValidFolder("Assets/Resources"))
        {
            AssetDatabase.CreateFolder("Assets", "Resources");
        }
        if (!AssetDatabase.IsValidFolder("Assets/Resources/RundeeItemFactory"))
        {
            AssetDatabase.CreateFolder("Assets/Resources", "RundeeItemFactory");
        }
        string folderName = assetsRoot.Substring(assetsRoot.LastIndexOf('/') + 1);
        string parentPath = "Assets/Resources/RundeeItemFactory";
        if (!AssetDatabase.IsValidFolder(assetsRoot))
        {
            AssetDatabase.CreateFolder(parentPath, folderName);
        }
    }

    private static string MakeSafeFileName(string id)
    {
        var invalid = Path.GetInvalidFileNameChars();
        string result = id;
        foreach (var c in invalid)
        {
            result = result.Replace(c, '_');
        }
        return result;
    }
}

