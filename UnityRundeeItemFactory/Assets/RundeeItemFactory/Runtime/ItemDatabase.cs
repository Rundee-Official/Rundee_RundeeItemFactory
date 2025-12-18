// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemDatabase.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Runtime database for loading and accessing all item ScriptableObjects.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

// Unity Includes
using UnityEngine;

// ============================================================================
// SECTION 1: ItemDatabase Class
// ============================================================================

/// <summary>
/// Runtime database component that loads all item ScriptableObjects from Resources folder.
/// </summary>
public class ItemDatabase : MonoBehaviour
{
    [Header("Food Items")]
    public FoodItemDataSO[] foodItems;

    [Header("Drink Items")]
    public DrinkItemDataSO[] drinkItems;

    [Header("Material Items")]
    public MaterialItemDataSO[] materialItems;

    [Header("Weapon Items")]
    public WeaponItemDataSO[] weaponItems;

    [Header("Weapon Component Items")]
    public WeaponComponentItemDataSO[] weaponComponentItems;

    [Header("Ammo Items")]
    public AmmoItemDataSO[] ammoItems;

    [Header("Armor Items")]
    public ArmorItemDataSO[] armorItems;

    [Header("Clothing Items")]
    public ClothingItemDataSO[] clothingItems;

    // ========================================================================
    // Unity Lifecycle Methods
    // ========================================================================

    private void Awake()
    {
        LoadAllItems();
    }

    public void LoadAllItems()
    {
        // Load Food items
        foodItems = Resources.LoadAll<FoodItemDataSO>("RundeeItemFactory/FoodItems");
        Debug.Log($"[ItemDatabase] Loaded {foodItems.Length} food items from Resources.");

        // Load Drink items
        drinkItems = Resources.LoadAll<DrinkItemDataSO>("RundeeItemFactory/DrinkItems");
        Debug.Log($"[ItemDatabase] Loaded {drinkItems.Length} drink items from Resources.");

        // Load Material items
        materialItems = Resources.LoadAll<MaterialItemDataSO>("RundeeItemFactory/MaterialItems");
        Debug.Log($"[ItemDatabase] Loaded {materialItems.Length} material items from Resources.");

        // Load Weapon items
        weaponItems = Resources.LoadAll<WeaponItemDataSO>("RundeeItemFactory/WeaponItems");
        Debug.Log($"[ItemDatabase] Loaded {weaponItems.Length} weapon items from Resources.");

        // Load Weapon Component items
        weaponComponentItems = Resources.LoadAll<WeaponComponentItemDataSO>("RundeeItemFactory/WeaponComponentItems");
        Debug.Log($"[ItemDatabase] Loaded {weaponComponentItems.Length} weapon component items from Resources.");

        // Load Ammo items
        ammoItems = Resources.LoadAll<AmmoItemDataSO>("RundeeItemFactory/AmmoItems");
        Debug.Log($"[ItemDatabase] Loaded {ammoItems.Length} ammo items from Resources.");

        // Load Armor items
        armorItems = Resources.LoadAll<ArmorItemDataSO>("RundeeItemFactory/ArmorItems");
        Debug.Log($"[ItemDatabase] Loaded {armorItems.Length} armor items from Resources.");

        // Load Clothing items
        clothingItems = Resources.LoadAll<ClothingItemDataSO>("RundeeItemFactory/ClothingItems");
        Debug.Log($"[ItemDatabase] Loaded {clothingItems.Length} clothing items from Resources.");

        // Output logs
        LogAllItems();
    }

    // ========================================================================
    // Private Helper Methods
    // ========================================================================

    private void LogAllItems()
    {
        foreach (var item in foodItems)
        {
            Debug.Log(
                $"[FoodItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"Hunger+{item.hungerRestore}, Thirst+{item.thirstRestore})");
        }

        foreach (var item in drinkItems)
        {
            Debug.Log(
                $"[DrinkItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"Hunger+{item.hungerRestore}, Thirst+{item.thirstRestore})");
        }

        foreach (var item in materialItems)
        {
            Debug.Log(
                $"[MaterialItem] {item.id} / {item.displayName} " +
                $"(cat: {item.category}, rarity: {item.rarity}, " +
                $"type: {item.materialType}, value: {item.value}, " +
                $"hardness: {item.hardness}, flammability: {item.flammability})");
        }
    }

    // ========================================================================
    // Public API - Find Methods (by Item Type - Alphabetical Order)
    // ========================================================================

    public AmmoItemDataSO FindAmmoItem(string id)
    {
        foreach (var item in ammoItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public ArmorItemDataSO FindArmorItem(string id)
    {
        foreach (var item in armorItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public ClothingItemDataSO FindClothingItem(string id)
    {
        foreach (var item in clothingItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public DrinkItemDataSO FindDrinkItem(string id)
    {
        foreach (var item in drinkItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public FoodItemDataSO FindFoodItem(string id)
    {
        foreach (var item in foodItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public MaterialItemDataSO FindMaterialItem(string id)
    {
        foreach (var item in materialItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public WeaponItemDataSO FindWeaponItem(string id)
    {
        foreach (var item in weaponItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }

    public WeaponComponentItemDataSO FindWeaponComponentItem(string id)
    {
        foreach (var item in weaponComponentItems)
        {
            if (item.id == id) return item;
        }
        return null;
    }
}

